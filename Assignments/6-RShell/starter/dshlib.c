#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"

/**** 
 **** FOR REMOTE SHELL USE YOUR SOLUTION FROM SHELL PART 3 HERE
 **** THE MAIN FUNCTION CALLS THIS ONE AS ITS ENTRY POINT TO
 **** EXECUTE THE SHELL LOCALLY
 ****
 */

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

    // check if buffer is empty
    if (*buffer == '\0') {
    cmd->argv[0] = NULL; // null terminate first arg
    printf("%s\n", CMD_WARN_NO_CMD);
    return WARN_NO_CMDS;
    }

    // main parsing of user command
    char *arg =buffer; // set arg to the start of the buffer
    int in_quotes =0; // tracking if we are in a quoted string
    char *write_ptr = buffer;  // setting up a pointrt to write out the cleaned output

    while (*buffer){
        if (*buffer == '"'){
            in_quotes = !in_quotes;
            buffer++; // if we hit quotes we toggle the inquotes variable
        } else if(isspace((unsigned char)*buffer) && !in_quotes){ //if we hit a space and we're not in quotes
                *write_ptr++ ='\0'; // null terminated the current arg since we are at the end
                if (arg[0] != '\0') { // If the current argument is not empty
                    cmd->argv[cmd->argc++] = arg; // add the current argment to argv
            }
            // eliminate trailing spaces
            while (isspace((unsigned char)* (++buffer)));
            arg = buffer; // set the arg to that beignin of next arg
            continue;
        }else {*write_ptr++ = *buffer++; // copy obver the characters to write ptr if not """ or space outside quotes
            }
        }
    *write_ptr = '\0'; //null terminate the last argument
    if (arg[0] != '\0'){ // checking that the last argument is not empty
        cmd->argv[cmd->argc++] =arg; // add the argument to the argv list
    }
    cmd->argv[cmd->argc] = NULL; // null terminating the argv list

    return 0;
}

int exec_local_cmd_loop(){
    char cmd_buff[ARG_MAX];
    int rc = 0;

    while(1){

        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
            printf("\n");
            break;
        }
        //remove the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';

        // separating the input via pipes using strtok
        char *commands[CMD_MAX];
        int num_commands = 0;
        char *command = strtok(cmd_buff, PIPE_STRING);
        while (command != NULL && num_commands < CMD_MAX) { // while we have a command and we haven't reached max commands
            commands[num_commands++] = command; // add the command to commands array
            command = strtok(NULL, PIPE_STRING); // select the next command
        }
        // Check if the number of commands exceeds the limit
        if (num_commands >= CMD_MAX) {
            printf("Error: too many commands");
            exit(EXIT_FAILURE);
        }

        // checking if there was no input
        if (num_commands == 0) {
            printf("%s\n", CMD_WARN_NO_CMD);
            continue;
        }

        int pipe_fds[2 * (num_commands - 1)]; // declaring a pipe file descrpiters array given that we need two file descriptors per pipe and there is one less pipe than no. of commands
        for (int i = 0; i < num_commands - 1; i++) { // creating each pipe with a read and write file descriptor that will be overriten by commands
            if (pipe(pipe_fds + 2 * i) < 0) { // runs pipe function to store the read and write file descripters in correct slots in array based on i
                perror("pipe"); // prints error on failure of piping
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < num_commands; i++) {
            cmd_buff_t cmd;
            memset(&cmd, 0, sizeof(cmd_buff_t)); // Reset cmd_buff_t structure

            // Trim leading and trailing spaces
            while (isspace((unsigned char)*commands[i])) commands[i]++;
            char *end = commands[i] + strlen(commands[i]) - 1;
            while (end > commands[i] && isspace((unsigned char)*end)) end--;
            *(end + 1) = '\0';

            // parse the command from commands array
            if (parse_input(commands[i], &cmd) == WARN_NO_CMDS) {
                printf("%s\n", CMD_WARN_NO_CMD);
                continue;
            }
            // Check if the command is "cd"
            if (strcmp(cmd.argv[0], "cd") == 0) {
                if (cmd.argc < 2) {
                    fprintf(stderr, "cd: missing argument\n");
                } else {
                    if (chdir(cmd.argv[1]) != 0) {
                        perror("cd");
                    }
                }
                continue;
            }
            pid_t pid = fork(); //  create new process by duplicating the current one
            if (pid == 0) {
                // Child process
                if (i > 0) { // check if we are not on the first command
                    // Redirect input from the previous pipe
                    if (dup2(pipe_fds[2 * (i - 1)], STDIN_FILENO) < 0) { // duping the read file descriptor of the preivous pipe to standard input for current command
                        perror("dup2"); 
                        exit(EXIT_FAILURE);
                    }
                }
                if (i < num_commands - 1) { // check if we are not at the last command
                    // Redirect output to the next pipe
                    if (dup2(pipe_fds[2 * i + 1], STDOUT_FILENO) < 0) { // duping the write file descriptor for next pipe to standard output of current command
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                }

                // closing all pipe file desciptors in child process since neccessary ones have been duped to standard input and output and so they cannot be picked up by other child processes
                for (int j = 0; j < 2 * (num_commands - 1); j++) {
                    close(pipe_fds[j]);
                }

                // execute the command with its arguments by replacing current process with the new process.
                if (execvp(cmd.argv[0], cmd.argv) == -1) {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            } else if (pid < 0) {
                // Fork failed
                perror("fork");
                exit(EXIT_FAILURE);
            }
        }

        // Close all pipe file descriptors in the parent process so we avoid any resource leaks
        for (int i = 0; i < 2 * (num_commands - 1); i++) {
            close(pipe_fds[i]);
        }

        // Wait for all child processes to finish
        for (int i = 0; i < num_commands; i++) {
            int status;
            wait(&status);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) { //check if child procces was successful
                printf("Child process exited with status %d\n", WEXITSTATUS(status));
            }
        }
    }

    return rc;
}
