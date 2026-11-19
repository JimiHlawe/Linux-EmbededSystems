
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sysmon_ipc.h>

extern char **environ;

volatile sig_atomic_t running = 1;

// Stop on Ctrl+C
void stop_program(int sig)
{
    running = 0;
}

int main()
{
    int memory_pipe[2] = {-1, -1};
    int cpu_fd = -1;
    int process_fd = -1;

    pid_t memory_pid = -1;
    pid_t cpu_pid = -1;
    pid_t process_pid = -1;

    char pipe_number[20];

    MemData memory_data;
    CpuData cpu_data;
    ProcData process_data;

    int exit_status = 1;

    signal(SIGINT, stop_program);
    signal(SIGTERM, stop_program);

    // Create pipes
    unlink(CPU_FIFO_PATH);
    unlink(PROC_FIFO_PATH);

    if (pipe(memory_pipe) == -1) {
        perror("pipe");
        goto cleanup;
    }

    if (mkfifo(CPU_FIFO_PATH, 0600) == -1) {
        perror("CPU FIFO");
        goto cleanup;
    }

    if (mkfifo(PROC_FIFO_PATH, 0600) == -1) {
        perror("Process FIFO");
        goto cleanup;
    }

    // Start memory sensor
    memory_pid = fork();

    if (memory_pid == -1) {
        perror("fork memory");
        goto cleanup;
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
        goto cleanup;
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
        perror("fork processes");
        goto cleanup;
    }

    if (process_pid == 0) {
        close(memory_pipe[0]);

        char *args[] = {"./bin/proc_sensor", NULL};

        execve(args[0], args, environ);
        perror("execve processes");
        _exit(1);
    }

    // Open FIFOs for reading
    cpu_fd = open(CPU_FIFO_PATH, O_RDONLY);

    if (cpu_fd == -1) {
        perror("open CPU FIFO");
        goto cleanup;
    }

    process_fd = open(PROC_FIFO_PATH, O_RDONLY);

    if (process_fd == -1) {
        perror("open process FIFO");
        goto cleanup;
    }

    // Read and print data
    while (running) {

        if (read(memory_pipe[0], &memory_data,
                 sizeof(memory_data)) != sizeof(memory_data))
            break;

        if (read(cpu_fd, &cpu_data,
                 sizeof(cpu_data)) != sizeof(cpu_data))
            break;

        if (read(process_fd, &process_data,
                 sizeof(process_data)) != sizeof(process_data))
            break;

        if (!running)
            break;

        printf("\n===== SYSTEM MONITOR =====\n");

        printf("\n--- MEMORY ---\n");
        printf("MemTotal: %ld kB\n", memory_data.mem_total);
        printf("MemFree: %ld kB\n", memory_data.mem_free);
        printf("MemAvailable: %ld kB\n", memory_data.mem_avail);
        printf("SwapTotal: %ld kB\n", memory_data.swap_total);
        printf("SwapFree: %ld kB\n", memory_data.swap_free);

        printf("\n--- CPU ---\n");
        printf("Load 1 min: %.2f\n", cpu_data.load1);
        printf("Load 5 min: %.2f\n", cpu_data.load5);
        printf("Load 15 min: %.2f\n", cpu_data.load15);

        printf("\n--- PROCESSES ---\n");
        printf("Total processes: %d\n", process_data.total_procs);

        printf("\n==========================\n");
        fflush(stdout);
    }

    if (!running)
        exit_status = 0;

cleanup:
    // Stop children
    if (memory_pid > 0)
        kill(memory_pid, SIGTERM);

    if (cpu_pid > 0)
        kill(cpu_pid, SIGTERM);

    if (process_pid > 0)
        kill(process_pid, SIGTERM);

    // Wait for children
    if (memory_pid > 0)
        waitpid(memory_pid, NULL, 0);

    if (cpu_pid > 0)
        waitpid(cpu_pid, NULL, 0);

    if (process_pid > 0)
        waitpid(process_pid, NULL, 0);

    // Close pipes
    if (memory_pipe[0] != -1)
        close(memory_pipe[0]);

    if (memory_pipe[1] != -1)
        close(memory_pipe[1]);

    if (cpu_fd != -1)
        close(cpu_fd);

    if (process_fd != -1)
        close(process_fd);

    // Remove FIFO files
    unlink(CPU_FIFO_PATH);
    unlink(PROC_FIFO_PATH);

    return exit_status;
}
