#ifndef __WINDOWING_H
#define __WINDOWING_H


#define CENTRED     1
#define CYCLIC      2

#define GAUSSIAN    0
#define BLACKMAN    1
#define HAMMING     2
#define HANNING     3
#define BARTLETT    4
#define CIRCULAR    5
#define RECTANGULAR 6

void create_window(double *window, int width, int TYPE );
void create_window_2D(double **window, int width, int TYPE );

void write_window( char *filename, double *window, int L, int H );
void write_window_labelled( char *filename, double *window, int L, int H,double low, double high, double bin );
void write_window_2D( char *filename, double **window, int Lx, int Hx,
		int Ly, int Hy);
void write_window_2D_pgm_P2( char *filename, double **window, int Lx, int
		  Hx, int Ly, int Hy, int maxval , char *commentstring);
void write_window_2D_pgm_P2_float( char *filename, float **window, int Lx, int
		  Hx, int Ly, int Hy, int maxval );
void write_window_2D_pgm_P5_float( char *filename, float **window, int Lx, int
		  Hx, int Ly, int Hy, int maxval );

void normalise_window( double *window, int L, int H );
void normalise_window_2D( double **window, int Lx, int Hx, int Ly, int Hy );

void convolve( double *data, int L, int H, double *window, int width,
	      int origin, double *result, int OPTIONS );
void convolve_2D( double **data, int Lx, int Hx, int Ly, int Hy,
	      double **window, int width, int originx, int originy,
	      double **result, int OPTIONS );

double window_rectangular_1D(int i, int N);
double window_circular_1D(int i, int N);
double window_bartlett_1D(int i, int N);
double window_hanning_1D(int i, int N);
double window_hamming_1D(int i, int N);
double window_blackman_1D(int i, int N);
double window_gaussian_1D(int i, int N, double sd);

#endif
