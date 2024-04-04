#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>

#define FILENAME "randomNumbers.txt" // File being written to
#define FOUND_KEYS_FILE "foundKeys.txt"     // File for key info
#define READ_END 0
#define WRITE_END 1

// Function prototypes
void generateRandomNumbers(int L);
void hiddenKeys(int H, int L);
int findMax(int arr[], int start, int end);
float findAvg(int arr[], int start, int end);
int findKey(int arr[], int start, int end, int H);
void createChild(int PN, int nums[], int start, int seg, int fd[][2], bool finalProcess, int H);

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

    int num[L];
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", FILENAME);
        return 1;
    }
    for (int i = 0; i < L; ++i) {
        fscanf(file, "%d", &num[i]);
    }
    fclose(file);

    int fd[PN][2];
    createChild(PN, num, 0, L/PN, fd, true, H);

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
        fprintf(file, "%d\n", num);
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
        position = rand() % L;
        fseek(file, position * sizeof(int), SEEK_SET);
        fprintf(file, "\n%d\n", num);
    }

    fclose(file);
    printf("Secret keys hidden in file\n");
}

int findMax(int arr[], int start, int end) {
   int max = arr[start];  // Initialize max with the first element of the specified part
  
   for (int i = start + 1; i <= end; i++) {
       if (arr[i] > max) {
           max = arr[i];  // Update max if a larger element is found
       }
   }
  
   return max;
}

float findAvg(int arr[], int start, int end) {
    int sum = 0;
    for (int i = start; i < end + 1; i++) {
        sum = arr[i] + sum;
    }
    return (float)sum / (end - start + 1);
}


int findKey(int arr[], int start, int end, int H) {
    int counter = 0;
    for (int i = start; i < end; i++) {
        if ((arr[i] < 0) && (counter < H)) {
            printf("Key at num[%d]\n", i);
            counter++;
        }
    }
}



void createChild(int PN, int nums[], int start, int seg, int fd[][2], bool finalProcess, int H) {
   if (PN <= 0) {
       return;
   }


   pid_t pid;
   int maxChild;
   for(int i=0; i<PN;i++){
       pipe(fd[i]);
   }

   pid = fork();
   if (pid < 0) {
       perror("Fork failed");
       exit(EXIT_FAILURE);
   } else if (pid == 0) {
       // Child process
       createChild(PN - 1, nums, start + seg, seg, fd, false, H);
       if (PN == 1) {
           // Last child, compute local maximum and write to pipe
           int end = start + seg - 1;
           int localMax = findMax(nums, start, end);
           write(fd[0][WRITE_END], &localMax, sizeof(localMax));  // Write max value to the pipe
           printf("Child PID: %d\n", getpid());
           close(fd[0][WRITE_END]);  // Close write end
           exit(0);
       }
   } else {
       // Parent process
       close(fd[PN-1][WRITE_END]);  // Close unused read end
       int end = start + seg - 1;
       int localMax = findMax(nums, start, end);
       float localAvg = findAvg(nums, start, end);
       int keys = findKey(nums, start, end, H);
       
       wait(NULL);  // Wait for child process to finish
       read(fd[PN-1][READ_END], &maxChild, sizeof(maxChild));  // Read max value from the pipe
        if (localMax > maxChild){
           maxChild = localMax;
       }
       printf("Hi I'm process %d with return arg %d and my parent is %d\n", getpid(), PN, getppid());
       if (finalProcess == true) {
        printf("Max: %d\tAverage: %f\n", maxChild, localAvg);
        }
       write(fd[PN][WRITE_END],&maxChild,sizeof(maxChild));
      
       close(fd[PN-1][READ_END]);  // Close read end
       exit(EXIT_SUCCESS);
   }
}