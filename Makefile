CC = gcc
CFLAGS = -Wall -g

all: bin/mem_sensor bin/cpu_sensor bin/proc_sensor bin/sysmond

bin/mem_sensor: src/mem_sensor.c
	$(CC) $(CFLAGS) src/mem_sensor.c -o bin/mem_sensor

bin/cpu_sensor: src/cpu_sensor.c
	$(CC) $(CFLAGS) src/cpu_sensor.c -o bin/cpu_sensor

bin/proc_sensor: src/proc_sensor.c
	$(CC) $(CFLAGS) src/proc_sensor.c -o bin/proc_sensor

bin/sysmond: src/sysmond.c
	$(CC) $(CFLAGS) src/sysmond.c -o bin/sysmond

clean:
	rm -f bin/mem_sensor bin/cpu_sensor bin/proc_sensor bin/sysmond

run: all
	bash run.sh
