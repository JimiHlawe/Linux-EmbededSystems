#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sysmon.h>

void *mem_sensor_thread(void *arg)
{
    SystemState *state = (SystemState *)arg;

    FILE *file;
    char line[256];
    char name[50];
    long value;

    while (running) {

        long total = 0, free_mem = 0, available = 0;
        long swap_total = 0, swap_free = 0;

        // Open memory information
        file = fopen("/proc/meminfo", "r");

        if (file == NULL) {
            printf("Cannot open meminfo\n");
            return NULL;
        }

        // Read memory values
        while (fgets(line, sizeof(line), file) != NULL) {

            if (sscanf(line, "%49s %ld", name, &value) == 2) {

                if (strcmp(name, "MemTotal:") == 0)
                    total = value;

                if (strcmp(name, "MemFree:") == 0)
                    free_mem = value;

                if (strcmp(name, "MemAvailable:") == 0)
                    available = value;

                if (strcmp(name, "SwapTotal:") == 0)
                    swap_total = value;

                if (strcmp(name, "SwapFree:") == 0)
                    swap_free = value;
            }
        }

        fclose(file);

        // Update shared memory
        pthread_mutex_lock(&state->lock);

        state->mem_total = total;
        state->mem_free = free_mem;
        state->mem_avail = available;
        state->swap_total = swap_total;
        state->swap_free = swap_free;

        pthread_mutex_unlock(&state->lock);

        sleep(2);
    }

    return NULL;
}
