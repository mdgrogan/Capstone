#include <stdbool.h>
#include <math.h>

#define VERTMAX 100
#define TOL 1e-7

/* Type definitions */
typedef struct { 
    double x; 
    double y; 
} vec_t;

typedef struct { 
    int n; 
    vec_t *v; 
} polygon_t;

/* Function declarations */
int boundary_check(double *pos, const polygon_t *p);
int read_boundary(const char *f, double *latvert, double *lonvert);
polygon_t *new_boundary(const int n, double *latvert, double *lonvert);
void free_boundary(polygon_t *p);
