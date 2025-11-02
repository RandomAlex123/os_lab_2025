#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("Создание зомби-процесса...\n");
    
    pid_t pid = fork();
    
    if (pid == 0) {
        printf("Дочерний: PID=%d завершает работу\n", getpid());
        exit(0);
    } else {
        printf("Родитель: PID=%d, создал дочерний PID=%d\n", getpid(), pid);
        printf("Дочерний процесс стал зомби на 30 секунд...\n");

        sleep(30);
        
        printf("Убираем зомби...\n");
        wait(NULL);
    }
    
    return 0;
}