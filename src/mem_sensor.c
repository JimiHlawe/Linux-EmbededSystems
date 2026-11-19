
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sysmon_ipc.h>

int main(int argc, char *argv[])
{
    FILE *file;
    MemData memory;
    char line[256];
    char name[50];
    long value;
    int pipe_fd = STDOUT_FILENO;

    // Get pipe descriptor from parent
    if (argc > 1) {
        pipe_fd = atoi(argv[1]);
    }

    while (1) {

        memory = (MemData){0};

        // Open memory information
        file = fopen("/proc/meminfo", "r");

        if (file == NULL) {
            perror("meminfo");
            return 1;
        }

        // Read memory values
        while (fgets(line, sizeof(line), file) != NULL) {

            if (sscanf(line, "%49s %ld", name, &value) == 2) {

                if (strcmp(name, "MemTotal:") == 0)
                    memory.mem_total = value;

                if (strcmp(name, "MemFree:") == 0)
                    memory.mem_free = value;

                if (strcmp(name, "MemAvailable:") == 0)
                    memory.mem_avail = value;

                if (strcmp(name, "SwapTotal:") == 0)
                    memory.swap_total = value;

                if (strcmp(name, "SwapFree:") == 0)
                    memory.swap_free = value;
            }
        }

        fclose(file);

        // Send data to parent
        if (write(pipe_fd, &memory, sizeof(memory)) != sizeof(memory)) {
            perror("write");
            return 1;
        }

        sleep(1);
    }

    return 0;
}
