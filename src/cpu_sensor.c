
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sysmon_ipc.h>

int main()
{
    FILE *file;
    CpuData cpu_data;
    int fifo_fd;

    // Open FIFO for writing
    fifo_fd = open(CPU_FIFO_PATH, O_WRONLY);

    if (fifo_fd == -1) {
        perror("open FIFO");
        return 1;
    }

    while (1) {

        // Read CPU load
        file = fopen("/proc/loadavg", "r");

        if (file == NULL) {
            perror("loadavg");
            close(fifo_fd);
            return 1;
        }

        if (fscanf(file, "%lf %lf %lf",
                   &cpu_data.load1,
                   &cpu_data.load5,
                   &cpu_data.load15) != 3) {
            printf("Cannot read CPU load\n");
            fclose(file);
            close(fifo_fd);
            return 1;
        }

        fclose(file);

        // Send data to parent
        if (write(fifo_fd, &cpu_data, sizeof(cpu_data)) != sizeof(cpu_data)) {
            printf("Cannot write CPU data\n");
            close(fifo_fd);
            return 1;
        }

        sleep(1);
    }

    return 0;
}
