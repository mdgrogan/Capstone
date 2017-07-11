#include "rtk/rtklib.h"
#include "rtkmod.h"

#define MAXSTR 1024
#define MAXRCVCMD 4096
#define OPTSDIR "."
#define OPTSFILE "position.conf"

#define MIN(x,y)	((x)<(y)?(x):(y))
#define MAX(x,y)	((x)>(y)?(x):(y))
#define SQRT(x)		((x)<=0.0?0.0:sqrt(x))

/* global variables */
/*static datacont_t datacont;*/
static char passwd[MAXSTR]="admin";     /* login password */
static int timetype     =0;             /* time format (0:gpst,1:utc,2:jst,3:tow) */
static int soltype      =0;             /* sol format (0:dms,1:deg,2:xyz,3:enu,4:pyl) */
static int solflag      =2;             /* sol flag (1:std+2:age/ratio/ns) */
static int strtype[]={                  /* stream types */
    STR_SERIAL,STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE,STR_NONE
};
static char strpath[8][MAXSTR]={""};    /* stream paths */
static int strfmt[]={                   /* stream formats */
    STRFMT_UBX,STRFMT_RTCM3,STRFMT_SP3,SOLF_LLH,SOLF_NMEA
};
static int datacontcycle=10;            /* server cycle (ms) */
static int timeout      =10000;         /* timeout time (ms) */
static int reconnect    =10000;         /* reconnect interval (ms) */
static int nmeacycle    =5000;          /* nmea request cycle (ms) */
static int buffsize     =32768;         /* input buffer size (bytes) */
static int navmsgsel    =0;             /* navigation mesaage select */
static char proxyaddr[256]="";          /* http/ntrip proxy */
static int nmeareq      =0;             /* nmea request type (0:off,1:lat/lon,2:single) */
static double nmeapos[] ={0,0};         /* nmea position (lat/lon) (deg) */
static char rcvcmds[3][MAXSTR]={""};    /* receiver commands files */
static char startcmd[MAXSTR]="";        /* start command */
static char stopcmd [MAXSTR]="";        /* stop command */
static int fswapmargin  =30;            /* file swap margin (s) */

static prcopt_t prcopt;                 /* processing options */
static solopt_t solopt[2]={{0}};        /* solution options */
static filopt_t filopt  ={""};          /* file options */

/* receiver options table ----------------------------------------------------*/
#define TIMOPT  "0:gpst,1:utc,2:jst,3:tow"
#define CONOPT  "0:dms,1:deg,2:xyz,3:enu,4:pyl"
#define FLGOPT  "0:off,1:std+2:age/ratio/ns"
#define ISTOPT  "0:off,1:serial,2:file,3:tcpdatacont,4:tcpcli,7:ntripcli,8:ftp,9:http"
#define OSTOPT  "0:off,1:serial,2:file,3:tcpdatacont,4:tcpcli,6:ntripdatacont"
#define FMTOPT  "0:rtcm2,1:rtcm3,2:oem4,3:oem3,4:ubx,5:ss2,6:hemis,7:skytraq,8:gw10,9:javad,10:nvs,15:sp3"
#define NMEOPT  "0:off,1:latlon,2:single"
#define SOLOPT  "0:llh,1:xyz,2:enu,3:nmea"
#define MSGOPT  "0:all,1:rover,2:base,3:corr"

