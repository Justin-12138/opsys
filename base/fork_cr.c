#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

int main(){
    int a=0;
    printf("process not created:%p\n",&a);
    pid_t pid=fork();
    // printf("length:\n",sizeof(pid));
    if (pid<0)
    {
        perror("fork error");
    }
    else if (pid==0)
    {
        a=10;
        printf("a(son)=:%d\n",a);
        printf("process(son) created:%p\n",&a);
        printf("child process created successfully\n");
        printf("current_pid=%d\n",getpid());
        printf("parent_pid = %d\n",getppid());
        // long long id;
        // printf("my id:\n");
        // scanf("%lld",&id);
        // printf("My ID x10 = %lld\n",id*10);

    }
    else if (pid>0)
    {
        wait(NULL);
        printf("a(parent)=%d\n",a);
        printf("process(parent) created:%p\n",&a);
        printf("parent process created sucessfully\n");
        printf("current_pid=%d\n",getpid());
        printf("parent_pid = %d\n",getppid());
        

    }
    printf("=========end==========\n");
    return 0;
}