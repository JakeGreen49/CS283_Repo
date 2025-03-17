
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>

//INCLUDES for extra credit
#include <signal.h>
#include <pthread.h>
//-------------------------

#include "dshlib.h"
#include "rshlib.h"

int told_to_stop_server = 0; // Used to keep track of whether any of the threaded connections send the stop-server command
static volatile sig_atomic_t sig_svr;

void* connection_handler(void* acpt_sock) {
	int sock = *((int *) acpt_sock);

	// Execute the client requests
	int rt = exec_client_requests(sock);

	// Once the client finishes, close its socket
	close(sock);

	// If the client finished by sending the stop-server command, we need to let the main function know to stop the server
	if (rt == OK_EXIT) {
		told_to_stop_server = 1;
	}

	return OK;

}

static void sig_handler(int sig_num) {
	if (sig_num == SIGINT) {
		write(STDOUT_FILENO, "Received interrupt, exiting gracefully...\n", 42);
		stop_server(sig_svr);
		exit(OK);
	}
}

int process_threaded_requests(int svr_socket) {
	int accpt_socket; int rt;
	sig_svr = svr_socket;

	// Make it so that the interrupt signal will make the server exit gracefully
	signal(SIGINT, sig_handler);


	while(1) {

		// If the stop-server command was received in a thread, break out of the connection loop
		if (told_to_stop_server) {
			break;
		}

		// Wait for a client connection
		accpt_socket = accept(svr_socket, NULL, NULL);

		if (accpt_socket < 0) {
			perror("Accept socket\n");
			return (ERR_RDSH_COMMUNICATION);
		}

		// Create a thread and handle the client requests inside that thread
		pthread_t thread_id;
		rt = pthread_create(&thread_id, NULL, connection_handler, (void *) &accpt_socket);

		if (rt != 0) {
			perror("Could not create thread\n");
			return (ERR_RDSH_SERVER);
		}


	}

	// Close the server
	stop_server(svr_socket);

    return OK_EXIT;	

}


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
    int svr_socket;
    int rc;

    //
    //TODO:  If you are implementing the extra credit, please add logic
    //       to keep track of is_threaded to handle this feature
    //


    svr_socket = boot_server(ifaces, port);
   	if (svr_socket < 0){
        int err_code = svr_socket;  //server socket will carry error code
   	    return err_code;
   	}

	if (is_threaded) {
		rc = process_threaded_requests(svr_socket);

		stop_server(svr_socket);
	}

	else {

	    rc = process_cli_requests(svr_socket);

    	stop_server(svr_socket);
	}

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
    return close(svr_socket);
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
	struct sockaddr_in addr;
	int rt;

	// Create the server socket
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (server_socket < 0) {
		perror("Server socket error\n");
		return (ERR_RDSH_COMMUNICATION);
	}

	// For development purposes
	int enable=1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	// Bind the server socket to the interface and port
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//addr.sin_addr.s_addr = htonl(ifaces);
	addr.sin_port = htons(port);

	rt = bind(server_socket, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in));

	if (rt < 0) {
		perror("Binding error\n");
		return (ERR_RDSH_COMMUNICATION);
	}

	// Get the server ready to listen for any connections
	rt = listen(server_socket, 20);

	if (rt < 0) {
		perror("Listen error\n");
		return (ERR_RDSH_COMMUNICATION);
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
	int accpt_socket; int rt;

	while(1) {
		// Wait for a client connection
		accpt_socket = accept(svr_socket, NULL, NULL);

		if (accpt_socket < 0) {
			perror("Accept socket\n");
			return (ERR_RDSH_COMMUNICATION);
		}
		
		// Execute all of the requests coming in from the client
		rt = exec_client_requests(accpt_socket);

		// When the client is finished, close its socket
		close(accpt_socket);

		// If the stop-server command was received, break out of the connection loop
		if (rt == OK_EXIT) {
			break;
		}

	}

	// Close the server
	stop_server(svr_socket);

    return OK_EXIT;
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
	int rt; int out_len;
	char* recv_buff = malloc(sizeof(char) * RDSH_COMM_BUFF_SZ);
	char* output_buff = malloc(sizeof(char) * RDSH_COMM_BUFF_SZ);
	char* no_spaces;

		
	while (1) {
		memset(recv_buff, 0, RDSH_COMM_BUFF_SZ);
		memset(output_buff, 0, RDSH_COMM_BUFF_SZ);

		rt = recv(cli_socket, recv_buff, RDSH_COMM_BUFF_SZ, 0);

		// This might need to be FIXME
		if (rt < 0) {
			perror("Receive error\n");
			return (ERR_RDSH_COMMUNICATION);
		}

		// ===== Process the command =====

		// Check if the command sent was blank; if so, send back the warning about blank commands
		if (recv_buff[0] == '\0') {
			out_len = sprintf(output_buff, "%s", CMD_WARN_NO_CMD);
			send_message_string(cli_socket, output_buff);
			continue;
		}

		// Remove any extra spaces from the string received
		no_spaces = remove_spaces(recv_buff);

		// Create the command list
		command_list_t* cmd_list = malloc(sizeof(command_list_t));
		rt = build_cmd_list(no_spaces, cmd_list);

		// Check that the maximum number of piped commands has not been exceeded
		if (rt == ERR_TOO_MANY_COMMANDS) {
			// Send back the error
			out_len = sprintf(output_buff, CMD_ERR_PIPE_LIMIT, CMD_MAX);
			send_message_string(cli_socket, output_buff);
			errno = CMD_MAX;

			// Free the allocated memory of the cmd list
			free_cmd_list(cmd_list);
			continue;
		}

		// Determine if the command is built-in or not, then execute it accordingly
		switch (rsh_match_command((const char *) no_spaces)) {
			case BI_CMD_EXIT:
				// Free allocated memory then exit out of the function, telling the process_cli_req loop to keep running
				free(recv_buff); free(output_buff);
				free_cmd_list(cmd_list);
				return OK;
				break;

			case BI_CMD_STOP_SVR:
				// Free allocated memory then exit out of the function, telling the process_cli_req loop to stop the server
				free(recv_buff); free(output_buff);
				free_cmd_list(cmd_list);
				return OK_EXIT;
				break;

			case BI_CMD_DRAGON:
				// Execute the dragon command
				errno = 0;
				if (exec_dragon_cmd(cli_socket) == BI_NOT_EXECUTED) {
					out_len = sprintf(output_buff, "Error executing command 'dragon'\n");
					send_message_string(cli_socket, output_buff);
					errno = -1;
				}

				break;

			case BI_CMD_CD:
				// Execute the cd command
				if (exec_cd_cmd(cmd_list->commands[0]) == BI_NOT_EXECUTED) {
					out_len = sprintf(output_buff, "Error: too many arguments provided\n");
					send_message_string(cli_socket, output_buff);
					errno = E2BIG; // Closest errno value to being relevant for too many arguments
				}

				else {
					errno = send_message_eof(cli_socket);
				}

				break;
			
			case BI_RC:
				// Send the value of errno to the client, then set errno to the result of this command (0 if sending was a success)
				out_len = sprintf(output_buff, "%d\n", errno);
				errno = send_message_string(cli_socket, output_buff);
				break;

			case BI_NOT_BI:
				//printf("Client sent: '%s'\n", recv_buff);

				// Execute the pipeline
				int* rc_list = rsh_execute_pipeline(cli_socket, cmd_list);

				// Set the errno according to the result (it will be based on the last command to execute)
				for (int i=0; i < cmd_list->num; i++) {
					errno = rc_list[i];
				}

				// Free the return code list that was allocated on the heap
				free(rc_list); rc_list = NULL;

				break;

			default:
				break;

		}

		
	}

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
	char output[] = { RDSH_EOF_CHAR };
	int rc = send(cli_socket, output, 1, 0);

	if (rc == -1) {
		return ERR_RDSH_COMMUNICATION;
	}

	else {
		return OK;
	}
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
	int rc = send(cli_socket, buff, strlen(buff), 0);

	if (rc == -1) {
		return ERR_RDSH_COMMUNICATION;
	}

	rc = send_message_eof(cli_socket);
    return rc;
}


