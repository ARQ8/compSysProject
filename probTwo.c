#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

#define NUM_CHILDREN 2

void signalHandler(int sig) {
    if (sig == SIGCONT) {
        printf("Child %d received SIGCONT, extending process life...\n", getpid());
        sleep(100); // Simulate extended task
        exit(0);
    }
}

// Placeholder function to simulate hidden key finding and local maximum calculation
int performTaskAndGetLocalMax() {
    // Simulate task
    sleep(1); // Short sleep to simulate work
    return rand() % 100; // Return a simulated local maximum
}

void createChildProcesses(int depth, int pipefd[]) {
    if (depth == 0) return;

    for (int i = 0; i < NUM_CHILDREN; ++i) {
        if (fork() == 0) { // Child process
            signal(SIGCONT, signalHandler);
            int localMax = performTaskAndGetLocalMax();
            write(pipefd[1], &localMax, sizeof(localMax)); // Send localMax to parent
            pause(); // Wait for potential SIGCONT from parent

            printf("Child process %d completing...\n", getpid());
            exit(0); // Child exits after handling SIGCONT
        }
    }

    // Parent process
    int childMax[NUM_CHILDREN] = {0};
    for (int i = 0; i < NUM_CHILDREN; ++i) {
        read(pipefd[0], &childMax[i], sizeof(childMax[i])); // Read localMax from children
    }

    // Determine if any child's localMax warrants extension
    for (int i = 0; i < NUM_CHILDREN; ++i) {
        if (childMax[i] > 50) { // Arbitrary condition to extend process life
            // Assuming child PIDs are tracked and available
            printf("Parent %d sending SIGCONT to child with localMax: %d\n", getpid(), childMax[i]);
            //kill(childPID[i], SIGCONT); // Send SIGCONT to selected child
        }
    }

    while (wait(NULL) > 0); // Wait for all child processes to exit
}

int main() {
    srand(time(NULL)); // Seed the random number generator
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    createChildProcesses(2, pipefd); // Example with depth = 2

    printf("Main parent process exiting...\n");
    return 0;
}
