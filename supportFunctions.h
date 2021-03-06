#ifndef SUPPORTFUNCTIONS_H
#define SUPPORTFUNCTIONS_H

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define HISTORY_DEPTH 10
struct historyStruct {
	char historyArray[HISTORY_DEPTH][COMMAND_LENGTH];
	int currentSize;
	int totalCommandsExecuted;
};
extern struct historyStruct history;

void handle_SIGINT();
void executeCommand(_Bool in_background, char* tokens[]);
void executePWDCommand();
void executeBuiltInCommand(char *tokens[]);
_Bool isBuiltInCommand(char *tokens[]);
void cleanupZombies();
void zeroArray(char* tokens[]);
int tokenize_command(char *buff, char *tokens[]);
int tokenizeAndProcessCommand(char* buff, char* tokens[], _Bool* in_background);
int read_command(char *buff, char *tokens[], _Bool *in_background);

#endif
