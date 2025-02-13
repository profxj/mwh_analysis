/*
Shapiro & Stockman, Computer Vision, Mar 2000
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "build.h"

#define PI 3.14159265358 

void lunion(int *parent, int x, int y) {
    /*

    Args:
       parent:
       x:
       y:
    */
    //..find root labels
    while (parent[x] != 0)
        x = parent[x];

    while (parent[y] != 0)
        y = parent[y];

    if (x < y) {
        parent[y] = x;
    }
    else if (x > y) {
        parent[x] = y;
    }
}

long convert_indices(long i, long j, long k, int DimY, int DimZ) {
    // Wrap around longitude
    if (j == DimY)
        j=0;
    if (j == -1)
        j=DimY-1;
    // Do it
    return i*DimY*DimZ + j*DimZ + k;
}

void first_pass(char *cube, int *mask, int *shape, int *parent, int *category) {
    /*
    */

    // Init
    int label = 0;

    int DimX = shape[0];
    int DimY = shape[1];
    int DimZ = shape[2];

    // printf("%d, %d, %d\n", DimX, DimY, DimZ);

    long idx, idx2;
    long i,j,k;
    long ii,jj,kk;
    long ip,im,jp,jm;
    int p;
    int count;

    int all_zero;

    // Init the mask
    for (i = 0; i<DimX; i++)
        for (j = 0; j<DimY; j++)
            for (k = 0; k<DimZ; k++) {
                idx = convert_indices(i,j,k, DimY, DimZ);
                mask[idx] = 0;
            }

    // Labels
    int prior_labels[27];
    int minlabel;
    int maxnlabels = 100000000;
    int this_label;

    // Ocean boundaries
    int indian_lon0 = 580; // Approximately 145E
    int indian_lat0 = 360; // Approximately 0deg
    int indian_lon1 = 400; // Approximately 100E
    int indian_lat1a = 360; // Approximately 0deg
    int indian_lat1b = 484; // Approximately 31degN
    int indian_lon2 = 80; // Approximately 20E
    int indian_lat2 = 360; // Approximately 0deg

    int pacific_lon0 = 1160; // Approximately 70W
    int pacific_lat0 = 360; // Approximately 0deg
    int pacific_lon1a = 400; // Approximately 100E
    int pacific_lon1b = 1040; // Approximately 70W
    int pacific_lat1 = 624; // Approximately 0deg

    // Loop me!
    for (i = 1; i<DimX-1; i++)
        for (j = 0; j<DimY; j++)
            for (k = 1; k<DimZ-1; k++) {
                idx = convert_indices(i,j,k, DimY, DimZ);
                // Debuggin
                //if (j == 1 && k == 1)
                //    printf("idx = %ld\n", idx);

                if (cube[idx] == 0)
                    continue;

                // Search for neighboring pixels
                // Also determine the minimum value of the label
                count = 0;
                all_zero = 0;
                minlabel = maxnlabels;

                // Reset
                jp=1;
                jm=1;
                ip=1;
                im=1;

                // Indian Ocean at 145E; E/W boundary
                if (j==indian_lon0 && i<=indian_lat0)
                    jp = 0;
                if (j==indian_lon0+1 && i<=indian_lat0)
                    jm = 0;
                // Indian Ocean at 100E; E/W boundary
                if (j==indian_lon1 && i>=indian_lat1a-1 && i<=indian_lat1b)
                    jp = 0;
                if (j==indian_lon1+1 && i>=indian_lat1a && i<=indian_lat1b)
                    jm = 0;
                // Indian Ocean 0deg N/S boundary
                if (i==indian_lat1a && j>=indian_lon1 && j<=indian_lon0+1)
                    ip = 0;
                if (i==indian_lat1a+1 && j>=indian_lon1 && j<=indian_lon0+1)
                    im = 0;
                // Indian Ocean at 20E; E/W boundary
                if (j==indian_lon2 && i<=indian_lat2)
                    jp = 0;
                if (j==indian_lon2+1 && i<=indian_lat2)
                    jm = 0;

                // Pacific Ocean at 70W; E/W boundary
                if (j==pacific_lon0 && i<=pacific_lat0)
                    jp = 0;
                if (j==pacific_lon0+1 && i<=pacific_lat0)
                    jm = 0;

                // Pacific Ocean at 66N; N/S boundary
                if (i==pacific_lat1 && j>=pacific_lon1a && j<=pacific_lon1b)
                    ip = 0;
                if (i==pacific_lat1+1 && j>=pacific_lon1a && j<=pacific_lon1b)
                    im = 0;

                // Do it
                for (ii=i-im; ii<=i+ip; ii++) // Latitude
                    for (jj=j-jm; jj<=j+jp; jj++) // Longitude
                        for (kk=k-1; kk<=k+1; kk++) {  // Time
                            idx2 = convert_indices(ii,jj,kk, DimY, DimZ);
                            // prior_labels[count] = mask[ii,jj,kk];
                            prior_labels[count] = mask[idx2];
                            if (prior_labels[count] != 0) {
                                all_zero = 1;
                                if (prior_labels[count] < minlabel)
                                    minlabel = prior_labels[count];
                            }
                            count++;
                        }
                /* Debuggin
                if (i==2 && j==5 && k==3) {
                    for (p=0; p<27; p++) { // p in range(prior_labels.size):
                        printf("A %d\n", prior_labels[p]);
                    }
                    printf("A2 %d\n", minlabel);
                }
                */

                if (all_zero == 0) {
                    label=label+1;
                    if (label > maxnlabels) // # STOP "Increase stack size (maxnlabels)!"
                        printf("Exception happened here\n");
                    // mask[i][j][k] = label;
                    mask[idx] = label;
                    category[label] = cube[idx];
                } else { // # !..this spaxel is connected to another one
                    this_label = minlabel; // np.min(prior_labels[prior_labels > 0])
                    mask[idx] = this_label;
                    // #!..update parent tree
                    for (p=0; p<count; p++) { // p in range(prior_labels.size):
                        if (prior_labels[p] != 0  &&  prior_labels[p] != this_label)
                            lunion(parent, this_label, prior_labels[p]);
                    }
                    category[minlabel] = fmax(category[minlabel], cube[idx]);
                }
             }
}

