#ifndef SUPPORTFUNCTIONS_H
#define SUPPORTFUNCTIONS_H

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)

void executeBuiltInCommand(char *tokens[]);
_Bool isBuiltInCommand(char *tokens[]);
void cleanupZombies();
int tokenize_command(char *buff, char *tokens[]);
void read_command(char *buff, char *tokens[], _Bool *in_background);

#endif
