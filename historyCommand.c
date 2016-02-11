#include <sys/wait.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "supportFunctions.h"
#include "historyCommand.h"

void executeHistoryCommand() {
	printHistory();
}

void addCommandToHistory(char *command) {
	if (history.currentSize == HISTORY_DEPTH) {
		for (int i = 1; i < HISTORY_DEPTH; i++) {
			strcpy(history.historyArray[i-1], history.historyArray[i]);
		}
		strcpy(history.historyArray[HISTORY_DEPTH-1], command); //todo what happens when tokens is overwritten?
	} else {
		//historyArray array isn't full
		strcpy(history.historyArray[history.currentSize - 1], command);
		history.currentSize++;
	}
}

void printHistory() {
	int commandLabeler = history.totalCommandsExecuted;
	char intAsStr[50];

	for (int i = 0; i < history.currentSize; i++) {
		sprintf(intAsStr, "%d", commandLabeler);
		strcat(intAsStr, "\t");
		commandLabeler--;
		write(STDOUT_FILENO, intAsStr, strlen(intAsStr));
		write(STDOUT_FILENO, history.historyArray[i],
				strlen(history.historyArray[i]));
		write(STDOUT_FILENO, "\n", strlen("\n"));
	}
}
