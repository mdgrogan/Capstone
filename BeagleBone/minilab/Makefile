CFLAGS += -Wall
LDLIBS += -lprussdrv

all: main.bin pru_enable-00A0.dtbo pwm_P8_36-00A0.dtbo main

main.bin: main.p
	pasm -b $^

main: main.o mio.o

mio.o: mio.c

pru_enable-00A0.dtbo: pru_enable-00A0.dts
	dtc -@ -O dtb -o $@ $^

pwm_P8_36-00A0.dtbo: pwm_P8_36-00A0.dts
	dtc -@ -O dtb -o $@ $^

clean:
	rm -f main *.o *.bin
