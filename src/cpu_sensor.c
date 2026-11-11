#include <stdio.h>
#include <unistd.h>

int main()
{
    FILE *file;
    FILE *output;
    float load1, load5, load15;

    while (1) {

        // Read CPU load
        file = fopen("/proc/loadavg", "r");

        if (file == NULL) {
            printf("Cannot open loadavg\n");
            return 1;
        }

        if (fscanf(file, "%f %f %f", &load1, &load5, &load15) != 3) {
            printf("Cannot read CPU load\n");
            fclose(file);
            return 1;
        }

        fclose(file);

        // Save CPU values
        output = fopen("/tmp/sysmon/cpu.tmp", "w");

        if (output == NULL) {
            printf("Cannot open cpu.dat\n");
            return 1;
        }

        fprintf(output, "Load 1 min: %.2f\n", load1);
        fprintf(output, "Load 5 min: %.2f\n", load5);
        fprintf(output, "Load 15 min: %.2f\n", load15);

        fclose(output);
	rename("/tmp/sysmon/cpu.tmp", "/tmp/sysmon/cpu.dat");
        sleep(2);
    }

    return 0;
}
