#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */
int parse_input(char *input, cmd_buff_t *cmd){
    cmd->_cmd_buffer = input; // using buffer instanitiated in exec_local_cmd_loop
    char *buffer = cmd->_cmd_buffer; // buffer is now an alias for _cmd_buffer
    cmd->argc =0; // set the arg count in the struct to zero

    //trimming the leading spaces of the input
    while(isspace((unsigned char)*buffer)) buffer++;
    // printf("Current buff: %s\n", buffer);

    // main parsing of user command
    char *arg =buffer; // set arg to the start of the buffer
    int in_quotes =0; // tracking if we are in a quoted string
    char *write_ptr = buffer;  // setting up a pointrt to write out the cleaned output

    while (*buffer){
        if (*buffer == '"'){
            in_quotes = !in_quotes; // if we hit quotes we toggle the inquotes variable
        } else if(isspace((unsigned char)*buffer) && !in_quotes){ //if we hit a space and we're not in quotes
                *write_ptr++ ='\0'; // null terminated the current arg since we are at the end
                if (arg[0] != '\0') { // If the current argument is not empty
                    cmd->argv[cmd->argc++] = arg; // add the current argment to argv
            }
            // eliminate trailing spaces
            while (isspace((unsigned char)* (++buffer)));
            arg = buffer; // set the arg to that beignin of next arg
            continue;
        }
        *write_ptr++ = *buffer++; // copy obver the characters to write ptr   
    }
    *write_ptr = '\0'; //null terminate the last argument
    if (arg[0] != '\0'){ // checking that the last argument is not empty
        cmd->argv[cmd->argc++] =arg; // add the argument to the argv list
    }
    cmd->argv[cmd->argc] = NULL; // null terminating the argv list

    return 0;
}


int exec_local_cmd_loop()
{
    char cmd_buff[ARG_MAX];
    int rc = 0;
    cmd_buff_t cmd;

    // TODO IMPLEMENT MAIN LOOP

    while(1){

        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
            printf("\n");
            break;
        }
        //remove the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
        printf("Command entered: %s\n", cmd_buff);
        rc = parse_input(cmd_buff, &cmd);
        if (rc!=0){
            printf("ERROR CODE: %d", rc);
            exit(1);
        } 

        if(strcmp(cmd.argv[0],"exit")==0){
            exit(1);
        }
        // Print the argument array
        printf("Parsed arguments:\n");
        for (int i = 0; i < cmd.argc; i++) { //  since we have direct access to cmd struct and not accesing
                                            //via pointer we can use the (.) notation to access its attributes
            printf("argv[%d]: %s\n", i, cmd.argv[i]);
        }

        if (cmd.argc >0 && strcmp(cmd.argv[0], "cd")==0){// check if first arg is cd
            if (cmd.argc ==1){// check if cd has no args (do nothing)
                continue;
            }else if (cmd.argc == 2){
                if (chdir(cmd.argv[1]) != 0){ // if chdir fails print the appropriate error message
                    perror("chdir");
                }
                continue;
            } else {
                printf("TOO MANY ARGS FOR CD\n");
                continue;
            }

        }
        // create fork & exec processes for external commands
        pid_t pid = fork(); // create a new child process by duplicated the current one - creating a child
        if (pid == 0) {//child process
        execvp(cmd.argv[0], cmd.argv); // execvp the external command if child process
        perror("execvp"); // throw error if the execvp returns an error, if not it won't return anything
        exit(1);
        } else if(pid >0){// parent process
            int status;
            waitpid(pid, &status, 0); // wait for child process to finish where status is the exit status of
        } else {                      // the child process
            perror("fork failure"); // print fork failure when fork returns -1 to pid
        }


    }

 
        //IMPLEMENT THE REST OF THE REQUIREMENTS
    

    // TODO IMPLEMENT if not built-in command, fork/exec as an external command
    // for example, if the user input is "ls -l", you would fork/exec the command "ls" with the arg "-l"

    return OK;
}