static opt_t rcvopts[]={
    {"console-passwd",  2,  (void *)passwd,              ""     },
    {"console-timetype",3,  (void *)&timetype,           TIMOPT },
    {"console-soltype", 3,  (void *)&soltype,            CONOPT },
    {"console-solflag", 0,  (void *)&solflag,            FLGOPT },

    {"inpstr1-type",    3,  (void *)&strtype[0],         ISTOPT },
    {"inpstr2-type",    3,  (void *)&strtype[1],         ISTOPT },
    {"inpstr3-type",    3,  (void *)&strtype[2],         ISTOPT },
    {"inpstr1-path",    2,  (void *)strpath [0],         ""     },
    {"inpstr2-path",    2,  (void *)strpath [1],         ""     },
    {"inpstr3-path",    2,  (void *)strpath [2],         ""     },
    {"inpstr1-format",  3,  (void *)&strfmt [0],         FMTOPT },
    {"inpstr2-format",  3,  (void *)&strfmt [1],         FMTOPT },
    {"inpstr3-format",  3,  (void *)&strfmt [2],         FMTOPT },
    {"inpstr2-nmeareq", 3,  (void *)&nmeareq,            NMEOPT },
    {"inpstr2-nmealat", 1,  (void *)&nmeapos[0],         "deg"  },
    {"inpstr2-nmealon", 1,  (void *)&nmeapos[1],         "deg"  },
    {"outstr1-type",    3,  (void *)&strtype[3],         OSTOPT },
    {"outstr2-type",    3,  (void *)&strtype[4],         OSTOPT },
    {"outstr1-path",    2,  (void *)strpath [3],         ""     },
    {"outstr2-path",    2,  (void *)strpath [4],         ""     },
    {"outstr1-format",  3,  (void *)&strfmt [3],         SOLOPT },
    {"outstr2-format",  3,  (void *)&strfmt [4],         SOLOPT },
    {"logstr1-type",    3,  (void *)&strtype[5],         OSTOPT },
    {"logstr2-type",    3,  (void *)&strtype[6],         OSTOPT },
    {"logstr3-type",    3,  (void *)&strtype[7],         OSTOPT },
    {"logstr1-path",    2,  (void *)strpath [5],         ""     },
    {"logstr2-path",    2,  (void *)strpath [6],         ""     },
    {"logstr3-path",    2,  (void *)strpath [7],         ""     },

    {"misc-datacontcycle",   0,  (void *)&datacontcycle,           "ms"   },
    {"misc-timeout",    0,  (void *)&timeout,            "ms"   },
    {"misc-reconnect",  0,  (void *)&reconnect,          "ms"   },
    {"misc-nmeacycle",  0,  (void *)&nmeacycle,          "ms"   },
    {"misc-buffsize",   0,  (void *)&buffsize,           "bytes"},
    {"misc-navmsgsel",  3,  (void *)&navmsgsel,          MSGOPT },
    {"misc-proxyaddr",  2,  (void *)proxyaddr,           ""     },
    {"misc-fswapmargin",0,  (void *)&fswapmargin,        "s"    },

    {"misc-startcmd",   2,  (void *)startcmd,            ""     },
    {"misc-stopcmd",    2,  (void *)stopcmd,             ""     },

    {"file-cmdfile1",   2,  (void *)rcvcmds[0],          ""     },
    {"file-cmdfile2",   2,  (void *)rcvcmds[1],          ""     },
    {"file-cmdfile3",   2,  (void *)rcvcmds[2],          ""     },

    {"",0,NULL,""}
};

/* lock/unlock data container */
extern void datacont_lock(datacont_t *datacont){lock(&datacont->lock);}
extern void datacont_unlock(datacont_t *datacont){unlock(&datacont->lock);}

/* write solution header to output stream ------------------------------------*/
static void writesolhead(stream_t *stream, const solopt_t *solopt){
    unsigned char buff[1024];
    int n;

    n=outsolheads(buff,solopt);
    strwrite(stream,buff,n);
}

/* save output buffer --------------------------------------------------------*/
static void saveoutbuf(datacont_t *datacont, unsigned char *buff, int n, int index)
{
    datacont_lock(datacont);

    n=n<datacont->buffsize-datacont->nsb[index]?n:datacont->buffsize-datacont->nsb[index];
    memcpy(datacont->sbuf[index]+datacont->nsb[index],buff,n);
    datacont->nsb[index]+=n;

    datacont_unlock(datacont);
}

