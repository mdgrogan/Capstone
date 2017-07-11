#include "rtk/rtklib.h"

/* data container */
typedef struct{
	int state;				/* 0:off, 1:running */
	int cycle;				/* processing cycle (ms) */
	int nmeacycle;			/* NMEA request cycle (ms) (0:no req) */
	int nmeareq;			/* NMEA request (0:no, 1:nmeapos, 2:single sol) */
	double nmeapos[3];		/* NMEA requiest postion (ECEF, m) */
	int buffsize;			/* input buffer size (bytes) */
	int format[3];			/* input format {rov,base,corr} */
	solopt_t solopt[2];		/* output solution options {sol1,sol2} */
	int navsel;				/* ephemeris select (0:all, 1:rover, 2:base, 3:corr) */
	int nsbs;				/* number of sbas message */
	int nsol;				/* number of solution buffer */
	rtk_t rtk;				/* RTK control/result sruct */
	int nb[3];				/* bytes in input buffers {rov,base} */
	int nsb[2];				/* bytes in solution buffers */
	int npb[3];				/* bytes in input peek buffers */
	unsigned char *buff[3];	/* input buffers {rov,base,corr} */
	unsigned char *sbuf[2];	/* output buffers {sol1,sol2} */
	unsigned char *pbuf[3];	/* peek buffers {rov,base,corr} */
	sol_t solbuf[MAXSOLBUF];/* solution buffer */
	unsigned int nmsg[3][10];/* input message counts */
	raw_t raw[3];			/* receiver raw control {rov,base,corr} */
	rtcm_t rtcm[3];			/* RTCM control {rov,base,corr} */
	gtime_t ftime[3];		/* download time {rov,base,corr} */
	char files[3][MAXSTRPATH];/* download paths {rov,base,corr} */
	obs_t obs[3][MAXOBSBUF];/* observation data {rov,base,corr} */
	nav_t nav;				/* navigation data */
	sbsmsg_t sbsmsg[MAXSBSMSG];/* SBAS message buffer */
	stream_t stream[8];		/* streams {rov,base,corr,sol1,sol2,logr,logb,logc} */
	unsigned int tick;		/* start tick */
	int cputime;			/* CPU time (ms) for a processing cycle */
	int prcout;				/* missing observation data count */
	lock_t lock;			/* lock flag */
} datacont_t;

void datacont_lock(datacont_t *datacont);
void datacont_unlock(datacont_t *datacont);
int position(const sol_t *sol, double *pos);
int decoderaw(datacont_t *datacont, int index);
void writesol(datacont_t *datacont, int index);
int init(datacont_t *datacont);
void freedatacont(datacont_t *datacont);
