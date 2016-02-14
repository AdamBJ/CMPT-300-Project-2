#include <sys/wait.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "supportFunctions.h"


/**
* Steps For Basic Shell:
* 1. Fork a child process
* 2. Child process invokes execvp() using results in token array.
* 3. If in_background is false, parent waits for
* child to finish. Otherwise, parent loops back to
* read_command() again immediately.
*/

struct historyStruct history;

int main(int argc, char* argv[])
{
	history.currentSize = 0;
	history.totalCommandsExecuted = 0;
	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];

	while (true) {
		// Get command

		executePWDCommand();
		// Use write because we need to use read()/write() to work with
		// signals which are incompatible with printf().
		write(STDOUT_FILENO, "> ", strlen("> "));

		_Bool in_background = false;
		if (read_command(input_buffer, tokens, &in_background))
			executeCommand(in_background, tokens);

		resetBuffers(tokens);
	}

return 0;
}
