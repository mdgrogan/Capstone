BIN = .
SRC = ../src

CTARGET = -DENAGLO -DENAQZS -DNFREQ=3
CFLAGS = -Wall -ggdb3 -O3 -Wcast-align -ansi -pedantic -I$(SRC) -DTRACE $(CTARGET)
LDLIBS  = -lm -lpthread

all	: main
main	: main.o linux_threads.o boundarycheck.o rtkmod.o
main	: rtkcmn.o rtkpos.o geoid.o solution.o 
main	: lambda.o sbas.o stream.o rcvraw.o
main	: rtcm.o preceph.o options.o pntpos.o
main	: ppp.o ppp_ar.o novatel.o ublox.o
main	: ss2.o crescent.o skytraq.o gw10.o
main	: javad.o nvs.o ephemeris.o rinex.o
main	: ionex.o rtcm2.o rtcm3.o rtcm3e.o 
main	: stec.o rtkpos_gsi.o

main.o		: $(SRC)/main.c
	$(CC) -c $(CFLAGS) $(SRC)/main.c
linux_threads.o	: $(SRC)/linux_threads.c
	$(CC) -c $(CFLAGS) -D_GNU_SOURCE $(SRC)/linux_threads.c
boundarycheck.o	: $(SRC)/boundarycheck.c
	$(CC) -c $(CFLAGS) $(SRC)/boundarycheck.c
rtkmod.o	: $(SRC)/rtkmod.c
	$(CC) -c $(CFLAGS) $(SRC)/rtkmod.c
rtkcmn.o	: $(SRC)/rtk/rtkcmn.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/rtkcmn.c
rtkpos.o	: $(SRC)/rtk/rtkpos.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/rtkpos.c
geoid.o		: $(SRC)/rtk/geoid.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/geoid.c
solution.o	: $(SRC)/rtk/solution.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/solution.c
lambda.o	: $(SRC)/rtk/lambda.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/lambda.c
sbas.o     	: $(SRC)/rtk/sbas.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/sbas.c
stream.o   	: $(SRC)/rtk/stream.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/stream.c
rcvraw.o	: $(SRC)/rtk/rcvraw.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/rcvraw.c
rtcm.o		: $(SRC)/rtk/rtcm.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/rtcm.c
rtcm2.o		: $(SRC)/rtk/rtcm2.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/rtcm2.c
rtcm3.o		: $(SRC)/rtk/rtcm3.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/rtcm3.c
rtcm3e.o	: $(SRC)/rtk/rtcm3e.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/rtcm3e.c
preceph.o	: $(SRC)/rtk/preceph.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/preceph.c
options.o	: $(SRC)/rtk/options.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/options.c
pntpos.o	: $(SRC)/rtk/pntpos.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/pntpos.c
ppp.o		: $(SRC)/rtk/ppp.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/ppp.c
ppp_ar.o	: $(SRC)/rtk/ppp_ar.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/ppp_ar.c
novatel.o	: $(SRC)/rtk/novatel.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/novatel.c
ublox.o		: $(SRC)/rtk/ublox.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/ublox.c
ss2.o		: $(SRC)/rtk/ss2.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/ss2.c
crescent.o	: $(SRC)/rtk/crescent.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/crescent.c
skytraq.o	: $(SRC)/rtk/skytraq.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/skytraq.c
gw10.o		: $(SRC)/rtk/gw10.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/gw10.c
javad.o		: $(SRC)/rtk/javad.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/javad.c
nvs.o		: $(SRC)/rtk/nvs.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/nvs.c
ephemeris.o	: $(SRC)/rtk/ephemeris.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/ephemeris.c
rinex.o		: $(SRC)/rtk/rinex.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/rinex.c
ionex.o		: $(SRC)/rtk/ionex.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/ionex.c
stec.o		: $(SRC)/rtk/stec.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/stec.c
rtkpos_gsi.o	: $(SRC)/rtk/rtkpos_gsi.c
	$(CC) -c $(CFLAGS) $(SRC)/rtk/rtkpos_gsi.c


main.o 		: $(SRC)/rtk/rtklib.h $(SRC)/rtkmod.h $(SRC)/linux_threads.h $(SRC)/boundarycheck.h
linux_threads.o	: $(SRC)/linux_threads.h
boundarycheck.o	: $(SRC)/boundarycheck.h
rtkmod.o	: $(SRC)/rtkmod.h
rtkcmn.o   	: $(SRC)/rtk/rtklib.h
rtkpos.o   	: $(SRC)/rtk/rtklib.h
geoid.o    	: $(SRC)/rtk/rtklib.h
solution.o 	: $(SRC)/rtk/rtklib.h
lambda.o   	: $(SRC)/rtk/rtklib.h
sbas.o     	: $(SRC)/rtk/rtklib.h
rcvraw.o   	: $(SRC)/rtk/rtklib.h
rtcm.o     	: $(SRC)/rtk/rtklib.h
rtcm2.o    	: $(SRC)/rtk/rtklib.h
rtcm3.o    	: $(SRC)/rtk/rtklib.h
rtcm3e.o   	: $(SRC)/rtk/rtklib.h
preceph.o  	: $(SRC)/rtk/rtklib.h
options.o  	: $(SRC)/rtk/rtklib.h
pntpos.o   	: $(SRC)/rtk/rtklib.h
ppp.o      	: $(SRC)/rtk/rtklib.h
novatel.o  	: $(SRC)/rtk/rtklib.h
ublox.o    	: $(SRC)/rtk/rtklib.h
ss2.o      	: $(SRC)/rtk/rtklib.h
crescent.o 	: $(SRC)/rtk/rtklib.h
skytraq.o  	: $(SRC)/rtk/rtklib.h
gw10.o     	: $(SRC)/rtk/rtklib.h
javad.o    	: $(SRC)/rtk/rtklib.h
ephemeris.o	: $(SRC)/rtk/rtklib.h
rinex.o    	: $(SRC)/rtk/rtklib.h
ionex.o    	: $(SRC)/rtk/rtklib.h
stec.o     	: $(SRC)/rtk/rtklib.h
rtkpos_gsi.o	: $(SRC)/rtk/rtklib.h

all:
	cp main ..
clean:
	rm -f main $(BIN)/*.o

