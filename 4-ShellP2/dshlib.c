#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"

// Helper function to remove any extra spaces in a string
char* remove_spaces(char* in) {
	int i=0;
	char *parsed_command = malloc(sizeof(char) * strlen(in));
	size_t in_len = strlen(in);
	bool in_quotes = false;

	for (size_t j=0; j < in_len; j++) {
		if (in_quotes) {
			// If the quotation has been closed, set the bool back to false
			if (*(in + j) == '"') {
				in_quotes = false;
			}

			parsed_command[i] = *(in + j);
			i++;
		}

		else {
			// Skip over a leading space at the start of the string
			if (j == 0 && *(in + j) == ' ') {
				continue;
			}

			// Skip over any leading or duplicate spaces after the first
			if (*(in + j) == ' ' && *(in + j - 1) == ' ') {
				continue;
			}

			// Skip over a trailing space at the end of the string
			if (j + 1 == in_len && *(in + j) == ' ') {
				break;
			}

			// If we encounter a quotation mark, set the in_quotes bool to true
			if (*(in + j) == '"') {
				in_quotes = true;
			}

			parsed_command[i] = *(in + j);
			i++;
		}
	}

	// Make sure the return string is properly null terminated
	if (parsed_command[i-1] == ' ') parsed_command[i-1] = '\0';
	else parsed_command[i] = '\0';


	return parsed_command;
}

int alloc_cmd_buff(cmd_buff_t* cmd_buff) {
	// Initialize the memory within the data structure and make sure it is considered emtpy

	cmd_buff->argc = 0;

	for (int i=0; i < CMD_ARGV_MAX; i++) {
		cmd_buff->argv[i] = malloc(sizeof(char) * ARG_MAX);
		if (cmd_buff->argv[i] == NULL) return ERR_MEMORY;

		cmd_buff->argv[i][0] = '\0';
	}

	cmd_buff->_cmd_buffer = malloc(sizeof(char) * SH_CMD_MAX);
	if (cmd_buff->_cmd_buffer == NULL) return ERR_MEMORY;

	cmd_buff->_cmd_buffer[0] = '\0';

	return OK;
}

int free_cmd_buff(cmd_buff_t* cmd_buff) {
	// Free all of the memory within the cmd_buff that has been allocated on the heap
	
	for (int i=0; i < CMD_ARGV_MAX; i++) {
		free(cmd_buff->argv[i]);
	}

	free(cmd_buff->_cmd_buffer);

	return OK;

}

// set the attributes of the cmd_buff_t so that new data is ready to be written into it
int clear_cmd_buff(cmd_buff_t* cmd_buff) {
	cmd_buff->argc = 0;

	for (int i=0; i < CMD_ARGV_MAX; i++) {
		// If the argument was rewritten to null for the exec call, reallocate it
		if (cmd_buff->argv[i] == NULL) {
			cmd_buff->argv[i] = malloc(sizeof(char) * ARG_MAX);
			if (cmd_buff->argv[i] == NULL) return ERR_MEMORY;
		}

		// Set the first character of the argument to be the null terminator
		cmd_buff->argv[i][0] = '\0';
	}

	cmd_buff->_cmd_buffer[0] = '\0';

	return OK;
}

// Parse the command into the cmd_buff_t structure
// NOTE: cmd_line should be a string with all extra spaces removed
int build_cmd_buff(char* cmd_line, cmd_buff_t* cmd_buff) {
	bool in_quotes = false;
	int j = 0;

	// Make sure the cmd buffer is empty
	clear_cmd_buff(cmd_buff);

	// Parse the command line into the cmd_buff_t structure
	for (size_t i=0; i < strlen(cmd_line); i++) {

		if (in_quotes) {
			cmd_buff->argv[cmd_buff->argc][j] = cmd_line[i];
			j++;

			// If we reach another quotation mark, we are no longer in between quotes
			if (cmd_line[i] == '"'){
				in_quotes = false;
				cmd_buff->argv[cmd_buff->argc][j-1] = '\0';
			}
		}

		else {
			// When we have reached a space, we copy it in as a null terminator, then move to the next argument
			if (cmd_line[i] == ' ') {
				cmd_buff->argv[cmd_buff->argc][j] = '\0';
				cmd_buff->argc++;
				j = 0;
			}

			// Otherwise, copy the character into the current argument
			else {
				if (cmd_line[i] == '"') {
					in_quotes = true;
					cmd_buff->argv[cmd_buff->argc][j] = '\0';
				}
				else {
					cmd_buff->argv[cmd_buff->argc][j] = cmd_line[i];
					j++;
				}
			}
		}
	}

	// Do some tidying of the cmd_buff after the loop has finished
	cmd_buff->argv[cmd_buff->argc][j] = '\0';
	cmd_buff->argc++;

	// Make the last slot into a null pointer in preparation for the exec call
	free(cmd_buff->argv[cmd_buff->argc]); 
	cmd_buff->argv[cmd_buff->argc] = NULL; // We need this null pointer for the execv system call

	// Copy the line into the cmd_buffer
	strcpy(cmd_buff->_cmd_buffer, cmd_line);

	return OK;
}

