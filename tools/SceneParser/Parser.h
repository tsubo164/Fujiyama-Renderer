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
	PSR_ERR_NOERR = 0,
	PSR_ERR_UNKNOWNCMD,
	PSR_ERR_MANYARGS,
	PSR_ERR_FEWARGS,
	PSR_ERR_NAMEEXISTS,
	PSR_ERR_NAMENOTFOUND,
	PSR_ERR_FAILSETPROP,
	PSR_ERR_PLUGINNOTFOUND,
	PSR_ERR_FAILNEW,
	PSR_ERR_FAILRENDER
};

extern struct Parser *PsrNew(void);
extern void PsrFree(struct Parser *parser);

extern int PsrGetErrorNo(void);
extern const char *PsrGetErrorMessage(int err_no);

extern int PsrParseLine(struct Parser *parser, const char *line);
extern int PsrGetLineNo(const struct Parser *parser);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

