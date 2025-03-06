
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

#include "dshlib.h"
#include "rshlib.h"
/*
 * exec_remote_cmd_loop(server_ip, port)
 *      server_ip:  a string in ip address format, indicating the servers IP
 *                  address.  Note 127.0.0.1 is the default meaning the server
 *                  is running on the same machine as the client
 *              
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -c implemented in dsh_cli.c
 *              For example ./dsh -c 10.50.241.18:5678 where 5678 is the new port
 *              number and the server address is 10.50.241.18    
 * 
 *      This function basically implements the network version of 
 *      exec_local_cmd_loop() from the last assignemnt.  It will:
 *  
 *          1. Allocate buffers for sending and receiving data over the 
 *             network
 *          2. Create a network connection to the server, getting an active
 *             socket by calling the start_client(server_ip, port) function.
 *          2. Go into an infinite while(1) loop prompting the user for
 *             input commands. 
 * 
 *             a. Accept a command from the user via fgets()
 *             b. Send that command to the server using send() - it should
 *                be a null terminated string
 *             c. Go into a loop and receive client requests.  Note each
 *                receive might not be a C string so you need to print it
 *                out using:
 *                     printf("%.*s", (int)bytes_received, rsp_buff);
 *                this version of printf() uses the "%.*s" flag that indicates
 *                that the rsp_buff might be a null terminated string, or
 *                it might not be, if its not, print exactly bytes_received
 *                bytes. 
 *             d. In the recv() loop described above. Each time you receive
 *                data from the server, see if the last byte received is the
 *                EOF character. This indicates the server is done and you can
 *                send another command by going to the top of the loop.  The
 *                best way to do this is as follows assuming you are receiving
 *                data into a buffer called recv_buff, and you received
 *                recv_bytes in the call to recv:
 * 
 *                  recv_bytes = recv(sock, recv_buff, recv_buff_sz, 0)
 *                  
 *                if recv_bytes:
 *                  <negative_number>: communication error
 *                    0:    Didn't receive anything, likely server down
 *                  > 0:    Got some data. Check if the last byte is EOF
 *                          is_eof = (recv_buff[recv_bytes-1] == RDSH_EOF_CHAR) ? 1 : 0;
 *                    if is_eof is true, this is the last part of the transmission
 *                    from the server and you can break out of the recv() loop. 
 * 
 *   returns:
 *          OK:      The client executed all of its commands and is exiting
 *                   either by the `exit` command that terminates the client
 *                   or the `stop-server` command that terminates both the
 *                   client and the server. 
 *          ERR_MEMORY:             If this function cannot allocate memory via
 *                                  malloc for the send and receive buffers
 *          ERR_RDSH_CLIENT:        If the client cannot connect to the server. 
 *                                  AKA the call to start_client() fails.
 *          ERR_RDSH_COMMUNICATION: If there is a communication error, AKA
 *                                  any failures from send() or recv().
 * 
 *   NOTE:  Since there are several exit points and each exit point must
 *          call free() on the buffers allocated, close the socket, and
 *          return an appropriate error code.  Its suggested you use the
 *          helper function client_cleanup() for these purposes.  For example:
 * 
 *   return client_cleanup(cli_socket, request_buff, resp_buff, ERR_RDSH_COMMUNICATION);
 *   return client_cleanup(cli_socket, request_buff, resp_buff, OK);
 *
 *   The above will return ERR_RDSH_COMMUNICATION and OK respectively to the main()
 *   function after cleaning things up.  See the documentation for client_cleanup()
 *      
 */
