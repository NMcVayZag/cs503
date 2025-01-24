#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define BUFF_SIZE 50

void print_msg(char *str){
    printf("%s\n", str);
}

int main(){
    char *msg = "hello class";
    // stack memory - not dynamic - it is defined as a literal string
    // the string lives in the const space of memory whereas the pointer to it lives
    //in the stack space

    char *buff = malloc(BUFF_SIZE);
    if (buff == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }
    // everytime you malloc something you have to free it
    // always check the result of malloc

    strcpy(buff, msg);
    // strcpy the msg into the buffer
    print_msg(msg);

    // free the buffer when it is no longer needed. Since we already printed we don't
    //need the buff otherwise we might get memory leaks/bugs/segfaults
    //free(buff);
    return 0;
}