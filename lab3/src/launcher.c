#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("Starting sequential_min_max...\n");
    
    pid_t pid = fork();
    
    if (pid == 0) {
        char *args[] = {
            "./sequential_min_max",
            "5", 
            "10000",
            NULL
        };
        
        execv(args[0], args);
        
        printf("Error: failed to start sequential_min_max\n");
        return 1;
    } else {
        wait(NULL);
        printf("sequential_min_max finished\n");
    }
    
    return 0;
}