#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include "boundarycheck.h"
#include "linux_threads.h"
#include "rtk/rtklib.h"
#include "rtkmod.h"

#define MAXSTR 1024
#define OPTSDIR "."
#define OPTSFILE "position.conf"
#define INFILE "UItoFC.txt"
#define OUTFILE "FCtoUI.txt"
#define BNDFILE "Boundary.txt"
#define GPIOPIN 65

/* global variables */
static int r_flag = 0;			/* run flag: 0=stop, 1=start */
static int b_flag = 0;			/* boundary flag: -1=out, 0=on, 1=in */
static int s_flag = 0;			/* shutdown flag to ensure all loops clean up and shutdown */
static double trkpos[3] = {0};		/* tracker position */
static polygon_t *boundary = NULL;	/* boundary polygon */

pthread_mutex_t bndMutex;

int ComLoopFunc(void *argPtr);
int BndLoopFunc(void *argPtr);
int PosLoopFunc(void *argPtr);


int main()
{
    int comLoopID;
    int bndLoopID;
    int posLoopID;
    char setValue[4], GPIOString[4], GPIODirection[64];
    FILE *fd;

    sprintf(GPIOString, "%d", GPIOPIN);
    sprintf(GPIODirection, "/sys/class/gpio/gpio%d/direction", GPIOPIN);

    if ((fd = fopen("/sys/class/gpio/export", "ab")) == NULL) {
        printf("Unable to export GPIO pin\n");
        return 0;
    }
    strcpy(setValue, GPIOString);
    fwrite(&setValue, sizeof(char), 2, fd);
    fclose(fd);
 
    if ((fd = fopen(GPIODirection, "rb+")) == NULL){
        printf("Unable to open direction handle\n");
        return 0;
    }
    strcpy(setValue,"out");
    fwrite(&setValue, sizeof(char), 3, fd);
    fclose(fd);


    pthread_mutex_init(&bndMutex, NULL);
    
    comLoopID = StartHPLoop(&ComLoopFunc, NULL, 100);
    bndLoopID = StartHPLoop(&BndLoopFunc, NULL, 100);
    posLoopID = StartHPLoop(&PosLoopFunc, NULL, 0);

    if (comLoopID<0 || bndLoopID<0 || posLoopID<0) {
        fprintf(stderr, "\nLoops didn't start properly!  Nothing to do but quit...\n");
        fprintf(stderr, "comLoopID = %d\n", comLoopID);
        fprintf(stderr, "bndLoopID = %d\n", bndLoopID);
        fprintf(stderr, "posLoopID = %d\n", posLoopID);
        return -1;
    }

    while(CheckHPLoopState(comLoopID) != HPL_STATE_DONE
	    || CheckHPLoopState(bndLoopID) != HPL_STATE_DONE
            || CheckHPLoopState(posLoopID) != HPL_STATE_DONE);

    pthread_mutex_destroy(&bndMutex);
    if (boundary != NULL) 
	free_boundary(boundary);

    if ((fd = fopen("/sys/class/gpio/unexport", "ab")) == NULL) {
        printf("Unable to unexport GPIO pin\n");
        return 0;
    }
    strcpy(setValue, GPIOString);
    fwrite(&setValue, sizeof(char), 2, fd);
    fclose(fd);

    return 0;
}

/*********************************************
 * ComLoop functions
 ********************************************/
