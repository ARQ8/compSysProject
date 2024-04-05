#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>
#include <time.h>

#define FILENAME "randomNumbers.txt"

int findMax(int arr[], int start, int end);
float findAverage(int arr[], int start, int end);
void create_processes(int n, int start, int seg, int num[]);
int calculate_depth(int n);
void generateRandomNumbers(int L);


int main() {
    int start = 0;
    int L = 10000;
    int PN = 511;
    int seg = L / PN;

    generateRandomNumbers(L);
    
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

    create_processes(calculate_depth(PN), start, seg, num);

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


int findMax(int arr[], int start, int end) {
   int max = arr[start];  // Initialize max with the first element of the specified part
  
   for (int i = start + 1; i <= end; i++) {
       if (arr[i] > max) {
           max = arr[i];  // Update max if a larger element is found
       }
   }
  
   return max;
}


// Function to calculate average value in a subarray
float findAverage(int arr[], int start, int end) {
    float sum = 0;
    for (int i = start; i <= end; i++) {
        sum += arr[i];
    }
    return sum / (end - start + 1);
}


void create_processes(int n, int start, int seg, int num[]) {
    if (n == 0)
        return;

    pid_t child1, child2;
    child1 = fork();
    int max;
    if (child1 == 0) { // Child process
        printf("Child %d created with parent %d\n", getpid(), getppid());
        max = findMax(num, (start + seg), (start + seg + seg - 1));
        //printf("Max: %d\n",max);
        start = start + (3 * seg);
        create_processes(n - 1, start, seg, num); // Recursive call for the first child
        exit(0);
    } else if (child1 > 0) { // Parent process
        child2 = fork();

        if (child2 == 0) { // Second child
            printf("Child %d created with parent %d\n", getpid(), getppid());
            max = findMax(num, start+(2*seg), start+(2*seg)+seg-1);
            //printf("Max: %d\n", max);
            create_processes(n - 1, start, seg, num); // Recursive call for the second child
            exit(0);
        } else if (child2 < 0) { // Error handling
            perror("fork");
            exit(EXIT_FAILURE);
        } else{
            max = findMax(num, start, start+seg-1);
            //printf("Max: %d\n", max);
        }
        int status;
        waitpid(child1, &status, 0);
        waitpid(child2, &status, 0);
        exit(0);
    } else { // Error handling
        perror("fork");
        exit(EXIT_FAILURE);
    }
}

int calculate_depth(int n) {
    return (int)ceil((log2(n + 1)) - 1);
}