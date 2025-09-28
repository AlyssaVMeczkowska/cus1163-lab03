#include "process_manager.h"

int run_basic_demo(void) {
    int pipe_fd[2];
    pid_t producer_pid, consumer_pid;
    int status;

    printf("\nStarting basic producer-consumer demonstration...\n\n");
    printf("Parent process (PID: %d) creating children...\n", getpid());

    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        return -1;
    }

    producer_pid = fork();
    if (producer_pid < 0) {
        perror("fork");
        return -1;
    } else if (producer_pid == 0) {
        close(pipe_fd[0]);
        producer_process(pipe_fd[1], 1);
    } else {
        printf("Created producer child (PID: %d)\n", producer_pid);
    }

    consumer_pid = fork();
    if (consumer_pid < 0) {
        perror("fork");
        return -1;
    } else if (consumer_pid == 0) {
        close(pipe_fd[1]);
        consumer_process(pipe_fd[0], 0);
    } else {
        printf("Created consumer child (PID: %d)\n", consumer_pid);
    }

    close(pipe_fd[0]);
    close(pipe_fd[1]);

    pid_t child;
    child = waitpid(producer_pid, &status, 0);
    if (child > 0) {
        printf("\nProducer child (PID: %d) exited with status %d\n", child, WEXITSTATUS(status));
    }

    child = waitpid(consumer_pid, &status, 0);
    if (child > 0) {
        printf("Consumer child (PID: %d) exited with status %d\n", child, WEXITSTATUS(status));
    }

    printf("\nSUCCESS: Basic producer-consumer completed!\n");
    return 0;
}

int run_multiple_pairs(int num_pairs) {
    pid_t pids[20];
    int pid_count = 0;

    printf("\nRunning multiple producer-consumer pairs...\n\n");
    printf("Parent creating %d producer-consumer pairs...\n", num_pairs);

    for (int i = 0; i < num_pairs; i++) {
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("pipe");
            return -1;
        }

        printf("\n=== Pair %d ===\n", i + 1);

        pid_t producer_pid = fork();
        if (producer_pid < 0) {
            perror("fork");
            return -1;
        } else if (producer_pid == 0) {
            close(pipe_fd[0]);
            producer_process(pipe_fd[1], i * NUM_VALUES + 1);
        } else {
            pids[pid_count++] = producer_pid;
        }

        pid_t consumer_pid = fork();
        if (consumer_pid < 0) {
            perror("fork");
            return -1;
        } else if (consumer_pid == 0) {
            close(pipe_fd[1]);
            consumer_process(pipe_fd[0], i + 1);
        } else {
            pids[pid_count++] = consumer_pid;
        }

        close(pipe_fd[0]);
        close(pipe_fd[1]);
    }

    int status;
    for (int i = 0; i < pid_count; i++) {
        pid_t child = waitpid(pids[i], &status, 0);
        if (child > 0) {
            printf("Child (PID: %d) exited with status %d\n", child, WEXITSTATUS(status));
        }
    }

    printf("\nAll pairs completed successfully!\n");
    printf("\nSUCCESS: Multiple pairs completed!\n");

    return 0;
}

void producer_process(int write_fd, int start_num) {
    printf("Producer (PID: %d) starting...\n", getpid());

    for (int i = 0; i < NUM_VALUES; i++) {
        int number = start_num + i;

        if (write(write_fd, &number, sizeof(number)) != sizeof(number)) {
            perror("write");
            exit(1);
        }

        printf("Producer: Sent number %d\n", number);
        usleep(100000);
    }

    printf("Producer: Finished sending %d numbers\n", NUM_VALUES);
    close(write_fd);
    exit(0);
}

void consumer_process(int read_fd, int pair_id) {
    int number;
    int sum = 0;

    printf("Consumer (PID: %d) starting...\n", getpid());

    while (read(read_fd, &number, sizeof(number)) > 0) {
        sum += number;
        printf("Consumer: Received %d, running sum: %d\n", number, sum);
    }

    printf("Consumer: Final sum: %d\n", sum);
    close(read_fd);
    exit(0);
}

