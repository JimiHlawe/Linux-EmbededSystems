
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
    int fifo_fd, i, is_pid;

    // Open FIFO for writing
    fifo_fd = open(PROC_FIFO_PATH, O_WRONLY);

    if (fifo_fd == -1) {
        perror("open process FIFO");
        return 1;
    }

    while (1) {

        process_data.total_procs = 0;
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
                if (entry->d_name[i] < '0' || entry->d_name[i] > '9') {
                    is_pid = 0;
                    break;
                }
            }

            if (is_pid == 1)
                process_data.total_procs++;
        }

        closedir(dir);

        // Send data to parent
        if (write(fifo_fd, &process_data, sizeof(process_data)) != sizeof(process_data)) {
            perror("write process");
            close(fifo_fd);
            return 1;
        }

        sleep(5);
    }

    return 0;
}
