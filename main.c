//
//  main.c
//  project1
//
//  Created by Nan  on 10/7/20.
//

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <signal.h>


#define MAX_LINE 80

static char signal_buff[MAX_LINE];
int cmdNum=0;
typedef struct cmdNode{
    char *data;
    //int order;
    struct cmdNode *next;
} cmdNode;

cmdNode *head = NULL;

bool isEmpty() {
    return head == NULL;
}

void insert(char *new_data)
{
    if(cmdNum > 100) {
        printf("List is full now\n");
        return;
    }
    char *ptr = (char*)malloc(sizeof(char));
    strcpy(ptr,new_data);
    cmdNode* link =(cmdNode*)malloc(sizeof(cmdNode));
    link->data = ptr;
    link->next=head;
    head = link;
    cmdNum++;
}
void printRecent10(){
    int counter = 1;
    int index = cmdNum;
    cmdNode* ptr =(cmdNode*)malloc(sizeof(cmdNode));
    ptr=head;
    printf("-------History------\n");
    while(ptr!=NULL && counter <=10){
        printf("%d\t%s\n",index--,ptr->data);
        ptr=ptr->next;
    }
}
 cmdNode *getNthCMD(int index){
     cmdNode *current =(cmdNode*)malloc(sizeof(cmdNode));
     current = head;
     for(int i=cmdNum;i>index &&current;i--)
         current=current->next;
     return current;
 }
 void insertAt(cmdNode *nthNd, int index){
     cmdNode *current = (cmdNode*)malloc(sizeof(cmdNode));
     cmdNode *temp = (cmdNode*)malloc(sizeof(cmdNode));
     temp=nthNd;
     current = head;
     int i = cmdNum;//counter for nth+1 position
     while(i > index+1&&current) {
         current = current->next;
         i--;
     }
     current->next = temp->next;//connecting before and after nodes
     temp->next=head;//move to the recent position
     head = temp;
 }
void execute(char **args, int tail){
    pid_t pid;
    pid = fork();
    if(pid < 0) {/* error occured */
        fprintf(stderr, "Fork Failed");
        exit(0);
    }
    else if(pid == 0){
        if(strcmp(args[tail-1],"&")!=0){
            if(execvp(args[0], args) == -1){
                perror("execvp");
                exit(0);
            }
        }
        else {
            args[tail-1]=NULL;
            if(execvp(args[0], args) == -1){
                perror("execvp");
                exit(0);
            }
        }
    }//end child process
    else {
        wait(NULL);
    }//end parents
}
void tokenize(const char *str){// tokenize input and execute
    char *input = (char*)malloc(sizeof(char));
    //strcpy(input,str);
    //printf("%c\n",input[0]);
    for(int i=0; i < strlen(str); i++) {
        if(str[i] == '&'){
            char temp;
            temp = str[i];
            input[i] =' ';
            input[i+1]=temp;
            //printf("%c\n",temp);
        }
        else input[i]=str[i];
    }
    printf("%s\n",input);
    int tail=0;
    char *token[MAX_LINE];
    token[tail]=strtok(input," \n");
    while(token[tail]!=NULL){
        token[++tail]=strtok(NULL," \n");
    }
    execute(token, tail);
}
cmdNode *getRecent1(){
    cmdNode *current =(cmdNode*)malloc(sizeof(cmdNode));
    current = head;
    return current;
}
/* the signal handler function */
void handle_SIGINT() {
    //write(STDOUT_FILENO,signal_buff,strlen(signal_buff));
        signal(SIGINT, handle_SIGINT);
        if(isEmpty() && cmdNum ==0) {
            printf("History is empty. Please enter command\n");
            printf("osh>");
            fflush(stdout);
        }
        else {
            printRecent10();
            printf("osh>");
            fflush(stdout);
        }
}
int main(int argc, const char * argv[]) {
    /* set up the signal handler */
    struct sigaction handler;
    handler.sa_handler = handle_SIGINT;
    sigaction(SIGINT, &handler, NULL);
    signal(SIGINT, handle_SIGINT);
    
    char str[MAX_LINE];
    int should_run = 1;
    
    while(should_run){
        //get user inputs
        printf("osh>");
        fflush(stdout);
        fgets(str, MAX_LINE, stdin);
        if(str[strlen(str)-1]=='\n') str[strlen(str)-1]=0;//delete newline after fgets
        
        if(strcmp(str, "exit") ==0){// if exit
            should_run=0;
            exit(0);
        }
        if(str[0]=='\0'|| str[0]==' ') // if input is empty, back to input
        {
            printf("Enter a command or enter \'exit\' to quit program\n");
        }
        else{// if input is not empty
            if(strcmp(str, "history")==0){// if history
               if(isEmpty() && cmdNum ==0) printf("History is empty. Please enter command\n");
               else printRecent10();
                insert(str);
            }
            else if(strcmp(str, "!!")==0){// !! execute last command
               if(isEmpty() && cmdNum ==0) printf("History is empty. Please enter command\n");
               else {
                   cmdNode *recent;
                   recent = getRecent1();
                   printf("Last command: %s\n",recent->data);
                   if(strcmp(recent->data,"!!")==0) {
                       printf("Oops! the recent one is !! already!");
                       continue;
                   }
                   if(strcmp(recent->data,"history")==0) printRecent10();
                   else tokenize(recent->data);
                   insert(str);
                   insert(recent->data);
               }
               
            }
            else if(str[0]=='!'){// if !Nth command
                int val;
                cmdNode *current;
                char originalInput[MAX_LINE];
                strcpy(originalInput,str);
                if(isEmpty() && cmdNum ==0) printf("History is empty. Please enter command\n");
                else {
                    memmove(str, str+1, strlen(str));//mutation the string
                    val = atoi(str);
                    if(val <=0 || val > cmdNum) {
                        printf("Out of bound. There are %d command(s)\n",cmdNum);
                        continue;
                    }
                    if(val == cmdNum) {
                        current = getRecent1();
                        printf("No.%d command: %s\n", val, current->data);
                        if(strcmp(current->data,"history")==0) printRecent10();
                        else tokenize(current->data);
                        insert(originalInput);
                        insert(current->data);
                    }
                    else {
                        //find the Nth cmd
                        current = getNthCMD(val);
                        if(strcmp(current->data,"!!")==0) {
                            printf("Oops! the recent one is !! already!\n");
                            continue;
                        }
                        if(strcmp(current->data,"history")==0) {
                            printRecent10();
                        }
                        else tokenize(current->data);
                        insert(originalInput);
                        //shift the place to most recent
                        insertAt(current,val);
                    }
                }
            }
//            else if(str[0] == 'r'){// runnning r x or r
//                //if(isEmpty() && cmdNum ==0) printf("History is empty. Please enter command\n");
//                if(strcpy(str, "r")) printf("r");
//                char c;
//                for(int i =1; i<strlen(str);i++){
//                    if(isalpha(str[i])) c = str[i];
//                }
//                cmdNode *current;
//                current = head;
//                while(current) {
//                    if(c == current->data[0]) tokenize(current->data);
//                    current = current->next;
//                }
//                insert(str);
//                insert(current->data);
//            }
            else {// regular input
                insert(str);
                tokenize(str);
            }
        }
    }//end while
    return 0;
    } //end main