/* write solution to output stream -------------------------------------------*/
void writesol(datacont_t *datacont, int index)
{
    unsigned char buff[1024];
    int i,n;

    for (i=0;i<2;i++) {
	/* output solution */
	n=outsols(buff,&datacont->rtk.sol,datacont->rtk.rb,datacont->solopt+i);
	strwrite(datacont->stream+i+3,buff,n);

	/* save output buffer */
	saveoutbuf(datacont,buff,n,i);

	/* output extended solution */
	n=outsolexs(buff,&datacont->rtk.sol,datacont->rtk.ssat,datacont->solopt+i);
	strwrite(datacont->stream+i+3,buff,n);

	/* save output buffer */
	saveoutbuf(datacont,buff,n,i);
    }
    /* save solution buffer */
    if (datacont->nsol<MAXSOLBUF) {
	datacont_lock(datacont);
	datacont->solbuf[datacont->nsol++]=datacont->rtk.sol;
	datacont_unlock(datacont);
    }
}

/* update navigation data ----------------------------------------------------*/
static void updatenav(nav_t *nav)
{
    int i,j;
    for (i=0;i<MAXSAT;i++) for (j=0;j<NFREQ;j++) {
	nav->lam[i][j]=satwavelen(i+1,j,nav);
    }
}

/* update data content struct --------------------------------------------------*/
static void updatedatacont(datacont_t *datacont, int ret, obs_t *obs, nav_t *nav, int sat,
	sbsmsg_t *sbsmsg, int index, int iobs)
{
    eph_t *eph1,*eph2,*eph3;
    geph_t *geph1,*geph2,*geph3;
    gtime_t tof;
    double pos[3],del[3]={0},dr[3];
    int i,n=0,prn,sbssat=datacont->rtk.opt.sbassatsel;
    int sys,iode;

    if (ret==1) { /* observation data */
	if (iobs<MAXOBSBUF) {
	    for (i=0;i<obs->n;i++) {
		if (datacont->rtk.opt.exsats[obs->data[i].sat-1]==1||
			!(satsys(obs->data[i].sat,NULL)&datacont->rtk.opt.navsys)) continue;
		datacont->obs[index][iobs].data[n]=obs->data[i];
		datacont->obs[index][iobs].data[n++].rcv=index+1;
	    }
	    datacont->obs[index][iobs].n=n;
	    sortobs(&datacont->obs[index][iobs]);
	}
	datacont->nmsg[index][0]++;
    }
    else if (ret==2) { /* ephemeris */
	if (satsys(sat,&prn)!=SYS_GLO) {
	    if (!datacont->navsel||datacont->navsel==index+1) {
		eph1=nav->eph+sat-1;
		eph2=datacont->nav.eph+sat-1;
		eph3=datacont->nav.eph+sat-1+MAXSAT;
		if (eph2->ttr.time==0||
			(timediff(eph1->toe,eph2->toe)>0.0&&eph1->iode!=eph2->iode)) {
		    *eph3=*eph2;
		    *eph2=*eph1;
		    updatenav(&datacont->nav);
		}
	    }
	    datacont->nmsg[index][1]++;
	}
	else {
	    if (!datacont->navsel||datacont->navsel==index+1) {
		geph1=nav->geph+prn-1;
		geph2=datacont->nav.geph+prn-1;
		geph3=datacont->nav.geph+prn-1+MAXPRNGLO;
		if (geph2->tof.time==0||
			(timediff(geph1->toe,geph2->toe)>0.0&&geph1->iode!=geph2->iode)) {
		    *geph3=*geph2;
		    *geph2=*geph1;
		    updatenav(&datacont->nav);
		}
	    }
	    datacont->nmsg[index][6]++;
	}
    }
    else if (ret==3) { /* sbas message */
	if (sbsmsg&&(sbssat==sbsmsg->prn||sbssat==0)) {
	    if (datacont->nsbs<MAXSBSMSG) {
		datacont->sbsmsg[datacont->nsbs++]=*sbsmsg;
	    }
	    else {
		for (i=0;i<MAXSBSMSG-1;i++) datacont->sbsmsg[i]=datacont->sbsmsg[i+1];
		datacont->sbsmsg[i]=*sbsmsg;
	    }
	    sbsupdatecorr(sbsmsg,&datacont->nav);
	}
	datacont->nmsg[index][3]++;
    }
    else if (ret==9) { /* ion/utc parameters */
	if (datacont->navsel==index||datacont->navsel>=3) {
	    for (i=0;i<8;i++) datacont->nav.ion_gps[i]=nav->ion_gps[i];
	    for (i=0;i<4;i++) datacont->nav.utc_gps[i]=nav->utc_gps[i];
	    for (i=0;i<4;i++) datacont->nav.ion_gal[i]=nav->ion_gal[i];
	    for (i=0;i<4;i++) datacont->nav.utc_gal[i]=nav->utc_gal[i];
	    for (i=0;i<8;i++) datacont->nav.ion_qzs[i]=nav->ion_qzs[i];
	    for (i=0;i<4;i++) datacont->nav.utc_qzs[i]=nav->utc_qzs[i];
	    datacont->nav.leaps=nav->leaps;
	}
	datacont->nmsg[index][2]++;
    }
    else if (ret==5) { /* antenna postion parameters */
	if (datacont->rtk.opt.refpos==4&&index==1) {
	    for (i=0;i<3;i++) {
		datacont->rtk.rb[i]=datacont->rtcm[1].sta.pos[i];
	    }
	    /* antenna delta */
	    ecef2pos(datacont->rtk.rb,pos);
	    if (datacont->rtcm[1].sta.deltype) { /* xyz */
		del[2]=datacont->rtcm[1].sta.hgt;
		enu2ecef(pos,del,dr);
		for (i=0;i<3;i++) {
		    datacont->rtk.rb[i]+=datacont->rtcm[1].sta.del[i]+dr[i];
		}
	    }
	    else { /* enu */
		enu2ecef(pos,datacont->rtcm[1].sta.del,dr);
		for (i=0;i<3;i++) {
		    datacont->rtk.rb[i]+=dr[i];
		}
	    }
	}
	datacont->nmsg[index][4]++;
    }
    else if (ret==7) { /* dgps correction */
	datacont->nmsg[index][5]++;
    }
    else if (ret==10) { /* ssr message */
	for (i=0;i<MAXSAT;i++) {
	    if (!datacont->rtcm[index].ssr[i].update) continue;
	    datacont->rtcm[index].ssr[i].update=0;
	    iode=datacont->rtcm[index].ssr[i].iode;
	    sys=satsys(i+1,&prn);

	    /* check corresponding ephemeris exists */
	    if (sys==SYS_GPS||sys==SYS_GAL||sys==SYS_QZS) {
		if (datacont->nav.eph[i].iode!=iode&&
			datacont->nav.eph[i+MAXSAT].iode!=iode) {
		    continue;
		}
	    }
	    else if (sys==SYS_GLO) {
		if (datacont->nav.geph[prn-1].iode!=iode&&
			datacont->nav.geph[prn-1+MAXPRNGLO].iode!=iode) {
		    continue;
		}
	    }
	    datacont->nav.ssr[i]=datacont->rtcm[index].ssr[i];
	}
	datacont->nmsg[index][7]++;
    }
    else if (ret==31) { /* lex message */
	lexupdatecorr(&datacont->raw[index].lexmsg,&datacont->nav,&tof);
	datacont->nmsg[index][8]++;
    }
    else if (ret==-1) { /* error */
	datacont->nmsg[index][9]++;
    }
}

