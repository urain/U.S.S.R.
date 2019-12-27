#include <signal.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <execinfo.h>
#include <string.h>
#include <sys/mman.h>
 
int myCheckinVar = 0;
pthread_mutex_t done = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
char buf[1024];
// setup blacklist for bad syscalls
uint8_t *blacklist;//[529];
uint64_t syscallNum = 0;
uint64_t myRsp = 0;
uint64_t myRbp = 0;
uint64_t myRdi = 0;
uint64_t myRsi = 0;
uint64_t myRdx = 0;
uint64_t myRcx = 0;
uint64_t myR10 = 0;
uint64_t myR08 = 0;
uint64_t myR09 = 0;
uint64_t myStk1= 0;
uint64_t myStk2= 0;
uint64_t failed = 0;
uint64_t succeeded = 0;
 
void killAll(){
    blacklist[syscallNum] += 1;
    FILE *fp = fopen("/Users/user/Desktop/blacklist.txt", "wb");
    fwrite(blacklist, 529, 1, fp);
    fclose(fp);
    
    //system("/bin/bash -c 'echo password | sudo -S echo \"sleep 1\necho password | sudo -S kill -9 `pgrep -d \" \" sudo test` \nsudo /Users/user/Desktop/test & \nexit\" > /Users/user/Desktop/kill.sh'");
    //system("/bin/bash -c 'echo password | sudo -S chmod +x /Users/user/Desktop/kill.sh'");
    //system("/bin/bash -c 'echo password | sudo -S /Users/user/Desktop/kill.sh &'");
    system("/bin/bash -c '/Users/user/Desktop/kill.sh &'");
    
    sleep(10);
    
    exit(0);
}
 
 
void myCheckinThread(){
    int fail = 0;
    while( fail < 5 ){
        if ( blacklist[5] == 1){
            fail = 0;
            blacklist[5] = 0;
            //printf("checkin good\n");
        }
        else{
            fail += 1;
        }
        sleep(1);
    }
    killAll();
}
 
void mySyscallThread(){
    //mprotect(blacklist, 529, PROT_WRITE);
    blacklist[syscallNum] += 1; // preemptively increase the fail count
    //mprotect(blacklist, 529, PROT_READ);
    failed += 1;
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
    // if we succeeded, decrease the fail count
    //mprotect(blacklist, 529, PROT_WRITE);
    blacklist[syscallNum] = 0; //reset blacklist count
    //mprotect(blacklist, 529, PROT_READ);
    
    pthread_cond_signal(&cond);
    //pthread_cancel(pthread_self());
    return;
}
 
 
void sig_handler(int signo)
{
    //mprotect(blacklist, 529, PROT_WRITE);
    blacklist[syscallNum] += 1;
    //mprotect(blacklist, 529, PROT_READ);
 
    //void *array[10];
    //size_t size;
 
    //printf("\r%llu syscall failed                 ",syscallNum);
    switch(signo){
            
        case SIGSYS:
        case SIGABRT:
        case SIGSEGV:
        case 4:
            killAll();
            sleep(10);
            exit(0);
            /*printf("MY SIGSEGV\n");
            fflush(stdout);
            asm{
                .intel_syntax noprefix
            myLoop:
                jmp myLoop
            }*/
            //break;
        default:
            //blacklist[syscallNum] += 1;
 
            /*printf("%s\n",strerror(errno));
            // get void*'s for all entries on the stack
            size = backtrace(array, 10);
            // print out all the frames to stderr
            fprintf(stderr, "Error: signal %d:\n", signo);
            backtrace_symbols_fd(array, size, STDERR_FILENO);
            pthread_kill(pthread_self(),9);*/
            break;
    }
    return;
}
 
