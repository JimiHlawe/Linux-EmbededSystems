#include <stdio.h>
#include <dirent.h>
#include <unistd.h>

int main()
{
    DIR *dir;
    struct dirent *entry;
    FILE *output;

    int count;
    int i;
    int is_pid;

    while (1) {

        count = 0;

        // Open proc directory
        dir = opendir("/proc");

        if (dir == NULL) {
            printf("Cannot open /proc\n");
            return 1;
        }

        // Check directory names
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

        // Save process count
        output = fopen("/tmp/sysmon/proc.tmp", "w");

        if (output == NULL) {
            printf("Cannot open proc.dat\n");
            return 1;
        }

        fprintf(output, "Total processes: %d\n", count);

        fclose(output);
	rename("/tmp/sysmon/proc.tmp", "/tmp/sysmon/proc.dat");
        sleep(2);
    }

    return 0;
}