/* decode receiver raw/rtcm data ---------------------------------------------*/
int decoderaw(datacont_t *datacont, int index)
{
    obs_t *obs;
    nav_t *nav;
    sbsmsg_t *sbsmsg=NULL;
    int i,ret,sat,fobs=0;

    datacont_lock(datacont);

    for (i=0;i<datacont->nb[index];i++) {
	/* input rtcm/receiver raw data from stream */
	if (datacont->format[index]==STRFMT_RTCM2) {
	    ret=input_rtcm2(datacont->rtcm+index,datacont->buff[index][i]);
	    obs=&datacont->rtcm[index].obs;
	    nav=&datacont->rtcm[index].nav;
	    sat=datacont->rtcm[index].ephsat;
	}
	else if (datacont->format[index]==STRFMT_RTCM3) {
	    ret=input_rtcm3(datacont->rtcm+index,datacont->buff[index][i]);
	    obs=&datacont->rtcm[index].obs;
	    nav=&datacont->rtcm[index].nav;
	    sat=datacont->rtcm[index].ephsat;
	}
	else {
	    ret=input_raw(datacont->raw+index,datacont->format[index],datacont->buff[index][i]);
	    obs=&datacont->raw[index].obs;
	    nav=&datacont->raw[index].nav;
	    sat=datacont->raw[index].ephsat;
	    sbsmsg=&datacont->raw[index].sbsmsg;
	}
	/*printf("decoderaw ret=%d\n",ret);*/
	/* update data container */
	if (ret>0) updatedatacont(datacont,ret,obs,nav,sat,sbsmsg,index,fobs);

	/* observation data received */
	if (ret==1) {
	    if (fobs<MAXOBSBUF) fobs++; else datacont->prcout++;
	}
    }
    datacont->nb[index]=0;

    datacont_unlock(datacont);

    return fobs;
}




