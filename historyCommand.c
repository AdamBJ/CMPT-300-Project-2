#include <sys/wait.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "supportFunctions.h"
#include "historyCommand.h"

void executePrintHistoryCommand() {
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

void executeSpecificHistoryCommand(int commandNo) {
	if (history.currentSize < HISTORY_DEPTH) {
		if (commandNo < 1 ) {
			write(STDOUT_FILENO, "Invalid command number",
					strlen("Invalid command number"));
			return;
		} else {
			// success
			readAndExecuteCommandAtHistoryArrayIndex(commandNo-1);
		}
	} else {
		//history full
		if (history.totalCommandsExecuted - 10 > history.totalCommandsExecuted
				- commandNo) {
			write(STDOUT_FILENO, "Invalid command number",
							strlen("Invalid command number"));
			return;
		} else {
			readAndExecuteCommandAtHistoryArrayIndex();//todo what index?
		}
	}
}
void readAndExecuteCommandAtHistoryArrayIndex(int commandNo) {
	// success
	char commandBuff[COMMAND_LENGTH];
	strcpy(commandBuff, history.historyArray[commandNo + 1]);
	char tokens[NUM_TOKENS];
	_Bool in_background = false;
	read_command(commandBuff, tokens, &in_background);
	executeCommand(in_background, tokens);
}

