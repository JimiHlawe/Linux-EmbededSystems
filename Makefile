CC = gcc
CFLAGS = -Wall -g -Iinc -pthread

all: bin/sysmond

bin/sysmond: src/sysmond.c src/mem_sensor.c src/cpu_sensor.c src/proc_sensor.c inc/sysmon.h
	mkdir -p bin
	$(CC) $(CFLAGS) src/sysmond.c src/mem_sensor.c src/cpu_sensor.c src/proc_sensor.c -o bin/sysmond

run: all
	./bin/sysmond

clean:
	rm -f bin/sysmond bin/mem_sensor bin/cpu_sensor bin/proc_sensor
