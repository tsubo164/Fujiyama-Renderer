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

enum PsrErroNo {
	ERR_PSR_NOERR = 0,
	ERR_PSR_UNKNOWNCMD,
	ERR_PSR_MANYARGS,
	ERR_PSR_FEWARGS,
	ERR_PSR_NAMEEXISTS,
	ERR_PSR_NAMENOTFOUND,
	ERR_PSR_PLUGINNOTFOUND,
	ERR_PSR_FAILNEW,
	ERR_PSR_FAILRENDER
};

extern struct Parser *PsrNew(void);
extern void PsrFree(struct Parser *parser);

extern int PsrGetErrorNo(void);
extern const char *PsrGetErrorMessage(int err_no);
#if 0
extern const char *PsrGetErrorDetail(void);
#endif

extern int PsrParseLine(struct Parser *parser, const char *line);
extern int PsrGetLineNo(const struct Parser *parser);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