int ComLoopFunc(void *argPtr)
{
    int n;
    int fd;
    double latvert[VERTMAX] = {0};
    double lonvert[VERTMAX] = {0};
    struct flock lock;
    char inbuf[MAXSTR] = {0};
    char outbuf[MAXSTR] = {0};

    /* Open file */
    fd = open(INFILE, O_RDONLY);
    /* set read lock */
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_RDLCK;
    fcntl(fd, F_SETLKW, &lock);
    read(fd, inbuf, MAXSTR);
    /* release lock */
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
    close(fd);
    
    /* Maintain state */
    if (inbuf[0] == '0') {
	;
	/*printf("no-op\n");*/
    }

    /* Start checking against new boundary */
    if (inbuf[0] == '1') {
        printf("start\n");

	/* Instantiate new boundary */
	pthread_mutex_lock(&bndMutex);
	if (boundary != NULL)
		free_boundary(boundary);

	n = read_boundary(BNDFILE, latvert, lonvert);
	if (n < 3) {
	    printf("bad boundary file, %d\n", n);
	    pthread_mutex_unlock(&bndMutex);
	    s_flag = 1;
	    return 0;
	}
	
	if ((boundary = new_boundary(n, latvert, lonvert)) == NULL) {
	    printf("boundary==NULL?\n");
	    pthread_mutex_unlock(&bndMutex);
	    s_flag = 1;
	    return 0;
	}
	r_flag = 1;
	pthread_mutex_unlock(&bndMutex);
    }
	
    /* Exit loop (and program) */
    if (inbuf[0] == '2') {
	printf("stop\n");
	s_flag = 1;
	return 0;
    }
    sprintf(outbuf, "%d %.8f %.8f", b_flag, trkpos[0], trkpos[1]);

    fd = open(OUTFILE, O_WRONLY);
    /* set write lock */
    lock.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &lock);
    write(fd, outbuf, sizeof(outbuf));
    /* release lock */
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
    close(fd);

    if (s_flag == 1)
	return 0;

    return 1;
}
/*********************************************
 * Boundary check and tracker alert loop
 ********************************************/
int BndLoopFunc(void *argPtr)
{
    /* For debugging/testing */
    FILE *fout = NULL;
    FILE *fon = NULL;
    FILE *fin = NULL;
    FILE *fd = NULL;
    char setValue[4], GPIOValue[64];

    sprintf(GPIOValue, "/sys/class/gpio/gpio%d/value", GPIOPIN);

    if (r_flag) {
	pthread_mutex_lock(&bndMutex);
	b_flag = boundary_check(trkpos, boundary);
	pthread_mutex_unlock(&bndMutex);

	printf("%.8f %.8f %.3f\n", trkpos[0], trkpos[1], trkpos[2]);

	if (b_flag == -1) {
	    /* write to solution files */
	    fout = fopen("solution/OUT.txt","a");
	    fprintf(fout, "%.8f %.8f\n", trkpos[0], trkpos[1]);
	    fclose(fout);

	    /* write to xbee gpio */
	    if ((fd = fopen(GPIOValue, "rb+")) == NULL){
		printf("Unable to open value handle\n");
		s_flag = 1;
		return 0;
	    }
	    strcpy(setValue, "0");
	    fwrite(&setValue, sizeof(char), 1, fd);
	    fclose(fd);

	    /* write to stdout */
	    printf("OUT\n");
	}

	if (b_flag == 0) {
	    printf("ON...\n");
	    fon = fopen("solution/ON.txt","a");
	    fprintf(fon, "%.8f %.8f\n", trkpos[0], trkpos[1]);
	    fclose(fon);
	}
	if (b_flag == 1) {
	    fin = fopen("solution/IN.txt","a");
	    fprintf(fin, "%.8f %.8f\n", trkpos[0], trkpos[1]);
	    fclose(fin);

	    if ((fd = fopen(GPIOValue, "rb+")) == NULL){
		printf("Unable to open value handle\n");
		s_flag = 1;
		return 0;
	    }
	    strcpy(setValue, "1");
	    fwrite(&setValue, sizeof(char), 1, fd);
	    fclose(fd);

	    printf("IN\n");
	}
    }

    if (s_flag == 1)
	return 0;

    return 1;
}

/*********************************************
 * Position Loop
 ********************************************/