int main(int argc, const char * argv[]) {
    
    int KEY = 27;
    int SIZE = 529;
    int sharedMemId = shmget(KEY, SIZE, IPC_CREAT | 0666);
    if (sharedMemId < 0){
        printf("Could Not Setup Shared Memory\n");
        return 0;
    }
    blacklist = shmat(sharedMemId, NULL, 0);
    if (blacklist <= 0){
        printf("Could Not Map In Shared Memory\n");
        return 0;
    }
 
    
    if ( argv[1] ){
        if(atoi(argv[1]) == 1){
            memset(blacklist, 0, 529);
        }
    }
    
    FILE *fp = fopen("/Users/user/Desktop/blacklist.txt", "a");
    fclose(fp);
    fp = fopen("/Users/user/Desktop/blacklist.txt", "rb");
    fread(blacklist, 529, 1, fp);
    fclose(fp);
    
    
    blacklist[37] = 0xff;
    blacklist[55] = 0xff;
    blacklist[66] = 0xff;
    blacklist[73] = 0xff;
    blacklist[111] = 0xff;
    blacklist[126] = 0xff;
    blacklist[134] = 0xff;
    blacklist[183] = 0xff;
    blacklist[244] = 0xff;
    blacklist[361] = 0xff;
    blacklist[380] = 0xff;
    blacklist[433] = 0xff;
    blacklist[434] = 0xff;
    blacklist[435] = 0xff;
    blacklist[520] = 0xff;
    blacklist[521] = 0xff;
    blacklist[529] = 0xff;
 
 
 
 
    
    signal(SIGSYS, sig_handler);
    signal(SIGABRT, sig_handler);
    signal(SIGSEGV, sig_handler);
    signal(4, sig_handler);
    //pthread_t myCheckinThread_t;
    //pthread_create(&myCheckinThread_t, 0, (void*)myCheckinThread, 0);
    
    /*int fuzzerPid = getpid();
    mprotect(blacklist, 529, PROT_WRITE);
    memcpy(blacklist, &fuzzerPid, 3);
    mprotect(blacklist, 529, PROT_READ);*/
    
    /*while(blacklist[5] != 0xff){
        continue;
    }*/
    
    uint64_t runs = 0;
    
    while(1){
        runs += 1;
        usleep(100);
        
        // checkin with timeout program
        //mprotect(blacklist, 529, PROT_WRITE);
        blacklist[5] = 1;
        //mprotect(blacklist, 529, PROT_READ);
        
        if ( runs % 500 == 0 ){
            printf("\n\n");
            // print the blasklist bytemap
            for ( int i = 0; i < 529; i++){
                if (i % 26 == 0 && i != 0){
                    printf("\n");
                }
                /*else if ( i % 8 == 0 && i != 0){
                 printf(" ");
                 }*/
                printf("%02x ",blacklist[i]);
            }
            
            printf("\nLASTCALL:\t%llu\t%d\n",syscallNum,blacklist[syscallNum]);
            printf("SUCCEEDED:\t%llu\n",succeeded);
            printf("FAILED:\t\t%llu\n",failed);
            fflush(stdout);
        }
 
        // choose syscall
        while(1){
            syscallNum = (arc4random() % 522) + 6;
            if (blacklist[syscallNum] <= 0x5){
                break;
            }
            else{
                blacklist[syscallNum] = 0xff;
            }
        }
        /*mprotect(blacklist, 529, PROT_WRITE);
        memcpy(blacklist+3, &syscallNum, 2);
        mprotect(blacklist, 529, PROT_READ);*/
        
        //printf(".");
        //fflush(stdout);
        // backup vars for stack
        myRsp = 0;
        myRbp = 0;
        
        // Choose args
        
        myRdi = ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        myRsi = ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        myRdx = ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        myRcx = ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        myR10 = ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        myR08 = ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        myR09 = ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        myStk1= ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
        myStk2= ((uint64_t)arc4random() << 32 | arc4random()) >> (arc4random()%64);
  
        
        
        // backup args to file
        snprintf(buf, 1024, "%llu\n%016llx\n%016llx\n%016llx\n%016llx\n%016llx\n%016llx\n%016llx\n%016llx\n%016llx\n", syscallNum, myRdi, myRsi, myRdx, myRcx, myR10, myR08, myR09, myStk1, myStk2);
        FILE *fptr;
        fptr = fopen("/Users/user/Desktop/arg.txt","w");
        fwrite(buf, sizeof(buf), 1, fptr);
        fclose(fptr);
        
        // create the syscall thread
        pthread_t mySyscallThread_t;
        pthread_create(&mySyscallThread_t, 0, (void*)mySyscallThread, 0);
        //pthread_detach(mySyscallThread_t);
        
        // Start the timer to wait for thread
        int rc;
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME,&ts);
        ts.tv_sec += 0;
        ts.tv_nsec += 500000;
        
        // Lock the mutex
        pthread_mutex_lock(&done);
        rc = pthread_cond_timedwait(&cond, &done, &ts);
        pthread_mutex_unlock(&done);
        
        // Check condition
        if ( rc == ETIMEDOUT ){
            //printf("\n%02x\t%llu syscall timed out         \n",blacklist[syscallNum],syscallNum);
            //if (pthread_kill(mySyscallThread_t,0)){
                //pthread_cancel(mySyscallThread_t);
            //}
            //mprotect(blacklist, 529, PROT_WRITE);
            blacklist[syscallNum] += 1;
            //mprotect(blacklist, 529, PROT_READ);
            failed += 1;
            
            FILE *fp = fopen("/Users/user/Desktop/blacklist.txt", "wb");
            fwrite(blacklist, 529, 1, fp);
            fclose(fp);
        } else {
            succeeded += 1;
            if (blacklist[syscallNum] > 0){
                //mprotect(blacklist, 529, PROT_WRITE);
                blacklist[syscallNum] = 0;
                //mprotect(blacklist, 529, PROT_READ);
 
            }
            //printf("\n%llu syscall succeeded            \n",syscallNum);
        }
 
    }
 
    return 0;
}
