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
		perror( "Unable to read command. Terminating.\n ");
		exit(-1);
	}
	if ( (length < 0) && (errno == EINTR) ) {
		// read interrupted by signal. Report failure of read_command to main().
		return 0;
	}

	// Null terminate input and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}

	int tokenizeSuccess = tokenizeAndProcessCommand(buff, tokens, in_background);
	return tokenizeSuccess;
}
void executeCommand(_Bool in_background, char *tokens[]) {
	if (isBuiltInCommand(tokens)) {
		executeBuiltInCommand(tokens);
	} else {
		//external command, execute as separate process

		pid_t pID = fork();
		if (pID == 0) {
			//in child

			/* execvp() ONLY returns is an error has occurred. Otherwise child
			 * process is chest-popped by the call to execvp (it is terminated)*/
			if (execvp(tokens[0], tokens) == -1) {
				write(STDOUT_FILENO, strerror(errno), strlen(strerror(errno)));
				write(STDOUT_FILENO, "\n", strlen("\n"));
				exit(0);
			}
		} else if (pID < 0) {
			perror("Failed to fork");
			return;
		}

		//parent
		if (!in_background) {
			if (waitpid(pID, NULL, 0) == -1)
				perror("Error waiting for child to exit");
		} else {
			//give time for child to finish executing before returning to loop
			//to ensure new prompt is printed after results of previous command
			usleep(10000);
		}

		cleanupZombies();
	}
}

_Bool isBuiltInCommand(char *tokens[]) {
	if(strcmp(tokens[0], "pwd") == 0 || strcmp(tokens[0], "cd") == 0 ||
		strcmp(tokens[0], "exit") == 0 || strcmp(tokens[0], "history") == 0
		|| strcmp(tokens[0], "\0") == 0 || tokens[0][0] == '!')
			return 1;
	return 0;
}

void executeBuiltInCommand(char *tokens[]) {
	if(strcmp(tokens[0], "pwd") == 0) {
		executePWDCommand();
		write(STDOUT_FILENO, "\n", strlen("\n"));
	} else if (strcmp(tokens[0], "cd") == 0) {
		if (chdir(tokens[1]) == -1)
			 write(STDOUT_FILENO, strerror(errno), strlen(strerror(errno)));
	} else if (strcmp(tokens[0], "history") == 0){
		executePrintHistoryCommand();
	} else if (strcmp(tokens[0], "!!") == 0) {
		executeNumberedHistoryCommand(history.totalCommandsExecuted);
	} else if (tokens[0][0] == '!') {
		// validate input (check syntax)
		float commandNo = strtof(&tokens[0][1], NULL);
		if(commandNo == 0 || commandNo != (int)commandNo) {
			write(STDOUT_FILENO, "Invalid history command number\n"
								, strlen("Invalid history command number\n"));
		} else {
			executeNumberedHistoryCommand(commandNo);
		}
	} else if (strcmp(tokens[0], "\0") == 0) {
		//carriage return, empty command
		return;
	} else {
		//exit command
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

/*Returns 1 if processing successful, 0 otherwise*/

int tokenizeAndProcessCommand(char* buff, char* tokens[], _Bool* in_background) {
	// Add command in buff to history if not !n, !!, or carriage return (newline)
	if (buff[0] != '!' && buff[0] != '\0') {
		addCommandToHistory(buff);
	}

	int token_count = tokenize_command(buff, tokens);
	if (token_count == 0) {
		//error
		return 0;
	}

	// Extract & if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		tokens[token_count - 1] = 0;
	}

	// tokenizing and processing successful
	return 1;
}

/* Fill token array. Upon return token array points
 * into buff, which has been transformed into a series
 * of null-terminated tokens rather than a single
 * null-terminated string. */

int tokenize_command(char *buff, char *tokens[]){
	int tokenCount = 0;
	char *tokenStart = buff;

	while (*buff != '\0'){
		if (*buff == ' '){//tokenStart points to start token delimited by this space
			if (tokenCount == NUM_TOKENS - 1) // no room for null terminator
				return 0; //error
			tokens[tokenCount++] = tokenStart;
			tokenStart = buff + 1;
			*buff = '\0';
		}
		buff++;
	}

	/*Create final token and null terminate*/
	tokens[tokenCount++] = tokenStart;
	tokens[tokenCount + 1] = '\0';

	return tokenCount;
}

void zeroArray(char* tokens[]) {
	for (int i = 0; i < NUM_TOKENS; i++) {
		if (tokens[i] == '\0')
			break;
		tokens[i] = '\0';
	}
}

// Cleanup any previously exited background child processes

void cleanupZombies() {
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
}