/* read receiver commands-----------------
 * args: char *file, char *cmd, int type
 * return: status (0:error, 1:ok)
 * --------------------------------------*/
static int readcmd(const char *file,char *cmd,int type){ 
    FILE *fp;
    char buff[MAXSTR],*p=cmd;
    int i=0;

    printf("readcmd\n");

    if(!(fp=fopen(file,"r"))) return 0;

    while(fgets(buff,sizeof(buff),fp)){
	if(*buff=='@') i=1;
	else if(i==type&&p+strlen(buff)+1<cmd+MAXRCVCMD){
	    p+=sprintf(p,"%s",buff);
	}
    }
    fclose(fp);
    return 1;
}

/* Get position of rover in decimal degrees
 * ---------------------------------------*/
int position(const sol_t *sol, double *pos){
    double dms1[3],dms2[3];
    
    ecef2pos(sol->rr,pos);

    pos[0]=pos[0]*R2D;
    pos[1]=pos[1]*R2D;
    pos[2]-=geoidh(pos);
    
    return 0;
}

/* Populate data container ----------------
 * args: void
 * return: status (0:error, 1:ok)
 * --------------------------------------*/
int populatedatacont(datacont_t *datacont){ 
    gtime_t time,time0={0};
    double pos[3],npos[3];
    char s[3][MAXRCVCMD]={"","",""},*cmds[]={NULL,NULL,NULL};
    char *paths[]={strpath[0],strpath[1],strpath[2],strpath[3],
	strpath[4],strpath[5],strpath[6],strpath[7]};
    int i,j,rw,stropt[8]={0};

    /* read start commands from command file */
    for(i=0;i<3;i++){
	if(!*rcvcmds[i]) continue;
	if(!readcmd(rcvcmds[i],s[i],0)){
	    printf("no comand file: %s\n",rcvcmds[i]);
	}
	else cmds[i]=s[i];
    }

    if(prcopt.refpos==4){ /* rtcm */
	for(i=0;i<3;i++) prcopt.rb[i]=0.0;
    }
    pos[0]=nmeapos[0]*D2R;
    pos[1]=nmeapos[1]*D2R;
    pos[2]=0.0;
    pos2ecef(pos,npos);

    if(solopt[0].geoid>0&&!opengeoid(solopt[0].geoid,filopt.geoid)){
	printf("geoid data open error: %s\n",filopt.geoid);
    }

    stropt[0]=timeout;;
    stropt[1]=reconnect;;
    stropt[2]=1000;
    stropt[3]=buffsize;
    stropt[4]=fswapmargin;
    strsetopt(stropt);

    if(strfmt[2]==8) strfmt[2]=STRFMT_SP3;

    solopt[0].posf=strfmt[3];
    solopt[1].posf=strfmt[4];

    strinitcom();
    datacont->cycle=datacontcycle>1?datacontcycle:1;
    datacont->nmeacycle=nmeacycle>1000?nmeacycle:1000;
    datacont->nmeareq=nmeareq;
    for (i=0;i<3;i++) datacont->nmeapos[i]=npos[i];
    datacont->buffsize=buffsize>4096?buffsize:4096;
    for (i=0;i<3;i++) datacont->format[i]=strfmt[i];
    datacont->navsel=navmsgsel;
    datacont->nsbs=0;
    datacont->nsol=0;
    datacont->prcout=0;
    rtkfree(&datacont->rtk);
    rtkinit(&datacont->rtk,&prcopt);

    for (i=0;i<3;i++) { /* input/log streams */
	datacont->nb[i]=datacont->npb[i]=0;
	if (!(datacont->buff[i]=(unsigned char *)malloc(buffsize))||
		!(datacont->pbuf[i]=(unsigned char *)malloc(buffsize))) {
	    printf("input/log stream malloc error\n"); 
	    return 0;
	}
	for (j=0;j<10;j++) datacont->nmsg[i][j]=0;
	for (j=0;j<MAXOBSBUF;j++) datacont->obs[i][j].n=0;

	/* initialize receiver raw and rtcm control */
	init_raw (datacont->raw +i);
	init_rtcm(datacont->rtcm+i);

	/* connect dgps corrections */
	datacont->rtcm[i].dgps=datacont->nav.dgps;
    }
    for (i=0;i<2;i++) { /* output peek buffer */
	if (!(datacont->sbuf[i]=(unsigned char *)malloc(buffsize))) {
	    printf("peek buffer malloc error\n");
	    return 0;
	}
    }
    /* set solution options */
    for (i=0;i<2;i++) {
	datacont->solopt[i]=solopt[i];
    }
    /* set base station position */
    for (i=0;i<6;i++) {
	datacont->rtk.rb[i]=i<3?prcopt.rb[i]:0.0;
    }

    /* update navigation data */
    for (i=0;i<MAXSAT *2;i++) datacont->nav.eph [i].ttr=time0;
    for (i=0;i<NSATGLO*2;i++) datacont->nav.geph[i].tof=time0;
    for (i=0;i<NSATSBS*2;i++) datacont->nav.seph[i].tof=time0;
    updatenav(&datacont->nav);

    /* open streams */
    for (i=0;i<8;i++) {
	rw=i<3?STR_MODE_R:STR_MODE_W;
	if (strtype[i]!=STR_FILE) rw|=STR_MODE_W;
	if (!stropen(datacont->stream+i,strtype[i],rw,paths[i])) {
	    for (i--;i>=0;i--) strclose(datacont->stream+i);
	    printf("open input streams failed\n"); 
	    return 0;
	}

	if (i<3) {
	    time=utc2gpst(timeget());
	    datacont->raw [i].time=strtype[i]==STR_FILE?strgettime(datacont->stream+i):time;
	    datacont->rtcm[i].time=strtype[i]==STR_FILE?strgettime(datacont->stream+i):time;
	}
    }
    /* sync input streams */
    strsync(datacont->stream,datacont->stream+1);
    strsync(datacont->stream,datacont->stream+2);

    /* write start commands to input streams */
    for (i=0;i<3;i++) {
	if (cmds[i]) strsendcmd(datacont->stream+i,cmds[i]);
    }
    /* write solution header to solution streams */
    for (i=3;i<5;i++) {
	writesolhead(datacont->stream+i,datacont->solopt+i-3);
    }
    return 1;
}

