/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef PARSER_H
#define PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

struct Parser;

extern struct Parser *PsrNew(void);
extern void PsrFree(struct Parser *parser);

extern const char *PsrGetErrorMessage(void);

extern int PsrParseLine(struct Parser *parser, const char *line);
extern int PsrGetLineNo(const struct Parser *parser);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