int PosLoopFunc(void *argPtr)
{
    datacont_t datacont;
    obs_t obs;
    obsd_t data[MAXOBS*2];
    double tt;
    unsigned int tick, ticknmea;
    unsigned char *p, *q;
    int fobs[3]={0}, cycle, cputime;
    int i, j, n;

    printf("PosLoop\n");
    i = init(&datacont);
    if (i==0) {
	printf("RTK init failed\n");
	return 0;
    }

    datacont.state=1; 
    obs.data=data;
    datacont.tick=tickget();
    ticknmea=datacont.tick-1000;
    
    for (cycle=0;datacont.state;cycle++) {
        tick=tickget();
        for (i=0;i<3;i++) {
            p=datacont.buff[i]+datacont.nb[i]; 
	    q=datacont.buff[i]+datacont.buffsize;
            
            /* read receiver raw/rtcm data from input stream */
            if ((n=strread(datacont.stream+i,p,q-p))<=0) {
                continue;
            }
        
	    /* write receiver raw/rtcm data to log stream */
            strwrite(datacont.stream+i+5,p,n);
            datacont.nb[i]+=n;
        
	    /* save peek buffer */
            datacont_lock(&datacont);
            n=n<datacont.buffsize-datacont.npb[i]?n:datacont.buffsize-datacont.npb[i];
            memcpy(datacont.pbuf[i]+datacont.npb[i],p,n);
            datacont.npb[i]+=n;
            datacont_unlock(&datacont);
        }
        for (i=0;i<3;i++) {
            if (datacont.format[i]==STRFMT_SP3||datacont.format[i]==STRFMT_RNXCLK) {
                /* decode download file */
		printf("hello?\n");
            }
            else {
                /* decode receiver raw/rtcm data */
                fobs[i]=decoderaw(&datacont,i);
            }
        }
        for (i=0;i<fobs[0];i++) { /* for each rover observation data */
            obs.n=0;
            for (j=0;j<datacont.obs[0][i].n&&obs.n<MAXOBS*2;j++) {
                obs.data[obs.n++]=datacont.obs[0][i].data[j];
            }
            for (j=0;j<datacont.obs[1][0].n&&obs.n<MAXOBS*2;j++) {
                obs.data[obs.n++]=datacont.obs[1][0].data[j];
            }
        
	    /* rtk positioning */
            datacont_lock(&datacont);
            rtkpos(&datacont.rtk,obs.data,obs.n,&datacont.nav);
            datacont_unlock(&datacont);

            if (datacont.rtk.sol.stat!=SOLQ_NONE) {
                /* adjust current time */
                tt=(int)(tickget()-tick)/1000.0+DTTOL;
                timeset(gpst2utc(timeadd(datacont.rtk.sol.time,tt)));
                
                /* write solution */
		position(&datacont.rtk.sol,trkpos);
                writesol(&datacont,i);
            }
            /* if cpu overload, inclement obs outage counter and break */
            if ((int)(tickget()-tick)>=datacont.cycle) {
                datacont.prcout+=fobs[0]-i-1;
            }
        }
        /* send null solution if no solution (1hz) */
        if (datacont.rtk.sol.stat==SOLQ_NONE&&cycle%(1000/datacont.cycle)==0) {
            writesol(&datacont,0);
        }
        /* send nmea request to base/nrtk input stream */
        if (datacont.nmeacycle>0&&(int)(tick-ticknmea)>=datacont.nmeacycle) {
            if (datacont.stream[1].state==1) {
                if (datacont.nmeareq==1) {
                    strsendnmea(datacont.stream+1,datacont.nmeapos);
                }
                else if (datacont.nmeareq==2&&norm(datacont.rtk.sol.rr,3)>0.0) {
                    strsendnmea(datacont.stream+1,datacont.rtk.sol.rr);
                }
            }
            ticknmea=tick;
        }
        if ((cputime=(int)(tickget()-tick))>0) datacont.cputime=cputime;

        /* sleep until next cycle */
        sleepms(datacont.cycle-cputime);
	/* exit if shutdown flag is set */
	if (s_flag == 1)
	    break;
    }
    freedatacont(&datacont);
    return 0;
}