/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 *      cli_sock:    The server-side socket that is connected to the client
 *      clist:       The command_list_t structure that we implemented in
 *                   the last shell. 
 *   
 *  This function executes the command pipeline.  It should basically be a
 *  replica of the execute_pipeline() function from the last deliverable. 
 *  The only thing different is that you will be using the cli_sock as the
 *  main file descriptor on the first executable in the pipeline for STDIN,
 *  and the cli_sock for the file descriptor for STDOUT, and STDERR for the
 *  last executable in the pipeline.  See picture below:  
 * 
 *      
 *┌───────────┐                                                    ┌───────────┐
 *│ cli_sock  │                                                    │ cli_sock  │
 *└─────┬─────┘                                                    └────▲──▲───┘
 *      │   ┌──────────────┐     ┌──────────────┐     ┌──────────────┐  │  │    
 *      │   │   Process 1  │     │   Process 2  │     │   Process N  │  │  │    
 *      │   │              │     │              │     │              │  │  │    
 *      └───▶stdin   stdout├─┬──▶│stdin   stdout├─┬──▶│stdin   stdout├──┘  │    
 *          │              │ │   │              │ │   │              │     │    
 *          │        stderr├─┘   │        stderr├─┘   │        stderr├─────┘    
 *          └──────────────┘     └──────────────┘     └──────────────┘   
 *                                                      WEXITSTATUS()
 *                                                      of this last
 *                                                      process to get
 *                                                      the return code
 *                                                      for this function       
 * 
 *  Returns:
 * 
 *      EXIT_CODE:  This function returns the exit code of the last command
 *                  executed in the pipeline.  If only one command is executed
 *                  that value is returned.  Remember, use the WEXITSTATUS()
 *                  macro that we discussed during our fork/exec lecture to
 *                  get this value. 
 */
int* rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    // For storing the pipes and child pids
    int pipes[clist->num - 1][2];
    int fd, redirects = 0;
    pid_t pids[clist->num];

    // For handling any command failures
    int* c_result = malloc(sizeof(int) * clist->num);
    for (int i=0; i < clist->num; i++) {
        c_result[i] = 0; // Just to make sure every return code is assumed to be 0
    }

    errno = 0;

    // Create all of the pipes we will be using
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            // If the pipe creation failed, bail early and extract the issue later via errno
            c_result[i] = ERR_EXEC_CMD;
            return c_result;
        }
    }

    // Loop through and execute all of the processes in the command list
    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            exit(ERR_EXEC_CMD);
        }

        // The child process
        if (pids[i] == 0) {
            // Set up input section of the pipe (except for the first command which doesn't take an input)
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }

            // Set up output section of the pipe (except for the last command which should print its output to the terminal)
            if (i < clist->num - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

			// In the last command, write to the socket instead of stdout
			if (i == clist->num - 1) {
				dup2(cli_sock, STDOUT_FILENO);
				dup2(cli_sock, STDERR_FILENO);
			}

            // Close all pipes in the child child processes
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Check for any redirections before executing
            for (int j=0; j < clist->commands[i]->argc; j++) {
                // There is an input redirection
                if (strcmp(clist->commands[i]->argv[j], IN_STRING) == 0 && j + 1 < clist->commands[i]->argc) {
                    fd = open(clist->commands[i]->argv[j + 1], O_RDONLY);

                    // File read error, exit the process early
                    if (fd < 0) {
                        exit(errno);
                    }

                    dup2(fd, STDIN_FILENO);
                    close(fd);
                    redirects++;
                }

                // There is an output redirection
                if (strcmp(clist->commands[i]->argv[j], OUT_STRING) == 0 && j + 1 < clist->commands[i]->argc) {
                    // Reassign the stdout to the file
                    fd = open(clist->commands[i]->argv[j + 1], O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    redirects++;
                }

                // There is an append redirection
                if (strcmp(clist->commands[i]->argv[j], APP_STRING) == 0 && j + 1 < clist->commands[i]->argc) {
                    fd = open(clist->commands[i]->argv[j + 1], O_WRONLY|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    redirects++;
                }
            }

            // Remove any redirect characters and their corresponding files, then execute the command
            if (redirects > 0) {
                cmd_buff_t* new_cmd = malloc(sizeof(cmd_buff_t));
                alloc_cmd_buff(new_cmd);
                int new_i = 0;

                for (int j=0; j < clist->commands[i]->argc; j++) {
                    // Skip over a redirection character and its corresponding file
                    if (strcmp(clist->commands[i]->argv[j], IN_STRING) == 0 || strcmp(clist->commands[i]->argv[j], OUT_STRING) == 0 ||
                        strcmp(clist->commands[i]->argv[j], APP_STRING) == 0) {
                        j++; continue;
                    }

                    // Copy the command from the old cmd_buff to the new
                    new_cmd->argv[new_i] = clist->commands[i]->argv[j];
                    new_i++; new_cmd->argc++;
                }

                new_cmd->argv[new_cmd->argc] = 0;

                execvp(new_cmd->argv[0], new_cmd->argv);
                exit(errno);
            }

            // If there were no redirects, just execute the command
            else {
                // Execute the command
                int rc = execvp(clist->commands[i]->argv[0], clist->commands[i]->argv);

                // If the command fails, return the value that it set errno to
                if (rc != 0) {
                    exit(errno);
                }
            }
        }
    }

    // The parent process
    for (int i = 0; i < clist->num - 1; i++) {
        // Close all of the pipes
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all of the children to finish executing
    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &c_result[i], 0);
    }

	send_message_eof(cli_sock);

    return c_result;

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
    if (strcmp(input, EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    }

    else if (strcmp(input, DRAGON_CMD) == 0) {
        return BI_CMD_DRAGON;
    }

    else if (strncmp(input, CD_CMD, strlen(CD_CMD)) == 0) {
        return BI_CMD_CD;
    }

    else if (strcmp(input, RC_CMD) == 0) {
        return BI_RC;
    }

	else if (strcmp(input, STOP_SVR_CMD) == 0) {
		return BI_CMD_STOP_SVR;
	}

    else {
        return BI_NOT_BI;
    }	
}

