
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sysmon_ipc.h>

int main()
{
    DIR *dir;
    struct dirent *entry;
    ProcData process_data;
    int fifo_fd;
    int is_pid;
    int i;

    // Open FIFO for writing
    fifo_fd = open(PROC_FIFO_PATH, O_WRONLY);

    if (fifo_fd == -1) {
        perror("open FIFO");
        return 1;
    }

    while (1) {

        process_data.total_procs = 0;

        // Open proc directory
        dir = opendir("/proc");

        if (dir == NULL) {
            perror("opendir");
            close(fifo_fd);
            return 1;
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
                process_data.total_procs++;
            }
        }

        closedir(dir);

        // Send data to parent
        if (write(fifo_fd, &process_data, sizeof(process_data)) != sizeof(process_data)) {
            perror("write");
            close(fifo_fd);
            return 1;
        }

        sleep(1);
    }

    return 0;
}
