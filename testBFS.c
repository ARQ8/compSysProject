#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>
int findMax(int arr[], int start, int end) {
   int max = arr[start];  // Initialize max with the first element of the specified part
  
   for (int i = start + 1; i <= end; i++) {
       if (arr[i] > max) {
           max = arr[i];  // Update max if a larger element is found
       }
   }
  
   return max;
}

void create_processes(int n, int start, int seg, int num[]) {
    if (n == 0)
        return;

    pid_t child1, child2;
    child1 = fork();
    int max;
    if (child1 == 0) { // Child process
        printf("Child %d created with parent %d\n", getpid(), getppid());
        max=findMax(num,start+seg,start+seg+seg-1);
        printf("Max: %d\n",max);
        start = start+(3*seg);
        create_processes(n - 1,start,seg,num); // Recursive call for the first child
        exit(0);
    } else if (child1 > 0) { // Parent process
        child2 = fork();

        if (child2 == 0) { // Second child
            printf("Child %d created with parent %d\n", getpid(), getppid());
            max=findMax(num,start+(2*seg),start+(2*seg)+seg-1);
            printf("Max: %d\n",max);
            create_processes(n - 1,start,seg,num); // Recursive call for the second child
            exit(0);
        } else if (child2 < 0) { // Error handling
            perror("fork");
            exit(EXIT_FAILURE);
        } else{
            max=findMax(num,start,start+seg-1);
            printf("Max: %d\n",max);
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

int main() {
    int n = 3;
    int start =0;
    int seg =2;
    int num[] = {1,2,3,4,5,6};
    create_processes(calculate_depth(n),start,seg,num);

    return 0;
}
