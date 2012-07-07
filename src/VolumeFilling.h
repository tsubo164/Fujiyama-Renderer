/*
Copyright (c) 2011-2012 Hiroshi Tsubokawa
See LICENSE and README
*/

#ifndef VOLUME_FILLING_H
#define VOLUME_FILLING_H

#ifdef __cplusplus
extern "C" {
#endif

struct Volume;

struct CloudControlPoint {
	double orig[3];
	double udir[3];
	double vdir[3];
	double wdir[3];
	double noise_space[3];

	double density;
	double radius;
	double noise_amplitude;
};

struct WispsControlPoint {
	double orig[3];
	double udir[3];
	double vdir[3];
	double wdir[3];
	double noise_space[3];

	double density;
	double radius;
	double noise_amplitude;

	double speck_count;
	double speck_radius;
};

extern void LerpWispConstrolPoint(struct WispsControlPoint *cp,
		const struct WispsControlPoint *cp0, const struct WispsControlPoint *cp1,
		double t);

extern void FillWithSphere(struct Volume *volume,
		const double *center, double radius, float density);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XXX_H */

