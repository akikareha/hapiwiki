#!/usr/bin/perl
use strict;
use warnings;
use utf8;
use open ":std", ":encoding(UTF-8)";
use LWP::UserAgent;
use JSON;

# OpenAI API key
my $api_key = do {
    open my $fh, "<", "$ENV{WIKI_API_KEY}" or die "Cannot read API key";
    chomp( my $k = <$fh> );
    $k;
};

my $input_text = do { local $/; <> };

my $prompt = <<"PROMPT";
以下の投稿文が公共のウィキに載せても問題ない内容かを確認し、
問題がある部分を削除あるいは修正してください。
意味の通らないところも修正してください。
なお、改行コードは、そのまま保存してください。
また、行頭の空白も、そのまま保存してください。
さらに、文体を丁寧かつ可愛い感じのですます調で、少しケモ耳少女風にしてください。
本文のみを提示してください。

元の投稿文:
$input_text
PROMPT

# HTTP request
my $ua = LWP::UserAgent->new;
$ua->default_header( "Authorization" => "Bearer $api_key" );
$ua->default_header( "Content-Type"  => "application/json" );

my $res = $ua->post(
    "https://api.openai.com/v1/chat/completions",
    Content => encode_json(
        {
            model    => "gpt-4o-mini",
            messages => [
                {
                    role    => "system",
                    content => "あなたは公共のウィキ投稿をチェックして、文体をマイルドに整えるモデレータです。",
                },
                { role => "user", content => $prompt },
            ],
            temperature => 0.3,
        }
    )
);

# Response
if ( $res->is_success ) {
    my $data = decode_json( $res->decoded_content );
    my $text = $data->{choices}[0]{message}{content};
    print $text;
}
else {
    print "(AI filter failed: " . $res->status_line . ")\n";
}
