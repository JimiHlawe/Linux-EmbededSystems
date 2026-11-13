# Sysmon - Phase 1

This is a simple Linux system monitor written in C.

It has three sensors:
- `mem_sensor` - reads memory information.
- `cpu_sensor` - reads CPU load.
- `proc_sensor` - counts processes.

Each sensor reads data from `/proc` and saves it in `/tmp/sysmon`. The `sysmond` program reads these files and prints a report every 2 seconds.

## How to Run

First, create the needed folders:

```bash
mkdir -p bin /tmp/sysmon
```

Then run:

```bash
make run
```

Press **Ctrl+C** to stop everything.

## Makefile

- `make` - compiles all four programs.
- `make run` - compiles if needed and starts everything using `run.sh`.
- `make clean` - removes the compiled programs.

## What I Added

I added `run.sh` to start and stop all processes together instead of doing it manually.

I also used temporary files before updating the `.dat` files, so `sysmond` won't read them while they are being written.
