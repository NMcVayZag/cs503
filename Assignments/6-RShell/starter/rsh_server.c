
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

//INCLUDES for extra credit
//#include <signal.h>
//#include <pthread.h>
//-------------------------

#include "dshlib.h"
#include "rshlib.h"
#include <ctype.h>


/*
 * start_server(ifaces, port, is_threaded)
 *      ifaces:  a string in ip address format, indicating the interface
 *              where the server will bind.  In almost all cases it will
 *              be the default "0.0.0.0" which binds to all interfaces.
 *              note the constant RDSH_DEF_SVR_INTFACE in rshlib.h
 * 
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -s implemented in dsh_cli.c
 *              For example ./dsh -s 0.0.0.0:5678 where 5678 is the new port  
 * 
 *      is_threded:  Used for extra credit to indicate the server should implement
 *                   per thread connections for clients  
 * 
 *      This function basically runs the server by: 
 *          1. Booting up the server
 *          2. Processing client requests until the client requests the
 *             server to stop by running the `stop-server` command
 *          3. Stopping the server. 
 * 
 *      This function is fully implemented for you and should not require
 *      any changes for basic functionality.  
 * 
 *      IF YOU IMPLEMENT THE MULTI-THREADED SERVER FOR EXTRA CREDIT YOU NEED
 *      TO DO SOMETHING WITH THE is_threaded ARGUMENT HOWEVER.  
 */
int start_server(char *ifaces, int port, int is_threaded){
    int server_socket;
    int rc;
    

    //
    //TODO:  If you are implementing the extra credit, please add logic
    //       to keep track of is_threaded to handle this feature
    //

    server_socket = boot_server(ifaces, port);
    if (server_socket < 0){
        int err_code = server_socket;  //server socket will carry error code
        return err_code;
    }
    printf("Server started on %s:%d\n", ifaces, port);
    rc = process_cli_requests(server_socket);
    // establish a main loop to process client requests

    return rc;
}

/*
 * stop_server(svr_socket)
 *      svr_socket: The socket that was created in the boot_server()
 *                  function. 
 * 
 *      This function simply returns the value of close() when closing
 *      the socket.  
 */
int stop_server(int svr_socket){
    int rc = close(svr_socket);;
    if (rc<0){
        perror("closing server socket");
    }
    return rc;
}

/*
 * boot_server(ifaces, port)
 *      ifaces & port:  see start_server for description.  They are passed
 *                      as is to this function.   
 * 
 *      This function "boots" the rsh server.  It is responsible for all
 *      socket operations prior to accepting client connections.  Specifically: 
 * 
 *      1. Create the server socket using the socket() function. 
 *      2. Calling bind to "bind" the server to the interface and port
 *      3. Calling listen to get the server ready to listen for connections.
 * 
 *      after creating the socket and prior to calling bind you might want to 
 *      include the following code:
 * 
 *      int enable=1;
 *      setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
 * 
 *      when doing development you often run into issues where you hold onto
 *      the port and then need to wait for linux to detect this issue and free
 *      the port up.  The code above tells linux to force allowing this process
 *      to use the specified port making your life a lot easier.
 * 
 *  Returns:
 * 
 *      server_socket:  Sockets are just file descriptors, if this function is
 *                      successful, it returns the server socket descriptor, 
 *                      which is just an integer.
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code is returned if the socket(),
 *                               bind(), or listen() call fails. 
 * 
 */
