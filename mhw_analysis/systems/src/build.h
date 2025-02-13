
#ifndef _BUILD_H_
#define _BUILD_H_

#include <stdbool.h>

void first_pass(char *cube, int *mask, int *shape, int *parent, int *category);
void second_pass(int *mask, int *parent, int *shape, long *NSpax, int *category);
void final_pass(int ndet, int *mask, int *shape, float *xcen, float *ycen, float *zcen, int *xboxmin, int *xboxmax,
                int *yboxmin, int *yboxmax, int *zboxmin, int *zboxmax, long *NSpax, int *dcat, int *LabelToId, int *category);
void lunion(int *parent, int x, int y);
long convert_indices(long i, long j, long k, int DimY, int DimZ);
void calc_km(int *mask, int *areas, float *areas_km2, double *NVox_km, int max_label, int *shape, float cell_deg);

#endif // _BUILD_H_