int exec_cmd(cmd_buff_t* cmd) {
	int c_result;
	int f_result = fork();

	// Check that the child process was created with no issue
	if (f_result < 0) {
		return ERR_EXEC_CMD;
	}

	// Execute the command in the child process
	if (f_result == 0) {
		
		int rc = execvp(cmd->argv[0], cmd->argv);
		if (rc != 0) {
			exit(errno);
		}
	}

	else {
		// Wait on the child process and return its return value
		wait(&c_result);
		return WEXITSTATUS(c_result);
	}

	return OK;
}

void print_error(cmd_buff_t* cmd) {
	printf("Error: ");

	switch (errno) {
		case EACCES:
			printf("Execute permissions denied for file '%s'\n", cmd->argv[0]);
			break;

		case EFAULT:
			printf("Attempt to access restricted memory\n");
			break;

		case EIO:
			printf("I/O error\n");
			break;

		case ELIBBAD:
			printf("The ELF interpreter was not formatted properly\n");
			break;

		case ELOOP:
			printf("Maximum recursion depth reached in execution\n");
			break;

		case ENFILE:
			printf("Too many files are open\n");
			break;

		case ENAMETOOLONG:
			printf("File path '%s' is too long\n", cmd->argv[0]);
			break;

		case ENOENT:
			printf("File '%s' does not exist\n", cmd->argv[0]);
			break;

		case ENOEXEC:
			printf("Issue with executable formatting\n");
			break;

		case ENOMEM:
			printf("Not enough memory avalaible to execute command\n");
			break;

		case ENOTDIR:
			printf("Part of the directory path does not exist\n");
			break;

		case EPERM:
			printf("Permissions issue for file %s\n", cmd->argv[0]);
			break;

		case ETXTBSY:
			printf("File %s is in use by another process\n", cmd->argv[0]);
			break;

		case EAGAIN:
			printf("Kernel is at its process limit\n");
			break;

		default:
			printf("Error in executing command '%s'\n", cmd->argv[0]);
	}
}

Built_In_Cmds match_command(const char* input) {
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

	else {
		return BI_NOT_BI;
	}
}

// Currently this function is only used to execute the cd command
Built_In_Cmds exec_built_in_cmd(cmd_buff_t* cmd) {

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

	return BI_EXECUTED;
}


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
int exec_local_cmd_loop()
{
    char *cmd_buff;
    int rc = 0;
    cmd_buff_t* cmd = malloc(sizeof(cmd_buff_t));
	if (cmd == NULL) return ERR_MEMORY;

	int alloc_cmd_rc = alloc_cmd_buff(cmd);
	if (alloc_cmd_rc < 0) return alloc_cmd_rc;


	// Shell main loop
	while (1) {
		cmd_buff = malloc(sizeof(char) * SH_CMD_MAX);
		if (cmd_buff == NULL) return ERR_MEMORY;
		int struct_command; char* no_spaces;

		printf(SH_PROMPT);

		if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
			free(cmd_buff);
			return(OK);
		}
		
		// Replace the trailing newline character with a null terminator 
		cmd_buff[strcspn(cmd_buff,"\n")] = '\0';

		// Check if the command was blank
		if (cmd_buff[0] == '\0') {
			printf(CMD_WARN_NO_CMD);

			// Reset the command buffer
			free(cmd_buff);	cmd_buff = NULL;
			continue;
		}

		switch (match_command(cmd_buff)) {

			case BI_CMD_EXIT:
				free(cmd_buff);
				free_cmd_buff(cmd); free(cmd);

				return(OK);

				break;

			case BI_CMD_DRAGON:
				// Reset the command buffer and execute the command
				free(cmd_buff); cmd_buff = NULL;
				rc = print_dragon();

				errno = rc;

				if (rc < 0) {
					printf("Error %d occured in printing the dragon\n", rc);

					// Closest errno value to being relevant, needs to be set for the return code function
					errno = EIO;
				}

				break;

			case BI_CMD_CD:
				// Parse the command into the cmd_buffer struct
				no_spaces = remove_spaces(cmd_buff);
				struct_command = build_cmd_buff(no_spaces, cmd);

				Built_In_Cmds rc_cd = exec_built_in_cmd(cmd);

				errno = 0;

				if (rc_cd == BI_NOT_EXECUTED) {
					// FIXME What should happen when this function fails?
					printf("Error: too many arguments provided\n");

					// Closest errno value to being relevant, needs to be set for the return code function
					errno = E2BIG; 
				}

				free(cmd_buff); cmd_buff = NULL;

				break;

			case BI_RC:
				printf("%d\n", errno);
				errno = 0; // Must be reset, as the RC command has successfully run
				free(cmd_buff); cmd_buff= NULL;
				break;
			
			case BI_NOT_BI:
				no_spaces = remove_spaces(cmd_buff);

				// Parse the command into the cmd_buffer structure
				struct_command = build_cmd_buff(no_spaces, cmd);

				// Execute the command
				rc = exec_cmd(cmd);
				
				// Update errno so that the built-in rc command can access this return code
				errno = rc;

				// Command error
				if (rc != 0) {
					print_error(cmd);
				}

				//free_cmd_buff(cmd); free(cmd);
				free(cmd_buff); cmd_buff = NULL;
				free(no_spaces); no_spaces = NULL;
				break;

			default:
				free(cmd_buff); cmd_buff = NULL;
				continue;
			

		}
	}



    return OK;
}