int exec_remote_cmd_loop(char *address, int port){
    int client_socket;
    char *cmd_request_buff = NULL;
    char *cmd_response_buff = NULL;
    ssize_t bytes_sent;
    ssize_t bytes_received;
    int is_EOF;

    // allocate buffers for sending req and recieving responses
    cmd_request_buff = malloc(RDSH_COMM_BUFF_SZ);
    cmd_response_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (cmd_request_buff == NULL || cmd_response_buff == NULL){
        perror("malloc");
        return client_cleanup(client_socket, cmd_request_buff, cmd_response_buff, ERR_MEMORY);
    }

    // create a client connection to the server
    client_socket = start_client(address, port);
    if (client_socket < 0){
        return client_cleanup(client_socket, cmd_request_buff, cmd_response_buff, ERR_RDSH_CLIENT);
    }

    int flags = fcntl(client_socket, F_GETFL);
    if (flags == -1) {
        perror("fcntl failed");
        return -1;
    }

    flags |= O_NONBLOCK;  // Set the O_NONBLOCK flag
    // flags &= ~O_NONBLOCK; // this is blocking
    if (fcntl(client_socket, F_SETFL, flags) == -1) {
        perror("fcntl failed to set non-blocking mode");
        return -1;
    }

    // infinite loop grabbing user inputed commands
    while(1){
        memset(cmd_request_buff, 0, RDSH_COMM_BUFF_SZ);
        memset(cmd_response_buff, 0, RDSH_COMM_BUFF_SZ);
        printf("waiting for a command from user...\n");
        printf("%s", SH_PROMPT);

        if (fgets(cmd_request_buff, RDSH_COMM_BUFF_SZ, stdin)== NULL){
            perror("fgets");
            return client_cleanup(client_socket, cmd_request_buff, cmd_response_buff, ERR_RDSH_COMMUNICATION);
        }

        size_t len = strlen(cmd_request_buff);
        if (len > 0 && cmd_request_buff[len - 1] == '\n') {
            cmd_request_buff[len - 1] = RDSH_EOF_CHAR; // Replace newline character with EOF character
        } else {
            cmd_request_buff[len] = RDSH_EOF_CHAR; // Append EOF character
            len++;
        }

        // send command to the server
        printf("sending %s to server...\n", cmd_request_buff);
        bytes_sent = send(client_socket, cmd_request_buff, len, 0);
        if (bytes_sent <0){
            perror("Send");
            return client_cleanup(client_socket, cmd_request_buff, cmd_response_buff, ERR_RDSH_COMMUNICATION);
        }

        // create loop to collect response from server
        printf("waiting for response from server...\n");
        sleep(1);
        while((bytes_received = recv(client_socket, cmd_response_buff, RDSH_COMM_BUFF_SZ-1, 0))>0){
            // print out server response data
            // printf("%.*s\n", (int)bytes_received, cmd_response_buff);
            // printf("received %ld bytes from server\n", bytes_received);
            // check if the last byte we recieved is the EOF char 
            is_EOF = (cmd_response_buff[bytes_received -1] == RDSH_EOF_CHAR) ? 1 : 0;
            if (is_EOF){
                cmd_response_buff[bytes_received-1] = '\0'; // replace the EOF char with a null
                break;
            }

        }
        if (bytes_received < 0 ){
            perror("recv");
            return client_cleanup(client_socket, cmd_request_buff, cmd_response_buff, ERR_RDSH_COMMUNICATION);
        } else if (bytes_received == 0){
            printf("Server disconnected\n");
            return client_cleanup(client_socket, cmd_request_buff, cmd_response_buff, OK);
        }
        
        printf("response from server: %s\n", cmd_response_buff);
        // printf("length of response from server: %ld\n", strlen(cmd_response_buff));

        // check if the command request is to exit or stop-server
        if (strcmp(cmd_response_buff, "exit")== 0){
            return client_cleanup(client_socket, cmd_request_buff, cmd_response_buff, OK);
        } else if (strcmp(cmd_response_buff, "stop-server") == 0){
            return client_cleanup(client_socket, cmd_request_buff, cmd_response_buff, STOP_SERVER_SC);
        }
    }
    return client_cleanup(client_socket, cmd_request_buff, cmd_response_buff, OK);
}

/*
 * start_client(server_ip, port)
 *      server_ip:  a string in ip address format, indicating the servers IP
 *                  address.  Note 127.0.0.1 is the default meaning the server
 *                  is running on the same machine as the client
 *              
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -c implemented in dsh_cli.c
 *              For example ./dsh -c 10.50.241.18:5678 where 5678 is the new port
 *              number and the server address is 10.50.241.18    
 * 
 *      This function basically runs the client by: 
 *          1. Creating the client socket via socket()
 *          2. Calling connect()
 *          3. Returning the client socket after connecting to the server
 * 
 *   returns:
 *          client_socket:      The file descriptor fd of the client socket
 *          ERR_RDSH_CLIENT:    If socket() or connect() fail
 * 
 */
int start_client(char *server_ip, int port){
    int client_socket;
    struct sockaddr_in addr;

    // create the client socket
    printf("Creating client socket...\n");
    client_socket = socket(AF_INET, SOCK_STREAM,0);
    if (client_socket < 0){
        perror("client socket");
        return ERR_RDSH_CLIENT;
    }

    // set up the server address structure
    memset(&addr, 0, sizeof(addr)); // initialize struct to zeros
    addr.sin_family = AF_INET; 
    addr.sin_port = htons(port); // convert to network byte by htons

    if (inet_pton(AF_INET, server_ip, &addr.sin_addr)<= 0){ // convert the ip text to binary and store in the struct
        perror("inet_pton");
        close(client_socket);
        return ERR_RDSH_CLIENT;
    }
    // connect to the server using the address structure created above
    printf("Connecting to server at %s:%d...\n", server_ip, port);
    if (connect(client_socket, (struct sockaddr *)&addr, sizeof(addr))<0){
        perror("connecting to server");
        close(client_socket);
        return ERR_RDSH_CLIENT;
    }
    printf("Connected to server successfully.\n");

    return client_socket;
}

/*
 * client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc)
 *      cli_socket:   The client socket
 *      cmd_buff:     The buffer that will hold commands to send to server
 *      rsp_buff:     The buffer that will hld server responses
 * 
 *   This function does the following: 
 *      1. If cli_socket > 0 it calls close(cli_socket) to close the socket
 *      2. It calls free() on cmd_buff and rsp_buff
 *      3. It returns the value passed as rc
 *  
 *   Note this function is intended to be helper to manage exit conditions
 *   from the exec_remote_cmd_loop() function given there are several
 *   cleanup steps.  We provide it to you fully implemented as a helper.
 *   You do not have to use it if you want to develop an alternative
 *   strategy for cleaning things up in your exec_remote_cmd_loop()
 *   implementation. 
 * 
 *   returns:
 *          rc:   This function just returns the value passed as the 
 *                rc parameter back to the caller.  This way the caller
 *                can just write return client_cleanup(...)
 *      
 */
int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc){
    //If a valid socket number close it.
    if(cli_socket > 0){
        close(cli_socket);
    }

    //Free up the buffers 
    free(cmd_buff);
    free(rsp_buff);

    //Echo the return value that was passed as a parameter
    return rc;
}