#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


#define NUM_PROCESSES 3
#define READ_END 0
#define WRITE_END 1


int findMax(int arr[], int start, int end) {
   int max = arr[start];  // Initialize max with the first element of the specified part
  
   for (int i = start + 1; i <= end; i++) {
       if (arr[i] > max) {
           max = arr[i];  // Update max if a larger element is found
       }
   }
  
   return max;
}


void createChild(int PN, int nums[], int start, int seg, int fd[][2]) {
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
       createChild(PN - 1, nums, start + seg, seg, fd);
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
       wait(NULL);  // Wait for child process to finish
       read(fd[PN-1][READ_END], &maxChild, sizeof(maxChild));  // Read max value from the pipe
        if (localMax > maxChild){
           maxChild = localMax;
       }
       printf("Parent PID: %d, Max value received from child: %d\n", getpid(), maxChild);
       write(fd[PN][WRITE_END],&maxChild,sizeof(maxChild));
      
       close(fd[PN-1][READ_END]);  // Close read end
       exit(EXIT_SUCCESS);
   }
}


int main() {
   int num[] = {6, 2, 3, 4, 5, 1};
   pid_t parent_pid = getpid();
   printf("Parent PID: %d\n", parent_pid);
  
   int fd[4][2];
   createChild(NUM_PROCESSES, num, 0, 2, fd);


   return 0;
}
