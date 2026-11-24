
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sysmon_ipc.h>

extern char **environ;

volatile sig_atomic_t running = 1;
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

// Stop on Ctrl+C
void stop_program(int sig)
{
    running = 0;
}

// Read memory data
void *read_memory(void *arg)
{
    int fd = *(int *)arg;
    MemData data;

    while (running) {
        if (read(fd, &data, sizeof(data)) != sizeof(data))
            break;

        pthread_mutex_lock(&print_lock);

        printf("\n--- MEMORY ---\n");
        printf("MemTotal: %ld kB\n", data.mem_total);
        printf("MemFree: %ld kB\n", data.mem_free);
        printf("MemAvailable: %ld kB\n", data.mem_avail);
        printf("SwapTotal: %ld kB\n", data.swap_total);
        printf("SwapFree: %ld kB\n", data.swap_free);

        fflush(stdout);
        pthread_mutex_unlock(&print_lock);
    }

    return NULL;
}

// Read CPU data
void *read_cpu(void *arg)
{
    int fd = *(int *)arg;
    CpuData data;

    while (running) {
        if (read(fd, &data, sizeof(data)) != sizeof(data))
            break;

        pthread_mutex_lock(&print_lock);

        printf("\n--- CPU ---\n");
        printf("Load 1 min: %.2f\n", data.load1);
        printf("Load 5 min: %.2f\n", data.load5);
        printf("Load 15 min: %.2f\n", data.load15);

        fflush(stdout);
        pthread_mutex_unlock(&print_lock);
    }

    return NULL;
}

// Read process count
void *read_processes(void *arg)
{
    int fd = *(int *)arg;
    ProcData data;

    while (running) {
        if (read(fd, &data, sizeof(data)) != sizeof(data))
            break;

        pthread_mutex_lock(&print_lock);

        printf("\n--- PROCESSES ---\n");
        printf("Total processes: %d\n", data.total_procs);

        fflush(stdout);
        pthread_mutex_unlock(&print_lock);
    }

    return NULL;
}

int main()
{
    int memory_pipe[2] = {-1, -1};
    int cpu_fd = -1;
    int process_fd = -1;

    pid_t memory_pid = -1;
    pid_t cpu_pid = -1;
    pid_t process_pid = -1;

    pthread_t memory_thread, cpu_thread, process_thread;
    int memory_started = 0, cpu_started = 0, process_started = 0;

    char pipe_number[20];
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

        snprintf(pipe_number, sizeof(pipe_number), "%d", memory_pipe[1]);
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
        perror("fork process");
        goto cleanup;
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
        goto cleanup;
    }

    process_fd = open(PROC_FIFO_PATH, O_RDONLY);

    if (process_fd == -1) {
        perror("open process FIFO");
        goto cleanup;
    }

    // Start reader threads
    if (pthread_create(&memory_thread, NULL, read_memory, &memory_pipe[0]) != 0) {
        printf("Cannot start memory thread\n");
        goto cleanup;
    }
    memory_started = 1;

    if (pthread_create(&cpu_thread, NULL, read_cpu, &cpu_fd) != 0) {
        printf("Cannot start CPU thread\n");
        goto cleanup;
    }
    cpu_started = 1;

    if (pthread_create(&process_thread, NULL, read_processes, &process_fd) != 0) {
        printf("Cannot start process thread\n");
        goto cleanup;
    }
    process_started = 1;

    // Wait until Ctrl+C
    while (running)
        sleep(1);

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

    // Wait for reader threads
    if (memory_started)
        pthread_join(memory_thread, NULL);

    if (cpu_started)
        pthread_join(cpu_thread, NULL);

    if (process_started)
        pthread_join(process_thread, NULL);

    // Close pipes
    if (memory_pipe[0] != -1)
        close(memory_pipe[0]);

    if (memory_pipe[1] != -1)
        close(memory_pipe[1]);

    if (cpu_fd != -1)
        close(cpu_fd);

    if (process_fd != -1)
        close(process_fd);

    // Remove FIFOs
    unlink(CPU_FIFO_PATH);
    unlink(PROC_FIFO_PATH);

    pthread_mutex_destroy(&print_lock);

    return exit_status;
}
