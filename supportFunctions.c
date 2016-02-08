#include <sys/wait.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "supportFunctions.h"
/*
 * pwd, cd, and exit are all executed by the shell itself. No execvp needed
 */
void executeBuiltInCommand(char *tokens[]) {
	if(strcmp(tokens[0], "pwd") == 0) {
		char buff[150];
		 if(!getcwd(buff, 150))
			 write(STDOUT_FILENO, strerror(errno), strlen(strerror(errno)));
		 else {//success
			 write(STDOUT_FILENO, buff, strlen(buff));
		 }

	} else if (strcmp(tokens[0], "cd") == 0) {
		if (chdir(tokens[1]) == -1) {
			 write(STDOUT_FILENO, strerror(errno), strlen(strerror(errno)));
		}
	} else {
		//exit
		exit(0);
	}
}

_Bool isBuiltInCommand(char *tokens[]) {
	if(strcmp(tokens[0], "pwd") == 0 || strcmp(tokens[0], "cd") == 0 ||
		strcmp(tokens[0], "exit") == 0)
			return 1;
	return 0;
}

void cleanupZombies() {
	// Cleanup any previously exited background child processes
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;

	// do nothing.
}

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
int tokenize_command(char *buff, char *tokens[]){
	int tokenCount = 0;
	char *tokenStart = buff;

	while (*buff != 0){
		if (*buff == 32){//" "
			tokens[tokenCount++] = tokenStart;
			tokenStart = buff + 1;
			*buff = 0;
		}
		buff++;
	}

	/*Final token*/
	tokens[tokenCount++] = tokenStart;

	//printf("%s \n", tokens[0]);
//	printf("%s \n", tokens[1]);

	return tokenCount;
}

void read_command(char *buff, char *tokens[], _Bool *in_background)
	{
		*in_background = false;

		// Read input
		int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);
		if ( (length < 0) && (errno != EINTR) ) //EINTR == interrupted system call
		{
			perror("Unable to read command. Terminating.\n");
			exit(-1); /* terminate with error */
		}

		// Null terminate and strip \n.
		buff[length] = '\0';
		if (buff[strlen(buff) - 1] == '\n') {
			buff[strlen(buff) - 1] = '\0';
		}

		// Tokenize (saving original command string)
		int token_count = tokenize_command(buff, tokens);
		if (token_count == 0) {
			return;
		}

		// Extract if running in background:
		if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
			*in_background = true;
			tokens[token_count - 1] = 0;
		}
}