/* initialize data container--------------
 * args: datacont_t *datacont
 * return: status (0:error, 1:ok)
 * --------------------------------------*/
int initdatacont(datacont_t *datacont){ 
    gtime_t time0={0};
    sol_t sol0={{0}};
    eph_t eph0={0,-1,-1};
    geph_t geph0={0,-1};
    seph_t seph0={0};
    int i,j;

    datacont->state=datacont->cycle=datacont->nmeacycle=datacont->nmeareq=0;

    for(i=0;i<3;i++) datacont->nmeapos[i]=0.0;
    datacont->buffsize=0;
    for(i=0;i<3;i++) datacont->format[i]=0;
    for(i=0;i<2;i++) datacont->solopt[i]=solopt_default;
    datacont->navsel=datacont->nsbs=datacont->nsol=0;
    rtkinit(&datacont->rtk, &prcopt_default);
    for(i=0;i<3;i++) datacont->nb[i]=0;
    for(i=0;i<2;i++) datacont->nsb[i]=0;
    for(i=0;i<3;i++) datacont->npb[i]=0;
    for(i=0;i<3;i++) datacont->buff[i]=NULL;
    for(i=0;i<2;i++) datacont->sbuf[i]=NULL;
    for(i=0;i<3;i++) datacont->pbuf[i]=NULL;
    for(i=0;i<MAXSOLBUF;i++) datacont->solbuf[i]=sol0;
    for(i=0;i<3;i++) for(j=0;j<10;j++) datacont->nmsg[i][j]=0;
    for(i=0;i<3;i++) datacont->ftime[i]=time0;
    for(i=0;i<3;i++) datacont->files[i][0]='\0';
    datacont->tick=0;
    datacont->cputime=datacont->prcout=0;

    if(!(datacont->nav.eph=(eph_t *)malloc(sizeof(eph_t)*MAXSAT*2))||
	    !(datacont->nav.geph=(geph_t *)malloc(sizeof(geph_t)*NSATGLO*2))||
	    !(datacont->nav.seph=(seph_t *)malloc(sizeof(seph_t)*NSATSBS*2))){
	printf("datacontinit malloc error\n");
	return 0;
    }
    for(i=0;i<MAXSAT*2;i++) datacont->nav.eph[i]=eph0;
    for(i=0;i<NSATGLO*2;i++) datacont->nav.geph[i]=geph0;
    for(i=0;i<NSATSBS*2;i++) datacont->nav.seph[i]=seph0;
    datacont->nav.n=MAXSAT*2;
    datacont->nav.ng=NSATGLO*2;
    datacont->nav.ns=NSATSBS*2;

    for(i=0;i<3;i++) for(j=0;j<MAXOBSBUF;j++){
	if(!(datacont->obs[i][j].data=(obsd_t *)malloc(sizeof(obsd_t)*MAXOBS))){
	    printf("datacontinit malloc error\n");
	    return 0;
	}
    }
    for(i=0;i<MAXSTRRTK;i++) strinit(datacont->stream+i);

    initlock(&datacont->lock);

    return 1;
}

int init(datacont_t *datacont)
{
    printf("init\n");
    char file[MAXSTR]="";

    if (initdatacont(datacont)==0)
	return 0;

    /* load configuration settings */
    if(!*file) sprintf(file, "%s/%s", OPTSDIR, OPTSFILE);

    /* initialize configuration to null */
    resetsysopts();
    if(!loadopts(file,rcvopts)||!loadopts(file,sysopts)){
	printf("no options file: %s. defaults used\n",file);
    }
    getsysopts(&prcopt,solopt,&filopt);

    if (populatedatacont(datacont)==0)
	return 0;

    return 1;
}

void freedatacont(datacont_t *datacont)
{
    ;
}
