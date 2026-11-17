#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sysmon.h>

void *proc_sensor_thread(void *arg)
{
    SystemState *state = (SystemState *)arg;

    DIR *dir;
    struct dirent *entry;
    int count;
    int i;
    int is_pid;

    while (running) {

        count = 0;

        // Open proc directory
        dir = opendir("/proc");

        if (dir == NULL) {
            printf("Cannot open /proc\n");
            return NULL;
        }

        // Count processes
        while ((entry = readdir(dir)) != NULL) {

            is_pid = 1;

            for (i = 0; entry->d_name[i] != '\0'; i++) {

                if (entry->d_name[i] < '0' ||
                    entry->d_name[i] > '9') {

                    is_pid = 0;
                    break;
                }
            }

            if (is_pid == 1) {
                count++;
            }
        }

        closedir(dir);

        // Update shared memory
        pthread_mutex_lock(&state->lock);

        state->total_procs = count;

        pthread_mutex_unlock(&state->lock);

        sleep(2);
    }

    return NULL;
}