Built_In_Cmds exec_cd_cmd(cmd_buff_t* cmd) {

    // If no arguments are given, the command does nothing
    if (cmd->argc == 1) {
        return BI_EXECUTED;
    }

    // If a single directory is given as an argument, change the working directory to that directory
    else if (cmd->argc == 2) {
        int rc = chdir(cmd->argv[1]);
        if (rc < 0) return BI_NOT_EXECUTED;

        return BI_EXECUTED;
    }

    else {
        return BI_NOT_EXECUTED;
    }
}

Built_In_Cmds exec_dragon_cmd(int sock) {
	int c_result, rc;
	int f_result = fork();

	// Execute the command inside of a child process
	if (f_result == 0) {
		// Remap the output to go into the socket
		rc = dup2(sock, STDOUT_FILENO);

		if (rc == -1) {
			exit(rc);
		}

		// Print the dragon
		print_dragon();
		rc = send_message_eof(sock);

		exit(rc);
	}

	else {
		// Wait on the child process to finish running
		wait(&c_result);

		if (WEXITSTATUS(c_result) == -1 || WEXITSTATUS(c_result) == ERR_RDSH_COMMUNICATION) {
			return BI_NOT_EXECUTED;
		}
		else {
			return BI_EXECUTED;
		}
	}
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
	// Get the build_in_cmd code of the command
	Built_In_Cmds bi_code = rsh_match_command(cmd->argv[0]);

	// handle the code accordingly
	switch(bi_code) {
		case BI_CMD_EXIT:
			return BI_CMD_EXIT;
			break;
		
		case BI_CMD_DRAGON:
			//print_dragon();
			return BI_CMD_DRAGON;
			break;

		case BI_CMD_CD:
			return exec_cd_cmd(cmd); // Executes the command in this function 
			break;

		case BI_RC:
			return BI_RC;
			break;

		case BI_CMD_STOP_SVR:
			return BI_CMD_STOP_SVR;
			break;

		default:
			return BI_NOT_BI;
	}
}



