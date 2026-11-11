#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main()
{
    FILE *file;
    FILE *output;
    char line[256];
    char name[50];
    long value;

    while (1) {

        // Open memory information
        file = fopen("/proc/meminfo", "r");

        if (file == NULL) {
            printf("Cannot open meminfo\n");
            return 1;
        }

        // Open output file
        output = fopen("/tmp/sysmon/mem.tmp", "w");

        if (output == NULL) {
            printf("Cannot open mem.dat\n");
            fclose(file);
            return 1;
        }

        // Read memory values
        while (fgets(line, sizeof(line), file) != NULL) {

            if (sscanf(line, "%49s %ld", name, &value) == 2) {

                if (strcmp(name, "MemTotal:") == 0 ||
                    strcmp(name, "MemFree:") == 0 ||
                    strcmp(name, "MemAvailable:") == 0 ||
                    strcmp(name, "SwapTotal:") == 0 ||
                    strcmp(name, "SwapFree:") == 0) {

                    fprintf(output, "%s %ld kB\n", name, value);
                }
            }
        }

        fclose(file);
        fclose(output);

	rename("/tmp/sysmon/mem.tmp", "/tmp/sysmon/mem.dat");

        sleep(2);
    }

    return 0;
}
