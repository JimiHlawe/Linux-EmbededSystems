#include <stdio.h>
#include <unistd.h>
#include <sysmon.h>

void *cpu_sensor_thread(void *arg)
{
    SystemState *state = (SystemState *)arg;

    FILE *file;
    double load1, load5, load15;

    while (running) {

        // Read CPU load
        file = fopen("/proc/loadavg", "r");

        if (file == NULL) {
            printf("Cannot open loadavg\n");
            return NULL;
        }

        if (fscanf(file, "%lf %lf %lf",
                   &load1, &load5, &load15) != 3) {
            fclose(file);
            return NULL;
        }

        fclose(file);

        // Update shared memory
        pthread_mutex_lock(&state->lock);

        state->load1 = load1;
        state->load5 = load5;
        state->load15 = load15;

        // Check CPU load
        if (load1 > 1.50) {
            state->alert_pending = 1;
            pthread_cond_signal(&state->alert_cond);
        }

        pthread_mutex_unlock(&state->lock);

        sleep(2);
    }

    return NULL;
}
