#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>

#define FILENAME "randomNumbers.txt" // File being written to
#define FOUND_KEYS_FILE "output.txt"     // File for key info
#define READ_END 0
#define WRITE_END 1

// Function prototypes
void generateRandomNumbers(int L);
//void hiddenKeys(int H, int L);
int findMax(int arr[], int start, int end);
float findAvg(int arr[], int start, int end);
int findKey(int arr[], int start, int end, int H);
void createChild(int PN, int nums[], int start, int seg, int fd[][2], bool finalProcess, int H, int size);

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
//    hiddenKeys(H, L);           // Call function to generate hidden keys

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
    createChild(PN, num, 0, (L)/PN, fd, true, H, L);

    return 0;
}

void generateRandomNumbers(int L) {
    FILE *file;
    int i, num, sum = 0, max = 0;
    int negativeCount = 0;
    float average;

    file = fopen(FILENAME, "w");    // Open file for writing

    // Check if file opened
    if (file == NULL) {
        printf("Error opening file.\n");
        exit(EXIT_FAILURE);
    }

    // Random number seeding
    srand(time(0));

    L = L + 59;
    // Generate random positive or negative integers and write them to file
    for (i = 0; i < L; i++) {
        // Calculate the probability for selecting a negative number
        double negativeProbability = (double)(60 - negativeCount) / (L - i);

        // Randomly select if the number will be negative based on probability
        if (rand() / (RAND_MAX + 1.0) < negativeProbability) {
            // Select a negative number
            num = -(rand() % 60 + 1); // Random negative number between -1 and -60
            negativeCount++;
        } else {
            // Select a positive number
            num = rand() % 10000 + 1; // Random positive number between 1 and 10000
        }

        sum += num;
        if (num > max) {
            max = num;
        }
        fprintf(file, "%d\n", num);

        // If 60 negative numbers are generated, ensure all remaining numbers are positive
        if (negativeCount == 60) {
            break;
        }
    }

    // If less than 60 negative numbers are generated, fill the remaining numbers with positive numbers
    while (i < L) {
        num = rand() % 10000 + 1; // Random positive number between 1 and 10000
        sum += num;
        if (num > max) {
            max = num;
        }
        fprintf(file, "%d\n", num);
        i++;
    }

    average = (double)sum / L;

    fclose(file);
    printf("Random numbers generated and written to file\n");
    printf("Self Calculated Sum of all numbers: %d\n", sum);
    printf("Self Calculated Max=%d, Avg=%.2f\n", max, average);
}


/*void hiddenKeys(int H, int L) {
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
}*/

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
    FILE *file = fopen(FOUND_KEYS_FILE, "a");
    int counter = 0;
    if (file == NULL) {
        printf("Error opening file.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = start; i <= end; i++) {
        if ((arr[i] < 0) && (counter < H)) {
            fprintf(file, "Key at num[%d]\n", i);
            counter++;
        }
    }

    fclose(file);
}



void createChild(int PN, int nums[], int start, int seg, int fd[][2], bool finalProcess, int H, int size) {
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
       createChild(PN - 1, nums, start + seg, seg, fd, false, H, size);
       if (PN == 1) {
           // Last child sends 0 values back so second to last isn't waiting at read
           int end = size-1;
           int localMax = 0;
           float localAvg = 0;
           write(fd[0][WRITE_END], &localAvg, sizeof(localAvg));  // Write local avg to the pipe
           write(fd[0][WRITE_END], &localMax, sizeof(localMax));  // Write max value to the pipe
           printf("Child PID: %d\n", getpid());
           close(fd[0][WRITE_END]);  // Close write end
           exit(0);
       }
   } else {
       // Parent process
       close(fd[PN-1][WRITE_END]);  // Close unused read end
       int end = start + seg - 1;
       if(PN==1){
        end = size-1;
       }
       int localMax = findMax(nums, start, end);
       float localAvg = findAvg(nums, start, end);
       int keys = findKey(nums, start, end, H);

       float totalAvgSum;
       float finalAvg;

       wait(NULL);  // Wait for child process to finish
       read(fd[PN-1][READ_END], &totalAvgSum, sizeof(totalAvgSum));  // Read local avg from the pipe
        totalAvgSum = totalAvgSum + localAvg;
       read(fd[PN-1][READ_END], &maxChild, sizeof(maxChild));  // Read max value from the pipe
        if (localMax > maxChild){
           maxChild = localMax;
       }
       FILE *file = fopen(FOUND_KEYS_FILE, "a");

       if (file == NULL) {
        printf("Error opening file\n");
        exit(EXIT_FAILURE);
       }
       
       fprintf(file, "Hi I'm process %d with return arg %d and my parent is %d\n", getpid(), PN, getppid());
       if (finalProcess == true) {
        finalAvg = totalAvgSum / PN;
        fprintf(file, "Max: %d\tAverage: %f\n", maxChild, finalAvg);
        }

        fclose(file);

       write(fd[PN][WRITE_END], &totalAvgSum, sizeof(totalAvgSum));
       write(fd[PN][WRITE_END], &maxChild, sizeof(maxChild));
       
      
       close(fd[PN-1][READ_END]);  // Close read end
       close(fd[PN][WRITE_END]);
       exit(EXIT_SUCCESS);
   }
}