int boot_server(char *ifaces, int port){
    int server_socket;
    struct sockaddr_in server_address;
    int enable = 1;

    // create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0){
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
        perror("setsockopt");
        close(server_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    memset(&server_address, 0, sizeof(server_address)); // initialize the sever address structure with zeros
    server_address.sin_family = AF_INET; //setting address family to AF_INET
    server_address.sin_port = htons(port); // converting int port to network byte and setting it to server_address

    if (inet_pton(AF_INET, ifaces, &server_address.sin_addr) <0){// converting text address to binary form and setting it to the server_address structure
        perror("inet_pton address setting");
        close(server_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    printf("Binding to %s:%d\n", ifaces, port);
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address))<0){// bind server socket to selected interface and port
        perror("bind");
        close(server_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    printf("Listening for incoming connections...\n");
    if (listen(server_socket, 20)< 0){ // listens for incoming connections max backlog of 20
        perror("listen");
        close(server_socket);
        return ERR_RDSH_COMMUNICATION;
    }


    return server_socket;
}

/*
 * process_cli_requests(svr_socket)
 *      svr_socket:  The server socket that was obtained from boot_server()
 *   
 *  This function handles managing client connections.  It does this using
 *  the following logic
 * 
 *      1.  Starts a while(1) loop:
 *  
 *          a. Calls accept() to wait for a client connection. Recall that 
 *             the accept() function returns another socket specifically
 *             bound to a client connection. 
 *          b. Calls exec_client_requests() to handle executing commands
 *             sent by the client. It will use the socket returned from
 *             accept().
 *          c. Loops back to the top (step 2) to accept connecting another
 *             client.  
 * 
 *          note that the exec_client_requests() return code should be
 *          negative if the client requested the server to stop by sending
 *          the `stop-server` command.  If this is the case step 2b breaks
 *          out of the while(1) loop. 
 * 
 *      2.  After we exit the loop, we need to cleanup.  Dont forget to 
 *          free the buffer you allocated in step #1.  Then call stop_server()
 *          to close the server socket. 
 * 
 *  Returns:
 * 
 *      OK_EXIT:  When the client sends the `stop-server` command this function
 *                should return OK_EXIT. 
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code terminates the loop and is
 *                returned from this function in the case of the accept() 
 *                function failing. 
 * 
 *      OTHERS:   See exec_client_requests() for return codes.  Note that positive
 *                values will keep the loop running to accept additional client
 *                connections, and negative values terminate the server. 
 * 
 */
int process_cli_requests(int svr_socket){
    int rc;

        while (1) {
        int cli_socket;
        struct sockaddr_in client_address;
        socklen_t client_address_length = sizeof(client_address);

        //accept incoming client connection
        printf("Waiting for client connection...\n");
        cli_socket = accept(svr_socket, (struct sockaddr *)&client_address, &client_address_length);
        if (cli_socket <0){
            perror("accepting client connection");
            rc = ERR_RDSH_COMMUNICATION;
            break;
        }
        printf("Accepted client connection\n");

        rc = exec_client_requests(cli_socket);
        printf("exec_client_requests returned %d\n", rc);

        if (rc == STOP_SERVER_SC){
            rc = OK_EXIT;
            break;
        } else if (rc<0){
            printf("Client connection terminated with error code from exec_client_req %d\n", rc);
            break;
        } else if (rc == OK){
            printf("Client connection terminated normally\n");
        }
    }

    stop_server(svr_socket);
    printf("Server stopped\n");

    return rc;
}


/*
 * exec_client_requests(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client
 *   
 *  This function handles accepting remote client commands. The function will
 *  loop and continue to accept and execute client commands.  There are 2 ways
 *  that this ongoing loop accepting client commands ends:
 * 
 *      1.  When the client executes the `exit` command, this function returns
 *          to process_cli_requests() so that we can accept another client
 *          connection. 
 *      2.  When the client executes the `stop-server` command this function
 *          returns to process_cli_requests() with a return code of OK_EXIT
 *          indicating that the server should stop. 
 * 
 *  Note that this function largely follows the implementation of the
 *  exec_local_cmd_loop() function that you implemented in the last 
 *  shell program deliverable. The main difference is that the command will
 *  arrive over the recv() socket call rather than reading a string from the
 *  keyboard. 
 * 
 *  This function also must send the EOF character after a command is
 *  successfully executed to let the client know that the output from the
 *  command it sent is finished.  Use the send_message_eof() to accomplish 
 *  this. 
 * 
 *  Of final note, this function must allocate a buffer for storage to 
 *  store the data received by the client. For example:
 *     io_buff = malloc(RDSH_COMM_BUFF_SZ);
 *  And since it is allocating storage, it must also properly clean it up
 *  prior to exiting.
 * 
 *  Returns:
 * 
 *      OK:       The client sent the `exit` command.  Get ready to connect
 *                another client. 
 *      OK_EXIT:  The client sent `stop-server` command to terminate the server
 * 
 *      ERR_RDSH_COMMUNICATION:  A catch all for any socket() related send
 *                or receive errors. 
 */
int exec_client_requests(int cli_socket) {
    int rc = OK;
    int bytes_received;
    int is_last_chunk;
    char eof_char = RDSH_EOF_CHAR;
    char *io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (io_buff == NULL){
        perror("malloc");
        return ERR_RDSH_COMMUNICATION;
        }
    printf("trying to receive message from client...\n\n");
    while(1){

            // while loop for collecting whole message from client
        memset(io_buff, 0, RDSH_COMM_BUFF_SZ);
        while((bytes_received = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ - 1, 0)) > 0){
            if (bytes_received < 0){
                perror("recv");
                rc = ERR_RDSH_COMMUNICATION;
                break;
            }
            is_last_chunk = ((char)io_buff[bytes_received-1] == eof_char) ? 1 : 0; // check if the last byte is the EOF character
            if (is_last_chunk){
                io_buff[bytes_received-1] = '\0'; //remove the marker and replace with a null
                                                  //this makes string processing easier
                }

            if (is_last_chunk){
                break;
            }
        }


        if (strcmp(io_buff, "exit") == 0){
            send_message_string(cli_socket, io_buff);
            rc = OK;
            break;
        } else if (strcmp(io_buff, "stop-server") == 0){
            send_message_string(cli_socket, io_buff);
            rc = STOP_SERVER_SC;
            break;
        } else if (bytes_received > 0 ) { // if command is not exit and a message was received then execute command
            printf("\n\nbuffer received from client: %s\n", io_buff);
            rc = rsh_execute_commands(cli_socket, io_buff);
            printf("main execution returned: %d\n",rc);
            if (rc != OK) {
                printf("invalid command");
            }
            if(rc == OK){
                send_message_eof(cli_socket);
            }
            
        }
        
    }
    free(io_buff);
    
    return rc;
}

/*
 * send_message_eof(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client

 *  Sends the EOF character to the client to indicate that the server is
 *  finished executing the command that it sent. 
 * 
 *  Returns:
 * 
 *      OK:  The EOF character was sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the EOF character. 
 */
int send_message_eof(int cli_socket){
    char eof_char = RDSH_EOF_CHAR;
    ssize_t bytes_sent = send(cli_socket, &eof_char, 1, 0);
    if(bytes_sent <0) {
        perror("sending EOF");
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}

/*
 * send_message_string(cli_socket, char *buff)
 *      cli_socket:  The server-side socket that is connected to the client
 *      buff:        A C string (aka null terminated) of a message we want
 *                   to send to the client. 
 *   
 *  Sends a message to the client.  Note this command executes both a send()
 *  to send the message and a send_message_eof() to send the EOF character to
 *  the client to indicate command execution terminated. 
 * 
 *  Returns:
 * 
 *      OK:  The message in buff followed by the EOF character was 
 *           sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the message followed by the EOF character. 
 */
int send_message_string(int cli_socket, char *buff){
    size_t len = strlen(buff);
    if (len > 0 && buff[len - 1] == '\n') {
        buff[len - 1] = RDSH_EOF_CHAR; // Replace newline character with EOF character
    } else {
        buff[len] = RDSH_EOF_CHAR; // Append EOF character
        len++;
    }
    ssize_t bytes_sent = send(cli_socket, buff, len, 0);
    if (bytes_sent<0){
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}
int server_parse_input(char *input, cmd_buff_t *cmd) {
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
int rsh_execute_commands(int cli_sock, char *buff) {
    char cmd_buff[ARG_MAX];
    int rc = 0;
    
    strcpy(cmd_buff, buff); // copy the buffer to a new buffer to avoid modifying the original buffer
    printf("Executing command: %s\n", cmd_buff);
        while(1){
        //remove the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';

        // seporating the input via pipes using strtok
        char *commands[CMD_MAX];
        int num_commands = 0;
        char *command = strtok(cmd_buff, PIPE_STRING);
        while (command != NULL && num_commands < CMD_MAX) { // while we have a command and we haven't reached max commands
            commands[num_commands++] = command; // add the command to commands array
            command = strtok(NULL, PIPE_STRING); // select the next command
        }
        // Check if the number of commands exceeds the limit
        if (num_commands >= CMD_MAX) {
            printf("Error: too many commands\n");
            rc = 1;
            char toomany[40];
            strcpy(toomany, "Invalid request: Too many commands");
            send_message_string(cli_sock, toomany);
            return rc;
            
        }

        // establish if we need pipes for command 
        int pipes_neccessary = 2 *(num_commands-1);

        int pids[num_commands];
        int pipe_fds[num_commands - 1][2];

        
        for (int i = 0; i < num_commands - 1; i++) { // creating each pipe with a read and write file descriptor
            if (pipe(pipe_fds[i]) < 0) { // runs pipe function to store the read and write file descriptors in correct slots in array based on i
                perror("pipe"); // prints error on failure of piping
                exit(EXIT_FAILURE);
            }
            //printf("Pipe %d created: read_fd=%d, write_fd=%d\n", i, pipe_fds[i][0], pipe_fds[i][1]); // Debugging statement
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
            if (server_parse_input(commands[i], &cmd) == WARN_NO_CMDS) {
                printf("%s\n", CMD_WARN_NO_CMD);
                continue;
            }

            // // Print the command and its arguments
            // printf("Command: %s\n", cmd.argv[0]);
            // printf("Arguments: ");
            // for (int j = 0; j < cmd.argc; j++) {
            //     printf("%s ", cmd.argv[j]);
            // }
            // printf("\n");

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
            //printf("pid: %d\n", pid);
            if (pid == 0) {
                // Child process

                pids[i] = getpid(); // add pid to list 

                if (pipes_neccessary > 0){

                    if (i > 0) { // check if we are not on the first command
                        // Redirect input from the previous pipe
                        //printf("Child process %d: redirecting stdin to pipe %d read_fd=%d\n", getpid(), i - 1, pipe_fds[i - 1][0]); // Debugging statement
                        if (dup2(pipe_fds[i - 1][0], STDIN_FILENO) < 0) { // duping the read file descriptor of the previous pipe to standard input for current command
                            perror("dup2 stdin");
                            exit(EXIT_FAILURE);
                        }
                    }
                    if (i < num_commands - 1) { // check if we are not at the last command
                        // Redirect output to the next pipe
                        //printf("Child process %d: redirecting stdout to pipe %d write_fd=%d\n", getpid(), i, pipe_fds[i][1]); // Debugging statement
                        if (dup2(pipe_fds[i][1], STDOUT_FILENO) < 0) { // duping the write file descriptor for next pipe to standard output of current command
                            perror("dup2 stdout");
                            exit(EXIT_FAILURE);
                        }
                    }else {
                        // Redirect output to the client socket
                        //printf("Child process %d: redirecting stdout to client socket\n", getpid()); // Debugging statement
                        if (dup2(cli_sock, STDOUT_FILENO) < 0) {
                            perror("dup2 stdout to client socket");
                            exit(EXIT_FAILURE);
                        }
                        if (dup2(cli_sock, STDERR_FILENO) < 0) {
                            perror("dup2 stdERR to client socket");
                            exit(EXIT_FAILURE);
                        }
                        // fflush(stdout);
                    }

                    // Close all pipe file descriptors in child process since necessary ones have been duped to standard input and output and so they cannot be picked up by other child processes
                    for (int j = 0; j < num_commands - 1; j++) {
                        close(pipe_fds[j][0]);
                        close(pipe_fds[j][1]);
                        //printf("Child process %d: closed pipe %d read_fd=%d, write_fd=%d\n", getpid(), j, pipe_fds[j][0], pipe_fds[j][1]); // Debugging statement
                    }

                    // execute the command with its arguments by replacing current process with the new process.
                    if (execvp(cmd.argv[0], cmd.argv) == -1) {
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    }
            } 
            if (pipes_neccessary == 0) {
                if (dup2(cli_sock, STDOUT_FILENO)<0){
                    printf("Error redirecting single command output to client socket\n");
                    fflush(stdout);
                }

                // Execute the command with its arguments by replacing the current process with the new process
                if (execvp(cmd.argv[0], cmd.argv) == -1) {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }

                }
            } else if (pid < 0) {
                // Fork failed
                rc = 1;
                perror("fork");
                exit(EXIT_FAILURE);
            }
        }
    // Close all pipe file descriptors in the parent process so we avoid any resource leaks
    for (int i = 0; i < num_commands - 1; i++) {
        close(pipe_fds[i][0]);
        close(pipe_fds[i][1]);
        //printf("Parent process: closed pipe %d read_fd=%d, write_fd=%d\n", i, pipe_fds[i][0], pipe_fds[i][1]); // Debugging statement
    }
        

        // Wait for all child processes to finish
        for (int i = 0; i < num_commands; i++) {
            int status;
            wait(&status);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) { //check if child procces was successful
                printf("Child process with PID %d exited with status %d\n", pids[i], WEXITSTATUS(status));
                rc = 1;
                char buf[20];
                strcpy(buf, "invalid input");
                send_message_string(cli_sock, buf);
            }
        }
        break; 
    }
    
    return rc;
}










/**************   OPTIONAL STUFF  ***************/
/****
 **** NOTE THAT THE FUNCTIONS BELOW ALIGN TO HOW WE CRAFTED THE SOLUTION
 **** TO SEE IF A COMMAND WAS BUILT IN OR NOT.  YOU CAN USE A DIFFERENT
 **** STRATEGY IF YOU WANT.  IF YOU CHOOSE TO DO SO PLEASE REMOVE THESE
 **** FUNCTIONS AND THE PROTOTYPES FROM rshlib.h
 **** 
 */

/*
 * rsh_match_command(const char *input)
 *      cli_socket:  The string command for a built-in command, e.g., dragon,
 *                   cd, exit-server
 *   
 *  This optional function accepts a command string as input and returns
 *  one of the enumerated values from the BuiltInCmds enum as output. For
 *  example:
 * 
 *      Input             Output
 *      exit              BI_CMD_EXIT
 *      dragon            BI_CMD_DRAGON
 * 
 *  This function is entirely optional to implement if you want to handle
 *  processing built-in commands differently in your implementation. 
 * 
 *  Returns:
 * 
 *      BI_CMD_*:   If the command is built-in returns one of the enumeration
 *                  options, for example "cd" returns BI_CMD_CD
 * 
 *      BI_NOT_BI:  If the command is not "built-in" the BI_NOT_BI value is
 *                  returned. 
 */
Built_In_Cmds rsh_match_command(const char *input)
{
    return BI_NOT_IMPLEMENTED;
}

/*
 * rsh_built_in_cmd(cmd_buff_t *cmd)
 *      cmd:  The cmd_buff_t of the command, remember, this is the 
 *            parsed version fo the command
 *   
 *  This optional function accepts a parsed cmd and then checks to see if
 *  the cmd is built in or not.  It calls rsh_match_command to see if the 
 *  cmd is built in or not.  Note that rsh_match_command returns BI_NOT_BI
 *  if the command is not built in. If the command is built in this function
 *  uses a switch statement to handle execution if appropriate.   
 * 
 *  Again, using this function is entirely optional if you are using a different
 *  strategy to handle built-in commands.  
 * 
 *  Returns:
 * 
 *      BI_NOT_BI:   Indicates that the cmd provided as input is not built
 *                   in so it should be sent to your fork/exec logic
 *      BI_EXECUTED: Indicates that this function handled the direct execution
 *                   of the command and there is nothing else to do, consider
 *                   it executed.  For example the cmd of "cd" gets the value of
 *                   BI_CMD_CD from rsh_match_command().  It then makes the libc
 *                   call to chdir(cmd->argv[1]); and finally returns BI_EXECUTED
 *      BI_CMD_*     Indicates that a built-in command was matched and the caller
 *                   is responsible for executing it.  For example if this function
 *                   returns BI_CMD_STOP_SVR the caller of this function is
 *                   responsible for stopping the server.  If BI_CMD_EXIT is returned
 *                   the caller is responsible for closing the client connection.
 * 
 *   AGAIN - THIS IS TOTALLY OPTIONAL IF YOU HAVE OR WANT TO HANDLE BUILT-IN
 *   COMMANDS DIFFERENTLY. 
 */
Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd)
{
    return BI_NOT_IMPLEMENTED;
}
