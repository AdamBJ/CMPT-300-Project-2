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
		strcpy(history.historyArray[HISTORY_DEPTH-1], command);
	} else {
		//historyArray array isn't full
		strcpy(history.historyArray[history.currentSize], command);
		history.currentSize++;
	}
}

void printHistory() {
	char intAsStr[50];

	for (int i = 0; i < history.currentSize; i++) {
		sprintf(intAsStr, "%d",
				history.totalCommandsExecuted - history.currentSize + i + 1);// + 1 to start count from 1 instead of 0
		strcat(intAsStr, "\t");
		write(STDOUT_FILENO, intAsStr, strlen(intAsStr));
		write(STDOUT_FILENO, history.historyArray[i],
				strlen(history.historyArray[i]));
		write(STDOUT_FILENO, "\n", strlen("\n"));
	}
}
