# Sysmon - Phase 3 (No Pthreads)

This version handles different sensor intervals without using threads.

- mem_sensor - updates every 1 second.
- cpu_sensor - updates every 3 seconds.
- proc_sensor - updates every 5 seconds.

Sysmond uses poll() to read and print new data when it arrives, without waiting for the slowest sensor.

## Makefile

- `make` - compiles all programs.
- `make run` - starts sysmond and the sensors.
- `make clean` - removes compiled programs.

Press Ctrl+C to stop everything.

## Changes

- Removed pthreads and mutex.
- Used poll() instead of reader threads.
- Kept pipes and FIFOs for communication.
- Added a cleanup() function instead of goto.
