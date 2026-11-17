#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sysmon.h>

volatile sig_atomic_t running = 1;

// Stop the program
void stop_program(int sig)
{
    running = 0;
}

// Alert thread
void *alert_thread(void *arg)
{
    SystemState *state = (SystemState *)arg;

    while (running) {

        pthread_mutex_lock(&state->lock);

        while (state->alert_pending == 0 && running) {
            pthread_cond_wait(&state->alert_cond, &state->lock);
        }

        if (running && state->alert_pending == 1) {
            fprintf(stderr, "WARNING: High CPU load!\n");
            state->alert_pending = 0;
        }

        pthread_mutex_unlock(&state->lock);
    }

    return NULL;
}

int main()
{
    SystemState state = {0};

    pthread_t mem, cpu, proc, alert;

    // Initialize mutex and condition
    pthread_mutex_init(&state.lock, NULL);
    pthread_cond_init(&state.alert_cond, NULL);

    // Handle Ctrl+C
    signal(SIGINT, stop_program);

    // Create threads
    pthread_create(&mem, NULL, mem_sensor_thread, &state);
    pthread_create(&cpu, NULL, cpu_sensor_thread, &state);
    pthread_create(&proc, NULL, proc_sensor_thread, &state);
    pthread_create(&alert, NULL, alert_thread, &state);

    // Print system information
    while (running) {

        pthread_mutex_lock(&state.lock);

        printf("\n===== SYSTEM MONITOR =====\n");

        printf("\n--- MEMORY ---\n");
        printf("MemTotal: %ld kB\n", state.mem_total);
        printf("MemFree: %ld kB\n", state.mem_free);
        printf("MemAvailable: %ld kB\n", state.mem_avail);
        printf("SwapTotal: %ld kB\n", state.swap_total);
        printf("SwapFree: %ld kB\n", state.swap_free);

        printf("\n--- CPU ---\n");
        printf("Load 1 min: %.2f\n", state.load1);
        printf("Load 5 min: %.2f\n", state.load5);
        printf("Load 15 min: %.2f\n", state.load15);

        printf("\n--- PROCESSES ---\n");
        printf("Total processes: %d\n", state.total_procs);

        printf("\n==========================\n");

        pthread_mutex_unlock(&state.lock);

        sleep(2);
    }

    // Wake up alert thread before exit
    pthread_mutex_lock(&state.lock);
    pthread_cond_signal(&state.alert_cond);
    pthread_mutex_unlock(&state.lock);

    // Wait for threads to finish
    pthread_join(mem, NULL);
    pthread_join(cpu, NULL);
    pthread_join(proc, NULL);
    pthread_join(alert, NULL);

    pthread_mutex_destroy(&state.lock);
    pthread_cond_destroy(&state.alert_cond);

    return 0;
}
