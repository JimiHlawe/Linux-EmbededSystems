#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

int main()
{
    FILE *file;
    char line[256];

    // Create data directory if needed
    if (mkdir("/tmp/sysmon", 0755) != 0 && errno != EEXIST) {
        printf("Cannot create directory\n");
        return 1;
    }

    while (1) {

        printf("\n===== SYSTEM MONITOR =====\n");

        // Memory
        printf("\n--- MEMORY ---\n");

        file = fopen("/tmp/sysmon/mem.dat", "r");

        if (file != NULL) {

            while (fgets(line, sizeof(line), file) != NULL) {
                printf("%s", line);
            }

            fclose(file);
        }
        else {
            printf("Memory data not available\n");
        }

        // CPU
        printf("\n--- CPU ---\n");

        file = fopen("/tmp/sysmon/cpu.dat", "r");

        if (file != NULL) {

            while (fgets(line, sizeof(line), file) != NULL) {
                printf("%s", line);
            }

            fclose(file);
        }
        else {
            printf("CPU data not available\n");
        }

        // Processes
        printf("\n--- PROCESSES ---\n");

        file = fopen("/tmp/sysmon/proc.dat", "r");

        if (file != NULL) {

            while (fgets(line, sizeof(line), file) != NULL) {
                printf("%s", line);
            }

            fclose(file);
        }
        else {
            printf("Process data not available\n");
        }

        printf("\n==========================\n");

        sleep(2);
    }

    return 0;
}
