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
int main(int argc, char* argv[])
{
	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];

	while (true) {
		// Get command
		// Use write because we need to use read()/write() to work with
		// signals which are incompatible with printf().
		write(STDOUT_FILENO, "\n> ", strlen("> "));
		_Bool in_background = false;
		read_command(input_buffer, tokens, &in_background);

		if (isBuiltInCommand(tokens)) {
			executeBuiltInCommand(tokens);
		} else { //command is executed using execvp()
			pid_t pID = fork();

			if (pID == 0) {
				//child
				if (execvp(tokens[0], tokens) == -1) {
					write(STDOUT_FILENO, strerror(errno), strlen(strerror(errno)));
				}

				exit(0);
			} else if (pID < 0) {
				perror("Failed to fork");
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

return 0;
}
