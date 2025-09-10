# HapiWiki - A simple wiki program written in C

Author: Aki Kareha <aki@kareha.org>  
Licence: CC0  

## Build and Installation

1. Edit `wiki/config.h` and adjust settings if needed.
2. Run `make` -> this will generate `wiki/wiki.cgi`.
3. Deploy:
    * Copy `wiki.cgi` to your CGI directory
    * Copy `style.css`, `logo.png`, `icon.png`, and `tile.png` to your HTML directory
4. Prepare data directory:
    * Create the path specified in `WIKI_ROOT` (e.g., `/home/hapi/wiki`)
    * Inside it, create a `data` directory writable by the web server user
5. `WIKI_ROOT` can be set in `wiki/config.h` or overridden by the environment variable `WIKI_ROOT`.

## Filtering (optional)

- To enable filtering:
    1. Place `wiki/filter.pl` in `WIKI_ROOT`
    2. Create `WIKI_ROOT/openai.key` and put your OpenAI API key inside
- To disable filtering:
    * Comment out `WIKI_FILTER` in `wiki/config.h`.

## Usage

- Access `wiki.cgi` with your web browser
- Use **Login** -> create an account and sign in
- Use **Edit** -> edit pages
- To save changes, you must **log in** and **preview** first

## Text Formatting Rules

- Links
    * CamelCase -> converted to wiki links
    * URLs -> converted to hyperlinks
    * [[Some Words]] -> converted to wiki links
- Lists
    * `*` -> unordered list
    * `:` -> definition list
- Others
    * `----` -> horizontal line
    * MathJax enabled -> write formulas using TeX notation
