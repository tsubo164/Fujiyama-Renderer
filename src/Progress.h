/*
Copyright (c) 2011-2013 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef PROGRESS_H
#define PROGRESS_H

#ifdef __cplusplus
extern "C" {
#endif

struct Progress;

extern struct Progress *PrgNew(void);
extern void PrgFree(struct Progress *prg);

extern void PrgStart(struct Progress *prg, int total_iterations);
extern void PrgIncrement(struct Progress *prg);
extern void PrgDone(struct Progress *prg);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */
