#include <sys/wait.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "supportFunctions.h"
#include "historyCommand.h"

/* Signal handler function */
void handle_SIGINT()
{
	write(STDOUT_FILENO, "\n", strlen("\n"));
	executePrintHistoryCommand();
}

int read_command(char *buff, char *tokens[], _Bool *in_background) {
	*in_background = false;

	// Read input
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

	if ( (length < 0) && (errno !=EINTR) ){
		perror( "Unable to read command 2. Terminating.\n ");
		exit(-1);
	}
	if ( (length < 0) && (errno == EINTR) ) {
		// read interrupted by signal. Report failure of read_command to main().
		return 0;
	}

	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}

	int tokenizeSuccess = tokenizeAndProcessCommand(buff, tokens, in_background);
	return tokenizeSuccess;
}
void executeCommand(_Bool in_background, char *tokens[]) {
	// add command to history
	char command[COMMAND_LENGTH];
	*command = 0; //contents of command[...]; are undetermined. Make sure it's clear before concat op
	for (int i = 0; i < NUM_TOKENS; i++) {
		strcat(command, tokens[i]);
		if (tokens[i+1] == NULL)
			break;
		else
			strcat(command, " ");
	}

//	char droppedCommand[COMMAND_LENGTH];
//	if (history.totalCommandsExecuted == 0)
//		droppedCommand = '\0';
//	else
//		strcpy(droppedCommand, history.historyArray[0]);

	// Don't add commands !n or !! to history
	if (command[0] != '!') {
		addCommandToHistory(command);
	}

	// Execute the command
	if (isBuiltInCommand(tokens)) {
		executeBuiltInCommand(tokens);
	} else {
		//external command, execute as seperate process
		pid_t pID = fork();
		if (pID == 0) { //child
			/*!! execvp() ONLY returns is an error has occurred. Otherwise this thread is chest-popped
			 * by the call to execvp (it is killed)!!*/
			if (execvp(tokens[0], tokens) == -1) {
				write(STDOUT_FILENO, strerror(errno), strlen(strerror(errno)));
				exit(0);
			}
		} else if (pID < 0) {
			perror("Failed to fork");
			write(STDOUT_FILENO, "Failed to fork", strlen("Failed to fork"));
		}

		//parent
		if (!in_background) {
			if (waitpid(pID, NULL, 0) == -1)
				perror("Error waiting for child to exit");
		}
		// Cleanup any previously exited background child processes
		cleanupZombies();
	}
}

_Bool isBuiltInCommand(char *tokens[]) {
	if(strcmp(tokens[0], "pwd") == 0 || strcmp(tokens[0], "cd") == 0 ||
		strcmp(tokens[0], "exit") == 0 || strcmp(tokens[0], "history") == 0
		|| tokens[0][0] == '!')
			return 1;
	return 0;
}

void executeBuiltInCommand(char *tokens[]) {
	if(strcmp(tokens[0], "pwd") == 0) {
		executePWDCommand();
	} else if (strcmp(tokens[0], "cd") == 0) {
		if (chdir(tokens[1]) == -1)
			 write(STDOUT_FILENO, strerror(errno), strlen(strerror(errno)));
	} else if (strcmp(tokens[0], "history") == 0){
		executePrintHistoryCommand();
	} else if (strcmp(tokens[0], "!!") == 0) {
		executeNumberedHistoryCommand(history.totalCommandsExecuted);
	} else if (tokens[0][0] == '!') {
		// validate input
		int commandNo = strtol(&tokens[0][1], NULL, 10);
		if(commandNo == 0) { // 0 is returned if input is invalid (not an int)
			write(STDOUT_FILENO, "Invalid history command number"
								, strlen("Invalid history command number"));
		} else {
			executeNumberedHistoryCommand(commandNo);
		}
	}
	else {
		//exit
		exit(0);
	}
}

void executePWDCommand() {
	char buff[150];
	if (!getcwd(buff, 150))
		write(STDOUT_FILENO, strerror(errno), strlen(strerror(errno)));
	else {
		//success
		write(STDOUT_FILENO, buff, strlen(buff));
	}
}

/*
 * pwd, cd, and exit are all executed by the shell itself. No execvp needed
 */
/**
* Read a command from the keyboard into the buffer 'buff' and tokenize it
* such that 'tokens[i]' points into 'buff' to the i'th token in the command.
* buff: Buffer allocated by the calling code. Must be at least
*
COMMAND_LENGTH bytes long.
* tokens[]: Array of character pointers which point into 'buff'. Must be at
*
least NUM_TOKENS long. Will strip out up to one final '&' token.
*
'tokens' will be NULL terminated.
* in_background: pointer to a boolean variable. Set to true if user entered
*
an & as their last token; otherwise set to false.
*/
int tokenizeAndProcessCommand(char* buff, char* tokens[], _Bool* in_background) {
	// Tokenize (buff has spaces replaced with nulls)
	int token_count = tokenize_command(buff, tokens);
	if (token_count == 0) {
		//error
		return token_count;
	}
	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		tokens[token_count - 1] = 0;
	}

	// tokenizing and processing successful
	return 1;
}

int tokenize_command(char *buff, char *tokens[]){
	int tokenCount = 0;
	char *tokenStart = buff;

	while (*buff != '\0'){
		if (*buff == ' '){//tokenStart points to start of newly found token
			if (tokenCount == NUM_TOKENS - 1) // no room for null terminator
				return 0; //error
			tokens[tokenCount++] = tokenStart;
			tokenStart = buff + 1;
			*buff = '\0';
		}
		buff++;
	}

	/*Final token and null terminate*/
	tokens[tokenCount++] = tokenStart;
	tokens[tokenCount + 1] = '\0';

	return tokenCount;
}

void resetBuffers(char* tokens[]) {
	for (int i = 0; i < NUM_TOKENS; i++) {
		if (tokens[i] == NULL)
			break;
		tokens[i] = '\0';
	}
}

void cleanupZombies() {
	// Cleanup any previously exited background child processes
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
	// do nothing.
}

