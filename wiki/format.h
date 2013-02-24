#ifndef WIKI_FORMAT_H
#define WIKI_FORMAT_H

#include <stdio.h>

#include "wiki/config.h"

extern void unify_line_breaks(char *s);
extern void wiki_format(const char *comment, FILE *out);

#endif
