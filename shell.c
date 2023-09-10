#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

int count;
char str[10000];
char *history[1000];
double details[1000][2];
int timings[1000][2];
int exit_code;
int num=0;
bool flag=false;

void handler(int num2){
    //printing the details upon termination
    printf("\n");
    for(int i=0; i<num; i++){
        printf("%d. %s:\n ppid:\t\t%.0lf\n exec_time:\t%lfs\n time:\t\t%02d:%02d\n\n",i+1,history[i],details[i][0],details[i][1],timings[i][0],timings[i][1]);
    }
    exit(0);
}

int create_process_and_run(char *cmd[]){
    //getting current time
    time_t t = time(NULL);
    struct tm date=*localtime(&t);

    //entering details
    timings[num-1][0]=date.tm_hour;
    timings[num-1][1]=date.tm_min;

    //setting the start time
    struct timespec start;
    timespec_get(&start, TIME_UTC);

    //creating a fork
    int status = fork();
    if (status == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    //checking which if the fork is child or parent
    if (status==0){
        //terminating the child process and executing the system call in place of that
        if (execvp(cmd[0],cmd)==-1){
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
    else{
        //waiting for the child process to complete
        int ret;
        int pid = wait(&ret);

        //typecasting int to double
        double pid_st=(double)pid;

        //getting the end time
        struct timespec end;
        timespec_get(&end, TIME_UTC);

        //calculating execution time
        double time_spent = (end.tv_sec-start.tv_sec)+(end.tv_nsec-start.tv_nsec)/1000000000.0;
        
        //entering the details
        details[num-1][0]=pid_st;
        details[num-1][1]=time_spent;
    }
    return 0;
}

int create_piped_process_run(char *cmd[]){

    int inputSize = 0;
    int j=0;
    while(cmd[j]!=NULL){
        inputSize++;
        j++;
    }

    // Initializing an array to hold subarrays
    char **subarrays[100][1000];
    int numSubarrays = 0;

    int k=0;
    j=0;
    for (int i = 0; i < inputSize; i++) {
        if (strcmp(cmd[i], "|") == 0) {
            // Found a "|" character, finalize the current subarray 
            subarrays[j++][k] = NULL;

            k=0;
        } else {
            // Add the string to the current subarray
            subarrays[j][k++]=strdup(cmd[i]);

        }
    }
    //calculating number of command
    numSubarrays=j+1;

    if (numSubarrays==2){
        //creating a pipe
        int fd[2];
        if (pipe(fd)==-1){
            printf("Some error occured");
            return 1;
        }
        //getting current time
        time_t t = time(NULL);
        struct tm date=*localtime(&t);

        //entering details
        timings[num-1][0]=date.tm_hour;
        timings[num-1][1]=date.tm_min;

        //setting the start time
        struct timespec start;
        timespec_get(&start, TIME_UTC);

        //creating a fork
        int status = fork(); 
        if (status == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        
        if (status==0){
        //changing stdout
            close(fd[0]);
            dup2(fd[1],STDOUT_FILENO);
            close(fd[1]);
            //copying the arguments from subarray into arg
            char *arg[1000];
            int m =0;
            while(subarrays[0][m]!=NULL){
                arg[m++]=strdup(subarrays[0][m]);
            }
            arg[m]=NULL;
            //giving arg to exec as a argument
            if (execvp(arg[0],arg)==-1){
                perror("execvp");
                exit(EXIT_FAILURE);
            }
                    
        }
        else{
            //wait
            int ret;
            int pid = wait(&ret);
            //2nd child process
            int status2 = fork();
            if (status2 == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            if (status2==0){
                //changing the std input
                close(fd[1]);
                dup2(fd[0],STDIN_FILENO);
                close(fd[0]);
                //copying the arguments from subarray into arg
                char *arg[1000];
                int m =0;
                while(subarrays[1][m]!=NULL){
                    arg[m++]=strdup(subarrays[1][m]);
                }
                arg[m]=NULL;
                //giving arg to exec as arguments
                if (execvp(arg[0],arg)==-1){
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            else{
                close(fd[0]);
                close(fd[1]);
                int ret;
                int pid = wait(&ret);
                double pid_st=(double)pid;

                //getting the end time
                struct timespec end;
                timespec_get(&end, TIME_UTC);

                //calculating execution time
                double time_spent = (end.tv_sec-start.tv_sec)+(end.tv_nsec-start.tv_nsec)/1000000000.0;
                
                //entering the details
                details[num-1][0]=pid_st;
                details[num-1][1]=time_spent;
            }
        }

    }

    else if(numSubarrays==3){
    //getting current time
    time_t t = time(NULL);
    struct tm date=*localtime(&t);

    //entering details
    timings[num-1][0]=date.tm_hour;
    timings[num-1][1]=date.tm_min;

    //setting the start time
    struct timespec start;
    timespec_get(&start, TIME_UTC);

    int pipe1[2]; // Pipe between command 1 and command 2
    int pipe2[2]; // Pipe between command 2 and command 3

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    int status1 = fork();
    if (status1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (status1 == 0) {
        // Child process 1 (Command 1)
        close(pipe1[0]);
        close(pipe2[0]);
        close(pipe2[1]); 

        // Redirect stdout to write end of pipe1
        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[1]);

        // Execute Command 1 
            char *arg[1000];
            int m =0;
            while(subarrays[0][m]!=NULL){
                arg[m++]=strdup(subarrays[0][m]);
            }
            arg[m]=NULL;
            //giving arg to exec as a argument
            if (execvp(arg[0],arg)==-1){
                perror("execvp");
                exit(EXIT_FAILURE);
            }
    }

    int status2 = fork();
    if (status2 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (status2 == 0) {
        // Child process 2 (Command 2)
        close(pipe1[1]); 
        close(pipe2[0]); 

        // Redirect stdin from read end of pipe1
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);

        // Redirect stdout to write end of pipe2
        dup2(pipe2[1], STDOUT_FILENO);
        close(pipe2[1]);

        // Execute Command 2 
            char *arg[1000];
            int m =0;
            while(subarrays[1][m]!=NULL){
                arg[m++]=strdup(subarrays[1][m]);
            }
            arg[m]=NULL;
            //giving arg to exec as a argument
            if (execvp(arg[0],arg)==-1){
                perror("execvp");
                exit(EXIT_FAILURE);
            }
    }

    int status3 = fork();
    if (status3 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (status3 == 0) {
        // Child process 3 (Command 3)
        close(pipe1[0]); 
        close(pipe1[1]); 
        close(pipe2[1]); 

        // Redirect stdin from read end of pipe2
        dup2(pipe2[0], STDIN_FILENO);
        close(pipe2[0]);

        // Execute Command 3 (e.g., "wc")
            char *arg[1000];
            int m =0;
            while(subarrays[2][m]!=NULL){
                arg[m++]=strdup(subarrays[2][m]);
            }
            arg[m]=NULL;
            //giving arg to exec as a argument
            if (execvp(arg[0],arg)==-1){
                perror("execvp");
                exit(EXIT_FAILURE);
            }
    }

    // Parent process
    close(pipe1[0]); // Close read end of pipe1
    close(pipe1[1]); // Close write end of pipe1
    close(pipe2[0]); // Close read end of pipe2
    close(pipe2[1]); // Close write end of pipe2

    int status;
    waitpid(status1, &status, 0);
    waitpid(status2, &status, 0);
    waitpid(status3, &status, 0);
    int pid = getppid();
    //typecasting pid
    double pid_st=(double)pid;
    //getting the end time
    struct timespec end;
    timespec_get(&end, TIME_UTC);

    //calculating execution time
    double time_spent = (end.tv_sec-start.tv_sec)+(end.tv_nsec-start.tv_nsec)/1000000000.0;
                
    //entering the details
    details[num-1][0]=pid_st;
    details[num-1][1]=time_spent;

    }

    return 0;
}

bool check_pipe(char *cmd[]){
    bool check = false;
    int i=0;
    while(cmd[i]!=NULL){
        if (strcmp(cmd[i++],"|")==0){
            check=true;
            break;
        }
    }
    return check;
}

int launch(char *cmd[]){
    int exit_code;
    flag=check_pipe(cmd);
    if (flag){
        exit_code=create_piped_process_run(cmd); //calls piped function
    }
    else{
        exit_code=create_process_and_run(cmd); //calls the funtion to create a child process
    }
    return exit_code;
}

void _history(){
    for (int i=0;i<num;i++){
        printf("%d. %s\n",i+1,history[i]);
    }
}

int main(){
    //handling sigint
    struct sigaction sa;
    sa.sa_handler=handler;

    sigaction(SIGINT, &sa, NULL);

    do{
        count=0;
        char *cmd[10000];//command array

        printf("[SampleShell ]$ ");
        fgets(str, sizeof(str),stdin);
        str[strlen(str)-1]='\0';

        //history command is handled differently
        if (strcmp(str,"history")==0){
            _history();
        }
        else{

            //adding the command to the history
            history[num++]=strdup(str);

            // Tokenize the string
            char *token = strtok(str, " ");

            while (token != NULL) {
                // Allocate memory for the word and copy it
                cmd[count] = strdup(token);

                // Move to the next token
                token = strtok(NULL, " ");
                count++;
            }

            // Null-terminate the array
            cmd[count] = NULL;

            //launch the command
            exit_code=launch(cmd);
        }

    }while(true);
    return 0;
}
