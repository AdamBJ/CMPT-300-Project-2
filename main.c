#include <sys/wait.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "supportFunctions.h"

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

struct historyStruct history;

int main(int argc, char* argv[])
{
	/* Set up the signal handler */
	struct sigaction handler;
	handler.sa_handler = handle_SIGINT;
	handler.sa_flags = 0;
	sigemptyset(&handler.sa_mask);
	sigaction(SIGINT, &handler, NULL);

	/* Initialize variables*/
	history.currentSize = 0;
	history.totalCommandsExecuted = 0;
	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];
	_Bool in_background = false;

	/* Read -> execute loop*/
	while (true) {
		executePWDCommand();
		//write used instead of printf for compatibility with signals
		write(STDOUT_FILENO, "> ", strlen("> "));

		if (read_command(input_buffer, tokens, &in_background))
			executeCommand(in_background, tokens);

		//reset
		zeroArray(tokens);
	}

return 0;
}
