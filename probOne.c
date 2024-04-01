#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FILENAME "randomNumbers.txt" // File being written to

// Function prototypes
void generateRandomNumbers(int L);
void hiddenKeys(int H, int L);

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
    printf("Sum of all numbers: %d\n", sum);
    printf("Max=%d, Avg=%.2f\n", max, average);

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
        position = rand() % L;
        fseek(file, position * sizeof(int), SEEK_SET);
        fprintf(file, "\n%d\n", num);
    }

    fclose(file);
    printf("Secret keys hidden in file\n");
}