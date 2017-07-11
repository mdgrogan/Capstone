#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "boundarycheck.h"

/* Pseudo-vector functions */
vec_t vsub(vec_t a, vec_t b)
{
	vec_t c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	return c;
}

vec_t vadd(vec_t a, vec_t b)
{
	vec_t c;
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	return c;
}

double vdot(vec_t a, vec_t b)
{
	return a.x * b.x + a.y * b.y;
}

double vcross(vec_t a, vec_t b)
{
	return a.x * b.y - a.y * b.x;
}
 
vec_t vmadd(vec_t a, double s, vec_t b)
{
	vec_t c;
	c.x = a.x + s * b.x;
	c.y = a.y + s * b.y;
	return c;
}
 
/***********************************************
 * Check if x0->x1 intersects with y0->y1
 * return 1=intersect, 0=unknown, -1=no intersect
 **********************************************/
int intersect(vec_t x0, vec_t x1, vec_t y0, vec_t y1, vec_t *sect)
{
    vec_t dx = vsub(x1, x0);
    vec_t dy = vsub(y1, y0);
    double d = vcross(dy, dx); 
    double a;

    /* check if edges are parallel */
    if (!d) 
        return 0;
 
    a = (vcross(x0, dx) - vcross(y0, dx)) / d;
    if (sect)
    	*sect = vmadd(y0, a, dy);
 
    if (a < -TOL || a > 1 + TOL) 
        return -1;
    if (a < TOL || a > 1 - TOL) 
        return 0;
 
    a = (vcross(x0, dy) - vcross(y0, dy)) / d;
    if (a < 0 || a > 1) 
        return -1;
 
    return 1;
}

/**********************************************
 * Find the shortest distance from x to y0->y1.
 * return distance
 *********************************************/
double distance(vec_t x, vec_t y0, vec_t y1)
{
    vec_t x1;
    vec_t s;
    vec_t dy = vsub(y1, y0);
    int r;

    x1.x = x.x + dy.y;
    x1.y = x.y - dy.x;
    r = intersect(x, x1, y0, y1, &s);
    if (r == -1) 
	return HUGE_VAL;
    
    s = vsub(s, x);

    return sqrt(vdot(s, s));
}

/**********************************************
 * Check if psuedovector (point) is within boundary
 * polygon.
 * Return 1 for in, 0 for on, -1 for out.
 *********************************************/
int boundary_check(double *pos, const polygon_t *p)
{
    vec_t v;
    vec_t *pv;
    vec_t ray;
    int i, j;
    int crosses;
    int ret;
    double xmin, xmax;
    double ymin, ymax;

    /* These may need to be switched!! */
    v.x = pos[0];
    v.y = pos[1];

    for (i=0; i<p->n; i++) {
	j = (i + 1) % p->n;
	xmin = distance(v, p->v[i], p->v[j]);
	if (xmin < TOL)
	    return 0;
    }
    
    xmin = xmax = p->v[0].x;
    ymin = ymax = p->v[1].y;
    for (i=0, pv=p->v; i<p->n; i++, pv++) {
	xmax = (xmax < pv->x) ? pv->x : xmax;
	xmin = (xmin > pv->x) ? pv->x : xmin;
	ymax = (ymax < pv->y) ? pv->y : ymax;
	ymin = (ymin > pv->y) ? pv->y : ymin;
    }

    if (v.x<xmin || v.x>xmax || v.y<ymin || v.y>ymax)
	return -1;

    xmax -= xmin;
    xmax *= 2;
    ymax -= ymin;
    ymax *= 2;
    xmax += ymax;
 
    while (1) {
	crosses = 0;
	ray.x = v.x + (1 + rand() / (RAND_MAX + 1.)) * xmax;
	ray.y = v.y + (1 + rand() / (RAND_MAX + 1.)) * xmax;
    
	for (i=0; i<p->n; i++) {
	    j = (i + 1) % p->n;
	    ret = intersect(v, ray, p->v[i], p->v[j], 0);
	    if (ret == 0)
		break;
	    if (ret == 1)
	        crosses++;
	}
	if (i == p->n)
	    break;
    }
    return (crosses & 1) ? 1 : -1;
}

/***********************************************
 * Read in boundary vertices:
 *  *f = path to boundary file
 *  *latvert = array for latitude vertex values
 *  *lonvert = array for longitude vertex values
 *  returns number of lat/lon pairs read
 *  TODO add some format error checking
 **********************************************/
int read_boundary(const char *f, double *latvert, double *lonvert)
{
    FILE *fp = fopen(f, "r");
    int line = 0;
    double tmplatvert[VERTMAX] = {0};
    double tmplonvert[VERTMAX] = {0};

    if (fp == NULL) {
	fprintf(stderr, "Failed to open file\n");
	return -1;
    }
    
    for (line=0; line<VERTMAX;) {
	if (2 != fscanf(fp, "%lf %lf", &tmplatvert[line], &tmplonvert[line])) {
	    break;
	}
	printf("Read: %lf, %lf\n", tmplatvert[line], tmplonvert[line]);
	line++;
    }
    fclose(fp);
    
    memset(latvert, 0, VERTMAX);
    memset(lonvert, 0, VERTMAX);
    memcpy(latvert, tmplatvert, VERTMAX);
    memcpy(lonvert, tmplonvert, VERTMAX);

    return line;
}

/**********************************************
 * Allocate and deallocate memory for a polygon
 * with n vertices.
 *********************************************/
polygon_t *new_boundary(const int n, double *latvert, double *lonvert)
{
    int i;
    polygon_t *ret = malloc(sizeof(polygon_t));
    if (ret == NULL)
	return NULL;

    ret->v = malloc(n * sizeof(vec_t));
    if (ret->v == NULL) {
	free(ret);
	return NULL;
    }
    for (i=0; i<n; i++) {
	ret->v[i].x = latvert[i];
	ret->v[i].y = lonvert[i];
    }
    ret->n = n;
    return ret;
}

void free_boundary(polygon_t *p)
{
    if (p != NULL) {
	free(p->v);
	free(p);
    }
}
