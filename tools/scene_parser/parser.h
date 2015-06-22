// Copyright (c) 2011-2015 Hiroshi Tsubokawa
// See LICENSE and README

#ifndef PARSER_H
#define PARSER_H

class Parser;

extern Parser *PsrNew(void);
extern void PsrFree(Parser *parser);

extern int PsrParseLine(Parser *parser, const char *line);
extern int PsrGetLineNo(const Parser *parser);
extern const char *PsrGetErrorMessage(const Parser *parser);

#endif // FJ_XXX_H
