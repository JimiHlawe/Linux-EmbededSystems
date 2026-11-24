
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sysmon_ipc.h>

extern char **environ;

volatile sig_atomic_t running = 1;

// Process IDs
pid_t memory_pid = -1;
pid_t cpu_pid = -1;
pid_t process_pid = -1;

// Pipe descriptors
int memory_pipe[2] = {-1, -1};
int cpu_fd = -1;
int process_fd = -1;

// Stop on Ctrl+C
void stop_program(int sig)
{
    running = 0;
}

// Stop processes and close pipes
void cleanup()
{
    if (memory_pid > 0)
        kill(memory_pid, SIGTERM);

    if (cpu_pid > 0)
        kill(cpu_pid, SIGTERM);

    if (process_pid > 0)
        kill(process_pid, SIGTERM);

    if (memory_pid > 0)
        waitpid(memory_pid, NULL, 0);

    if (cpu_pid > 0)
        waitpid(cpu_pid, NULL, 0);

    if (process_pid > 0)
        waitpid(process_pid, NULL, 0);

    if (memory_pipe[0] != -1)
        close(memory_pipe[0]);

    if (memory_pipe[1] != -1)
        close(memory_pipe[1]);

    if (cpu_fd != -1)
        close(cpu_fd);

    if (process_fd != -1)
        close(process_fd);

    unlink(CPU_FIFO_PATH);
    unlink(PROC_FIFO_PATH);
}

int main()
{
    char pipe_number[20];
    struct pollfd pipes[3];

    MemData memory_data;
    CpuData cpu_data;
    ProcData process_data;

    int result = 0;

    signal(SIGINT, stop_program);
    signal(SIGTERM, stop_program);

    // Create pipes
    unlink(CPU_FIFO_PATH);
    unlink(PROC_FIFO_PATH);

    if (pipe(memory_pipe) == -1) {
        perror("pipe");
        cleanup();
        return 1;
    }

    if (mkfifo(CPU_FIFO_PATH, 0600) == -1 ||
        mkfifo(PROC_FIFO_PATH, 0600) == -1) {
        perror("mkfifo");
        cleanup();
        return 1;
    }

    // Start memory sensor
    memory_pid = fork();

    if (memory_pid == -1) {
        perror("fork memory");
        cleanup();
        return 1;
    }

    if (memory_pid == 0) {
        close(memory_pipe[0]);

        snprintf(pipe_number, sizeof(pipe_number),
                 "%d", memory_pipe[1]);

        char *args[] = {"./bin/mem_sensor", pipe_number, NULL};

        execve(args[0], args, environ);
        perror("execve memory");
        _exit(1);
    }

    close(memory_pipe[1]);
    memory_pipe[1] = -1;

    // Start CPU sensor
    cpu_pid = fork();

    if (cpu_pid == -1) {
        perror("fork CPU");
        cleanup();
        return 1;
    }

    if (cpu_pid == 0) {
        close(memory_pipe[0]);

        char *args[] = {"./bin/cpu_sensor", NULL};

        execve(args[0], args, environ);
        perror("execve CPU");
        _exit(1);
    }

    // Start process sensor
    process_pid = fork();

    if (process_pid == -1) {
        perror("fork process");
        cleanup();
        return 1;
    }

    if (process_pid == 0) {
        close(memory_pipe[0]);

        char *args[] = {"./bin/proc_sensor", NULL};

        execve(args[0], args, environ);
        perror("execve process");
        _exit(1);
    }

    // Open FIFOs
    cpu_fd = open(CPU_FIFO_PATH, O_RDONLY);

    if (cpu_fd == -1) {
        perror("open CPU FIFO");
        cleanup();
        return 1;
    }

    process_fd = open(PROC_FIFO_PATH, O_RDONLY);

    if (process_fd == -1) {
        perror("open process FIFO");
        cleanup();
        return 1;
    }

    // Set up poll
    pipes[0].fd = memory_pipe[0];
    pipes[1].fd = cpu_fd;
    pipes[2].fd = process_fd;

    pipes[0].events = POLLIN;
    pipes[1].events = POLLIN;
    pipes[2].events = POLLIN;

    // Read data when available
    while (running) {

        if (poll(pipes, 3, -1) == -1) {
            if (running) {
                perror("poll");
                result = 1;
            }
            break;
        }

        if (!running)
            break;

        // Memory
        if (pipes[0].revents & POLLIN) {

            if (read(pipes[0].fd, &memory_data,
                     sizeof(memory_data)) != sizeof(memory_data)) {
                result = 1;
                break;
            }

            printf("\n--- MEMORY ---\n");
            printf("MemTotal: %ld kB\n", memory_data.mem_total);
            printf("MemFree: %ld kB\n", memory_data.mem_free);
            printf("MemAvailable: %ld kB\n", memory_data.mem_avail);
            printf("SwapTotal: %ld kB\n", memory_data.swap_total);
            printf("SwapFree: %ld kB\n", memory_data.swap_free);
        }

        // CPU
        if (pipes[1].revents & POLLIN) {

            if (read(pipes[1].fd, &cpu_data,
                     sizeof(cpu_data)) != sizeof(cpu_data)) {
                result = 1;
                break;
            }

            printf("\n--- CPU ---\n");
            printf("Load 1 min: %.2f\n", cpu_data.load1);
            printf("Load 5 min: %.2f\n", cpu_data.load5);
            printf("Load 15 min: %.2f\n", cpu_data.load15);
        }

        // Processes
        if (pipes[2].revents & POLLIN) {

            if (read(pipes[2].fd, &process_data,
                     sizeof(process_data)) != sizeof(process_data)) {
                result = 1;
                break;
            }

            printf("\n--- PROCESSES ---\n");
            printf("Total processes: %d\n", process_data.total_procs);
        }

        fflush(stdout);

        // Check closed pipes
        if (pipes[0].revents & (POLLHUP | POLLERR | POLLNVAL))
            break;

        if (pipes[1].revents & (POLLHUP | POLLERR | POLLNVAL))
            break;

        if (pipes[2].revents & (POLLHUP | POLLERR | POLLNVAL))
            break;
    }

    // Clean up before exit
    cleanup();

    return result;
}
