#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "dshlib.h"

//acts as a library for the main function

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int build_cmd_list(char *cmd_line, command_list_t *clist){
    // clist is a pointer to a command_list_t structure that is empty and needs to be populated
    // cmd_line is a string that holds the command line from the user
    // initialize the command_list_t structure(clist is a pointer to struture, 0 is initial value to be set in each byte of memory block, sizeof(command_list_t) is the size of the memory block to be set to 0)
    memset(clist, 0, sizeof(command_list_t));

    // Split the line of user input into commands based on the pipe character
    char *command, *saveptr;
    command = strtok_r(cmd_line, "|", &saveptr);
    int command_count = 0;

     while (command != NULL) {
        bool no_arg = false;

        //remove leading spaces
        while (*command && isspace((unsigned char)*command)) {
        command++;
        }  
        //remove trailing spaces by getting end of string and moving backwards until a non-space character is found
        char *end = command + strlen(command) - 1;
        while (end > command && isspace((unsigned char)*end)) {
            end--;
        }
        // Null terminate the string
        *(end +1) = '\0';

        //split the command into executable and arguments peices
        char *command_peice, *arg_saveptr;
        command_peice = strtok_r(command, " ", &arg_saveptr);
        

        if (command_peice == NULL) {// check if the command is empty
            printf("No commands in request\n");
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }
        // write the executable into the command list
        strncpy(clist->commands[command_count].exe, command_peice, EXE_MAX);
        if (strlen(command_peice) >= EXE_MAX) {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }
        // iteract to first argument
        command_peice = strtok_r(NULL, " ", &arg_saveptr);
        if (command_peice == NULL) {
            no_arg = true;
        }
    

        // Concatenate the arguments into a single string surrounded by brackets
        if (!no_arg) {
        char args_combined[ARG_MAX] = "["; // Start with an opening bracket
        int arg_len = 1; // Start with 1 for the opening bracket
        while (command_peice != NULL) {
            int peice_len = strlen(command_peice);
            if (arg_len + peice_len + 2 >= ARG_MAX) { // this plus 2 is for the brackets involved
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            if (arg_len > 1) { // Add a space before the next argument if it's not the first one
                strcat(args_combined, " ");
                arg_len++;
            }
            strcat(args_combined, command_peice);
            arg_len += peice_len;
            command_peice = strtok_r(NULL, " ", &arg_saveptr);
        }
        strcat(args_combined, "]");
        arg_len++;
        strncpy(clist->commands[command_count].args, args_combined, arg_len);
        }

        command_count++;
        command = strtok_r(NULL, "|", &saveptr);
    }
    
    clist->num = command_count;
    return OK;
}

void print_dragon() {
    // printf(
    //     "                                                                        @%%%%                       \n"
    //     "                                                                     %%%%%%                         \n"
    //     "                                                                    %%%%%%                          \n"
    //     "                                                                 % %%%%%%%           @              \n"
    //     "                                                                %%%%%%%%%%        %%%%%%%           \n"
    //     "                                       %%%%%%%  %%%%@         %%%%%%%%%%%%@    %%%%%%  @%%%%        \n"
    //     "                                  %%%%%%%%%%%%%%%%%%%%%%      %%%%%%%%%%%%%%%%%%%%%%%%%%%%          \n"
    //     "                                %%%%%%%%%%%%%%%%%%%%%%%%%%   %%%%%%%%%%%% %%%%%%%%%%%%%%%           \n"
    //     "                               %%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%     %%%            \n"
    //     "                             %%%%%%%%%%%%%%%%%%%%%%%%%%%%@ @%%%%%%%%%%%%%%%%%%        %%            \n"
    //     "                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%%%%                \n"
    //     "                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%              \n"
    //     "                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@%%%%%%@              \n"
    //     "      %%%%%%%%@           %%%%%%%%%%%%%%%%        %%%%%%%%%%%%%%%%%%%%%%%%%%      %%                \n"
    //     "    %%%%%%%%%%%%%         %%@%%%%%%%%%%%%           %%%%%%%%%%% %%%%%%%%%%%%      @%                \n"
    //     "  %%%%%%%%%%   %%%        %%%%%%%%%%%%%%            %%%%%%%%%%%%%%%%%%%%%%%%                        \n"
    //     " %%%%%%%%%       %         %%%%%%%%%%%%%             %%%%%%%%%%%%@%%%%%%%%%%%                       \n"
    //     "%%%%%%%%%@                % %%%%%%%%%%%%%            @%%%%%%%%%%%%%%%%%%%%%%%%%                     \n"
    //     "%%%%%%%%@                 %%@%%%%%%%%%%%%            @%%%%%%%%%%%%%%%%%%%%%%%%%%%%                  \n"
    //     "%%%%%%%@                   %%%%%%%%%%%%%%%           %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%              \n"
    //     "%%%%%%%%%%                  %%%%%%%%%%%%%%%          %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%      %%%%  \n"
    //     "%%%%%%%%%@                   @%%%%%%%%%%%%%%         %%%%%%%%%%%%@ %%%% %%%%%%%%%%%%%%%%%   %%%%%%%%\n"
    //     "%%%%%%%%%%                  %%%%%%%%%%%%%%%%%        %%%%%%%%%%%%%      %%%%%%%%%%%%%%%%%% %%%%%%%%%\n"
    //     "%%%%%%%%%@%%@                %%%%%%%%%%%%%%%%@       %%%%%%%%%%%%%%     %%%%%%%%%%%%%%%%%%%%%%%%  %%\n"
    //     " %%%%%%%%%%                  % %%%%%%%%%%%%%%@        %%%%%%%%%%%%%%   %%%%%%%%%%%%%%%%%%%%%%%%%% %%\n"
    //     "  %%%%%%%%%%%%  @           %%%%%%%%%%%%%%%%%%        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  %%% \n"
    //     "   %%%%%%%%%%%%% %%  %  %@ %%%%%%%%%%%%%%%%%%          %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    %%% \n"
    //     "    %%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%%%%           @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    %%%%%%% \n"
    //     "     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%              %%%%%%%%%%%%%%%%%%%%%%%%%%%%        %%%   \n"
    //     "      @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                  %%%%%%%%%%%%%%%%%%%%%%%%%               \n"
    //     "        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                      %%%%%%%%%%%%%%%%%%%  %%%%%%%          \n"
    //     "           %%%%%%%%%%%%%%%%%%%%%%%%%%                           %%%%%%%%%%%%%%%  @%%%%%%%%%         \n"
    //     "              %%%%%%%%%%%%%%%%%%%%           @%@%                  @%%%%%%%%%%%%%%%%%%   %%%        \n"
    //     "                  %%%%%%%%%%%%%%%        %%%%%%%%%%                    %%%%%%%%%%%%%%%    %         \n"
    //     "                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                      %%%%%%%%%%%%%%            \n"
    //     "                %%%%%%%%%%%%%%%%%%%%%%%%%%  %%%% %%%                      %%%%%%%%%%  %%%@          \n"
    //     "                     %%%%%%%%%%%%%%%%%%% %%%%%% %%                          %%%%%%%%%%%%%@          \n"
    //     "                                                                                 %%%%%%%@       \n"
    // );
}

// takes in one line of input from the user into a buffer and then parses the buffer into a command list format and printing the commands
// and their arguments.
// if the string has a pipe it holds multiple commands and if it doesn't it holds one command
// make sure to use the test script. -> permissions denied.