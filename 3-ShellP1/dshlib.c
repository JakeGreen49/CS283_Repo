#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

// Helper function to parse a single command given as a string in cmd_buff into a command struct
int make_struct(char* cmd_buff, command_t* command) {
	size_t i = 0; size_t j = 0;
	size_t cmd_len = strlen(cmd_buff);

	// First parse the command
	while (cmd_buff[i] != ' ' && i < cmd_len) {
		// Check if the command exceeds its maximum length
		if (i > EXE_MAX) {
			//printf("While loop error\n");
			return(ERR_CMD_OR_ARGS_TOO_BIG);
		}

		command->exe[i] = cmd_buff[i]; 
		i++; 
	}
	command->exe[i] = '\0';

	// Check if the arguments exceed their maximum length
	if (cmd_len - i > ARG_MAX) {
		//printf("arg length check\n");
		return ERR_CMD_OR_ARGS_TOO_BIG;
	}

	// The command contained no arguments, so return early
	if (i == cmd_len) {
		command->args[0] = '\0';
		return OK;
	}

	// Then parse the arguments
	i++; // This skips over the space between the command and arg
	while (i < cmd_len) {
		command->args[j] = cmd_buff[i];
		i++; j++;
	}

	return OK;
}

// Helper function to remove any extra spaces in a string
char* remove_spaces(char* in) {
	int i=0;
	char *parsed_command = malloc(sizeof(char) * strlen(in));
	size_t in_len = strlen(in);

	for (size_t j=0; j < in_len; j++) {
		// Skip over a leading space at the start of the string
		if (j == 0 && *(in + j) == ' ') {
			continue;
		}

		// Skip over any leading or duplicate spaces after the first
		if (*(in + j) == ' ' && *(in + j - 1) == ' ') {
			continue;
		}
		else if (j + 1 != in_len && *(in + j) == ' ' && *(in + j + 1) == ' ') {
			continue;
		}

		// Skip over a trailing space at the end of the string
		if (j + 1 == in_len && *(in + j) == ' ') {
			break;
		}

		parsed_command[i] = *(in + j);
		i++;
	}

	// Make sure the return string is properly null terminated
	parsed_command[i] = '\0';

	return parsed_command;
}


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



int build_cmd_list(char *cmd_line, command_list_t *clist)
{
	//size_t line_len = strlen(cmd_line);
	clist->num = 0;

	char* token = strtok(cmd_line, PIPE_STRING);

	// Iterate over each command from the cmd_line
	while (token != NULL) {
		// The number of commands exceeds the maximum allowable number
		if (clist->num == CMD_MAX) {
			return ERR_TOO_MANY_COMMANDS;
		}

		//int i = 0;
		command_t* command_struct = malloc(sizeof(command_t));

		// First remove any extra spaces
		char* parsed_command = remove_spaces(token);
		//printf("parsed command = '%s'\n", parsed_command);

		// Convert the parsed command into a command struct
		int mk_struct = make_struct(parsed_command, command_struct);

		// Check that everything went well making the command struct
		if (mk_struct < 0) {
			//printf("Struct error\n");
			return mk_struct;
		}

		// Add the command struct to the commands list struct
		clist->commands[clist->num] = command_struct;
		(clist->num)++;

		// Free the parsed command which is allocated on the heap
		free(parsed_command);

		// Advance to the next token
		token = strtok(NULL, PIPE_STRING);
	}

    return OK;
}
