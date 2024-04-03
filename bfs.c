#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>

#define FILENAME "randomNumbers.txt" // File being written to
#define FOUND_KEYS_FILE "foundKeys.txt"     // File for key info

// Function prototypes
void generateRandomNumbers(int L);
void hiddenKeys(int H, int L);
void processSegment(FILE *file, int start, int end, int pipe_write);
void receiveMetrics(int pipe_read, int *max, float *average);
void sendMetrics(int pipe_write, int max, float average);

int main(int argc, char* argv[]) {
    // Check if the correct number of arguments is provided
    if (argc != 4) {
        printf("Usage: %s <L> <H> <PN>\n", argv[0]);
        printf("L: Number of integers to generate (>= 10,000)\n");
        printf("H: Number of hidden keys (30 <= H <= 60)\n");
        printf("PN: Number of processes to use for parallel processing (1 - 10)\n");
        return 1;
    }

    int L = atoi(argv[1]);
    int H = atoi(argv[2]);
    int PN = atoi(argv[3]);

    // Validate input arguments
    if (L < 10000 || H < 30 || H > 60 || PN < 1) {
        printf("Invalid input arguments.\n");
        return 1;
    }

    // Generate random numbers
    generateRandomNumbers(L);   // Call function to generate L random numbers
    hiddenKeys(H, L);           // Call function to generate hidden keys

    // Open the file
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    int pipes[PN][2];
    for (int i = 0; i < PN; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Pipe creation failed");
            exit(EXIT_FAILURE);
        }
    }

    pid_t pid;
    int segment_size = L / PN;
    for (int i = 0; i < PN; i++) {
        pid = fork();

        if (pid == -1) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        
        } else if (pid == 0) {  // Child process
            // Do something
            close(pipes[i][0]);     // Close read end of pipe
            int start = i * segment_size;
            int end = (i == PN - 1) ? L : start + segment_size;
            processSegment(file, start, end, pipes[i][1]);
            exit(0);
        
        } else {                // Parent process
            // Do something
            close(pipes[i][1]);     // Close write end of pipe
        }
    }

    int total_max = INT_MIN;
    int sum = 0;
    for (int i = 0; i < PN; i++) {
        int child_max;
        float child_avg;
        receiveMetrics(pipes[i][0], &child_max, &child_avg);
        if (child_max > total_max) {
            total_max = child_max;
        }
        sum += child_avg;
    }

    float total_avg = (float) sum / PN;

    printf("Pipe max: %d\n", total_max);
    printf("Pipe average: %f\n", total_avg);


    return 0;
}

void generateRandomNumbers(int L) {
    FILE *file;
    int i, num, sum, max;
    float average;

    file = fopen(FILENAME, "w");    // Open file for writing

    // Check if file opened
    if (file == NULL) {
        printf("Error opening file.\n");
        exit(EXIT_FAILURE);
    }

    // Random number seeding
    srand(time(0));

    // Generate random positive integers and write them to file
    for (i = 0; i < L; i++) {
        num = rand() % 10000 + 1;
        sum = sum + num;
        if (num > max) {
            max = num;
        }
        fprintf(file, "\n%d\n", num);
    }

    average = (double)sum/L;

    fclose(file);
    printf("Random numbers generated and written to file\n");
    printf("Self Calculated Sum of all numbers: %d\n", sum);
    printf("Self Calcualted Max=%d, Avg=%.2f\n", max, average);

}


void hiddenKeys(int H, int L) {
    FILE *file;
    int i, num, position;
    const char EOL = '\n';

    file = fopen(FILENAME, "r+");   // Open file for reading and writing

    // Check if file opened
    if (file == NULL) {
        printf("Error opening file.\n");
        exit(EXIT_FAILURE);
    }

    // Random number seeding
    srand(time(0));

    // Add hidden keys to file
    for (i = 0; i < 60; i++) {
        num = -(rand() % 60 + 1);
        position = rand() % (L + 1);
        
        fseek(file, position*sizeof(int), SEEK_SET);
        fprintf(file, "%d", num);
    }

    fclose(file);
    printf("Secret keys hidden in file\n");
}



void processSegment(FILE *file, int start, int end, int pipe_write) {
    // Process segment of the file
    int max = INT_MIN;
    float sum = 0;
    int count = 0;
    fseek(file, 0, SEEK_SET);

    for (int i = 0; i < start; i++) {
        char buffer[20];
        fgets(buffer, sizeof(buffer), file); // Consume lines before start
    }

    for (int i = start; i < end; i++) {
        int num;
        fscanf(file, "%d", &num);
        if (num < 0) continue; // Ignore negative keys
        sum += num;
        count++;
        if (num > max) {
            max = num;
        }
    }

    float avg = (count == 0) ? 0 : sum / count;
    sendMetrics(pipe_write, max, avg);
}


void receiveMetrics(int pipe_read, int *max, float *average) {
    read(pipe_read, max, sizeof(int));
    read(pipe_read, average, sizeof(float));
}


void sendMetrics(int pipe_write, int max, float average) {
    write(pipe_write, &max, sizeof(int));
    write(pipe_write, &average, sizeof(float));
}