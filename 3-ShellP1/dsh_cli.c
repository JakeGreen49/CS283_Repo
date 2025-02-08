#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dshlib.h"

int print_dragon() {
	int line_index = 0;
	int offset = 0;
	char* line = DRAGON[line_index];
	int replacements;

	char c; 
	int i;

	// Iterate over each line in the DRAGON array
	while (line_index < NUM_DRAGON_LINES) {
		// For each line, parse out and print the required number of characters
		while ((replacements = sscanf(line + offset, "%d%c", &i, &c)) > 0) {
			// Error in scanning in part of the line
			/*if (replacements < 0) {
				printf("%d\t", replacements);
				return -69;
			}*/

			// Print out the specified number of characters
			for (int j=0; j < i; j++) {
	
				if (c == 's') printf(" ");
				else if (c == 'a') printf("@");
				else if (c == 'p') fputc('%', stdout);

			}

			// Increment the offset for each digit scanned in
			for (int k=i; k != 0; k /= 10) {
				offset++;
			}
			offset++; // Increment the offset once more for the character scanned in
		}

		// Advance the line and reset the offset
		line_index++;
		line = DRAGON[line_index];
		offset = 0;

		// Print the newline character in anticipation of the next line
		printf("\n");
		
	}

	return OK;
}

/*
 * Implement your main function by building a loop that prompts the
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.  Since we want fgets to also handle
 * end of file so we can run this headless for testing we need to check
 * the return code of fgets.  I have provided an example below of how
 * to do this assuming you are storing user input inside of the cmd_buff
 * variable.
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
 *
 *   Expected output:
 *
 *      CMD_OK_HEADER      if the command parses properly. You will
 *                         follow this by the command details
 *
 *      CMD_WARN_NO_CMD    if the user entered a blank command
 *      CMD_ERR_PIPE_LIMIT if the user entered too many commands using
 *                         the pipe feature, e.g., cmd1 | cmd2 | ... |
 *
 *  See the provided test cases for output expectations.
 */

int main()
{
    char *cmd_buff;
    int rc = 0;

	// Shell main loop
	while (1) {
		cmd_buff = malloc(sizeof(char) * SH_CMD_MAX);
		printf(SH_PROMPT);

		if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
			//printf("command too long\n");
			free(cmd_buff);
			exit(ERR_CMD_OR_ARGS_TOO_BIG);
		}
		
		// Replace the trailing newline character with a null terminator for easier handling
		cmd_buff[strcspn(cmd_buff,"\n")] = '\0';

		// Check if the command was blank
		if (cmd_buff[0] == '\0') {
			printf(CMD_WARN_NO_CMD);

			// Reset the command buffer
			free(cmd_buff);	cmd_buff = NULL;
			continue;
		}

		// If the command was "exit", exit the program with a status of 0
		if (strcmp(cmd_buff, EXIT_CMD) == 0) {
			free(cmd_buff);
			exit(OK);
		}

		if (strcmp(cmd_buff, DRAGON_CMD) == 0) {
			// Reset the command buffer and execute the command
			free(cmd_buff); cmd_buff = NULL;
			rc = print_dragon();

			if (rc < 0) {
				printf("Error %d occured in dragon function\n", rc);
			}
			continue;
		}

		command_list_t* cmd_list = malloc(sizeof(command_list_t));	

		// Parse the command string given into a command list struct
		rc = build_cmd_list(cmd_buff, cmd_list);

		// The user exceeded the pipe limit
		if (rc == ERR_TOO_MANY_COMMANDS) {
			printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
			continue;
		}
		// If there was a different error in building the command list, exit with the error code
		else if (rc < 0) {
			//printf("error in building the command list");
			free(cmd_buff);
			free(cmd_list);
			exit(rc);
		}

		// Output the parsed commands
		printf(CMD_OK_HEADER, cmd_list->num);
		for (int i=0; i < cmd_list->num; i++) {
			// The command contained no arguments
			if (cmd_list->commands[i]->args[0] == '\0') {
				printf(CMD_OK_NO_ARGS, i + 1, cmd_list->commands[i]->exe);
			}
			// The command does have arguments
			else {
				printf(CMD_OK_ARGS, i + 1, cmd_list->commands[i]->exe, cmd_list->commands[i]->args);
			}
		}

		// Free all the allocated memory associated with the command list
		for (int i=0; i < cmd_list->num; i++) {
			free(cmd_list->commands[i]);
		}

		free(cmd_list);
		free(cmd_buff); cmd_buff = NULL;

	}




    printf(M_NOT_IMPL);
    exit(EXIT_NOT_IMPL);
}
