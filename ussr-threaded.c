#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
 
int myCheckinVar = 0;
 
void myCheckinThread(){
    int fail = 0;
    while( fail < 3 ){
        if ( myCheckinVar == 1){
            fail = 0;
            myCheckinVar = 0;
            //printf("checkin good\n");
        }
        else{
            fail += 1;
            printf("checkin failed: %d\n",fail);
        }
        sleep(1);
    }
    kill(getpid(),9);
}
 
void myHungThread(){
    /*for (int i = 0; i < 5; i++){
        myCheckinVar = 1;
        sleep(1);
    }
    printf("hung thread hangin out\n");*/
    /*printf("hung thread bout to segfault\n");
    asm(
        "mov 0, %%rax \n"
        "mov (%%rax), %%rax \n"
        :::
        );*/
    uint8_t blacklist[529];
    blacklist[55] = 1;
    blacklist[66] = 1;
    while(1){
 
        myCheckinVar = 1;
        int syscallNum = 0;
        while(1){
            syscallNum = arc4random()%524 + 5;
            if (blacklist[syscallNum] == 1){
                continue;
            }
            else{
                break;
            }
        }
        char buf[1024];
        
        uint64_t myRsp = 0;
        uint64_t myRbp = 0;
        uint64_t myRdi = ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        uint64_t myRsi = ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        uint64_t myRdx = ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        uint64_t myRcx = ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        uint64_t myR10 = ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        uint64_t myR08 = ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        uint64_t myR09 = ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        uint64_t myStk1= ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        uint64_t myStk2= ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        
        snprintf(buf, 1024, "%d\n%016llx\n%016llx\n%016llx\n%016llx\n%016llx\n%016llx\n%016llx\n%016llx\n%016llx\n", syscallNum, myRdi, myRsi, myRdx, myRcx, myR10, myR08, myR09, myStk1, myStk2);
        FILE *fptr;
        fptr = fopen("/Users/user/Desktop/arg.txt","w");
        fwrite(buf, sizeof(buf), 1, fptr);
        fclose(fptr);
        fclose(fptr);
        usleep(3);
 
       asm(
            "mov %%rsp, %0 \n"
            "mov %%rbp, %1 \n"
            "mov %2, %%rax \n"
            "add $0x2000000, %%rax \n"
            "mov %3, %%rdi \n"
            "mov %4, %%rsi \n"
            "mov %5, %%rdx \n"
            "mov %6, %%rcx \n"
            "mov %7, %%r10 \n"
            "mov %8, %%r8 \n"
            "mov %9, %%r9 \n"
            "mov %10, %%r11 \n"
            "push %%r11 \n"
            "mov %11, %%r11 \n"
            "push %%r11 \n"
            "syscall \n"
            "mov %12, %%rsp \n"
            "mov %13, %%rbp \n"
            : "=m"(myRsp), "=m"(myRbp)
            : "m"(syscallNum), "m"(myRdi), "m"(myRsi), "m"(myRdx), "m"(myRcx), "m"(myR10), "m"(myR08), "m"(myR09), "m"(myStk1), "m"(myStk2), "m"(myRsp), "m"(myRbp)
            :
            );
        printf("%d syscall succeeded\n",syscallNum);
        usleep(1);
    }
}
 
int main(int argc, const char * argv[]) {
 
    pthread_t myCheckinThread_t;
    pthread_t myHungThread_t;
    
    pthread_create(&myCheckinThread_t, 0, (void*)myCheckinThread, 0);
    pthread_create(&myHungThread_t, 0, (void*)myHungThread, 0);
 
    pthread_join(myCheckinThread_t,0);
    pthread_join(myHungThread_t,0);

    return 0;
}