void second_pass(int *mask, int *parent, int *shape, long *NSpax, int *category) {

    // Init
    int DimX = shape[0];
    int DimY = shape[1];
    int DimZ = shape[2];

    long i,j,k;
    long idx;
    int this_label;
    int p;

    // !..second pass:
    // !... replace labels using the parent tree
    // !... get NSpax for each individual connected component
    // for i in range(DimX):
    //    for j in range(DimY):
    //        for k in range(DimZ):
    for (i = 0; i<DimX; i++)
        for (j = 0; j<DimY; j++)
            for (k = 0; k<DimZ; k++) {
                idx = convert_indices(i,j,k, DimY, DimZ);
                this_label=mask[idx];
                if (this_label != 0) {
                    // #!..assign value from parent tree
                    p = this_label;
                    while (parent[p] != 0)
                       p = parent[p];

                    mask[idx] = p;
                    category[p] = category[this_label];  // This may be superfluous
                    // !..update NSpax counter associated with this label
                    NSpax[p] = NSpax[p]+1;
                }
            }
}

void final_pass(int ndet, int *mask, int *shape, float *xcen, float *ycen, float *zcen, int *xboxmin, int *xboxmax,
                int *yboxmin, int *yboxmax, int *zboxmin, int *zboxmax, long *NSpax, int *dcat, int *LabelToId, int *category) {

    // Init
    int DimX = shape[0];
    int DimY = shape[1];
    int DimZ = shape[2];

    long i,j,k;
    long idx;
    long nbig=0;
    int id;
    int this_label;

    int* n0 = (int*) malloc (ndet * sizeof(int));
    int* n180 = (int*) malloc (ndet * sizeof(int));
    int* n360 = (int*) malloc (ndet * sizeof(int));
    long* x = (long*) malloc (ndet * sizeof(long));
    long* y = (long*) malloc (ndet * sizeof(long));
    long* z = (long*) malloc (ndet * sizeof(long));
    float* ycen2 = (float*) malloc (ndet * sizeof(float));
    for (i=0; i<ndet; i++) {
        x[i] = 0;
        y[i] = 0;
        z[i] = 0;
        n0[i] = 0;
        n180[i] = 0;
        n360[i] = 0;
        ycen2[i] = 0.;
    }


    printf("starting the loop");
    // # Fill !..find bounding boxes and centroid for each objects
    for (i = 0; i<DimX; i++)
        for (j = 0; j<DimY; j++)
            for (k = 0; k<DimZ; k++) {
                idx = convert_indices(i,j,k, DimY, DimZ);
                this_label = mask[idx];
                if (this_label != 0) {
                    id = LabelToId[this_label]; //  #!..get object associated with pixel  (0-based)
                    if (id != -1) {
                        x[id] += i;
                        y[id] += j;
                        z[id] += k;
                        // Debug
                        //if (this_label == 32106)
                        //    printf("bigone: z=%ld, k=%ld\n", z[id], k);
                        // Deal with longitude
                        if (j < DimY/2) {
                            ycen2[id] += j - 0.5 + DimY/2;  // Shifted by 180deg
                        } else {
                            ycen2[id] += j - 0.5 - DimY/2;  // Shifted by 180deg
                        }

                        if (j == 0)
                            n0[id] += 1;
                        if (j == DimY-1)
                            n360[id] += 1;
                        if (j == DimY/2)
                            n180[id] += 1;

                        xboxmin[id] = fmin(xboxmin[id], i);
                        yboxmin[id] = fmin(yboxmin[id], j);
                        zboxmin[id] = fmin(zboxmin[id], k);

                        xboxmax[id] = fmax(xboxmax[id], i);
                        yboxmax[id] = fmax(yboxmax[id], j);
                        zboxmax[id] = fmax(zboxmax[id], k);

                        dcat[id] = category[this_label];
                    } else { // # Cleanup mask
                        mask[idx] = 0;
                    }
                }
            }

    //
    printf("bigone: nbig=%ld\n", nbig);

    // # !..finalize geometrical centroid calculation
    for (i = 0; i<ndet; i++) {
        xcen[i] = (float)x[i] / (float)NSpax[i];
        // Debuggin
        //if (i == 181)
        //    printf("bigone: zcen=%f, NSpax=%ld\n", zcen[i], NSpax[i]);
        zcen[i] = (float)z[i] / (float)NSpax[i];
        // Deal with longitude
        if (n0[i] > n180[i] && n360[i] > n180[i]) {
            ycen[i] = ycen2[i] / (float)NSpax[i] - DimY/2;
            if (ycen[i] < 0)
                ycen[i] += DimY;
        } else {
            ycen[i] = (float)y[i] / (float)NSpax[i];
        }
        // TOOD -- FIX yboxmin too!
    }


}

