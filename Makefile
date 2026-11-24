
CC = gcc
CFLAGS = -Wall -g -Iinc

all: bin/mem_sensor bin/cpu_sensor bin/proc_sensor bin/sysmond

bin/mem_sensor: src/mem_sensor.c inc/sysmon_ipc.h
	mkdir -p bin
	$(CC) $(CFLAGS) src/mem_sensor.c -o bin/mem_sensor

bin/cpu_sensor: src/cpu_sensor.c inc/sysmon_ipc.h
	$(CC) $(CFLAGS) src/cpu_sensor.c -o bin/cpu_sensor

bin/proc_sensor: src/proc_sensor.c inc/sysmon_ipc.h
	$(CC) $(CFLAGS) src/proc_sensor.c -o bin/proc_sensor

bin/sysmond: src/sysmond.c inc/sysmon_ipc.h
	$(CC) $(CFLAGS) -pthread src/sysmond.c -o bin/sysmond

run: all
	./bin/sysmond

clean:
	rm -f bin/mem_sensor bin/cpu_sensor bin/proc_sensor bin/sysmond
