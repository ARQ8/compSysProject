#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FILENAME "randomNumbers.txt" // File being written to
#define L 10000     // Number of integers to generate
#define H_MIN 30    // Minimum number of hidden keys allowed
#define H_MAX 60    // Maximum number of hidden keys allowed

int main(int argc, char* argv[]) {
    FILE *file;
    int i, num, sum, max;
    float average;

    file = fopen(FILENAME, "w");    // Open file

    // Check if file opened
    if (file == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    srand(time(0));

    // Generate random positive integers and write them to file
    for (i = 0; i < L; i++) {
        num = rand() % 10000 + 1;
        sum = sum + num;
        if (num > max){
            max = num;
        }
        fprintf(file, "%d\n", num);
    }

    average = (double)sum/L;

    fclose(file);
    printf("Random numbers generated and written to file\n");
    printf("Sum of all numbers: %d\n", sum);
    printf("Maximum: %d\n", max);
    printf("Average: %.3lf", average);

    return 0;
}