// Calculate areas and NVox in physical units
void calc_km(int *mask, int *areas, float *areas_km2, double *NVox_km, int max_label, int *shape, float cell_deg) {

    // cell_deg -- Cell size in deg. Used to convert to km

    // Init
    int DimX = shape[0];
    int DimY = shape[1];
    int DimZ = shape[2];

    long i,j,k, ii;
    long idx;

    // Deal with km
    double R_Earth = 6371.0;
    double cell_km;
    double lat;

    // cell size in km on Equator
    cell_km = cell_deg * 2 * PI * R_Earth / 360.;

    // Area of a cell at a given lat 
    double* cell_lat = (double*) malloc (DimX * sizeof(double));
    for (i = 0; i<DimX; i++) {
        lat = -89.875 + i*cell_deg;
        cell_lat[i] = cell_km * cell_km * cos(PI * lat / 180.);
    }

    //printf("Define sub_areas with max_label=%d\n", max_label);
    int* sub_areas = (int*) malloc (max_label * sizeof(int));
    float* sub_areas_km2 = (float*) malloc (max_label * sizeof(float));
    //printf("Entering the main loops\n");

    // # Fill !..find bounding boxes and centroid for each objects
    for (k = 0; k<DimZ; k++) {
        for (i = 0; i<DimX; i++)
            for (j = 0; j<DimY; j++) {
                if (i == 0 && j == 0) {
                    // Reset to 0
                    for (ii = 0; ii <= max_label; ii++) {
                        sub_areas[ii] = 0;
                        sub_areas_km2[ii] = 0.;
                    }
                }
                idx = convert_indices(i,j,k, DimY, DimZ);
                if (mask[idx] == 0)
                    continue;

                // Increment
                sub_areas[mask[idx]]++;
                sub_areas_km2[mask[idx]] = sub_areas_km2[mask[idx]] + cell_lat[i];
                NVox_km[mask[idx]] = NVox_km[mask[idx]] + cell_lat[i];
            }

        // Update
        for (ii = 0; ii <= max_label; ii++) {
            areas[ii] = fmax(areas[ii], sub_areas[ii]);
            areas_km2[ii] = fmax(areas_km2[ii], sub_areas_km2[ii]);
        }
    }
}