#!/bin/bash

# Start sensors
./bin/mem_sensor &
mem_pid=$!

./bin/cpu_sensor &
cpu_pid=$!

./bin/proc_sensor &
proc_pid=$!

# Stop sensors when the script exits
stop_all() {
    kill "$mem_pid" "$cpu_pid" "$proc_pid" 2>/dev/null
    wait "$mem_pid" "$cpu_pid" "$proc_pid" 2>/dev/null
}

trap stop_all EXIT

# Run the monitor
./bin/sysmond
