#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define EXIT_SUCCESS 0
#define EXIT_SYSTEM_FAILURE 500
//#define EXIT_USER_FAILURE 400
#define EXIT_USER_SIGNAL 401
#define EXIT_USER_TERMINATE_SIGNAL 402
#define EXIT_USER_INTERRUPT_SIGNAL 403
#define COMMAND_REPL_EXIT ".exit"

typedef enum {
    META_COMMAND_SUCCESS, META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum {
    PREPARE_SUCCESS, PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum { INSERT, SELECT} StatementType;

typedef struct {
    StatementType type;
} Statement;

typedef struct {
    char *buffer;
    size_t bufferLength;
    size_t inputLength;
} InputBuffer;

InputBuffer *newInputBuffer() {
    InputBuffer *ib = (InputBuffer *) malloc(sizeof(InputBuffer) * 1);
    ib->buffer = NULL;
    ib->bufferLength = 0;
    ib->inputLength = 0;
    return ib;
}

MetaCommandResult doMetaCommand(InputBuffer * ib) {
    if (strcmp(ib->buffer, ".exit") == 0) {
        exit(EXIT_SUCCESS);
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

void printPrompt() {
    printf("db > ");
}

void readInput(InputBuffer *ib) {
    ssize_t bytesRead = getline(&(ib->buffer), &(ib->inputLength), stdin);

    if (bytesRead <= 0) {
        printf("Error reading input.\nExiting.\n");
        exit(EXIT_SYSTEM_FAILURE);
    }

    /* Ignoring trailing newline */
    ib->inputLength = bytesRead - 1;
    ib->buffer[bytesRead - 1] = 0;
}

void closeInputBuffer(InputBuffer *ib) {
    free(ib->buffer);
    free(ib);
}

void signalHandler(int signal) {
    printf("\nReceived signal: %d.\n", signal);
    switch (signal) {
        case SIGUSR1: {
            printf("SIGUSR1 received.");
            exit(EXIT_USER_SIGNAL);
        }
        case SIGTERM: {
            printf("SIGTERM received.");
            exit(EXIT_USER_TERMINATE_SIGNAL);
        }
        case SIGINT: {
            printf("SIGINT received: ctrl+c.");
            exit(EXIT_USER_INTERRUPT_SIGNAL);
        }
        default:
            printf("Program won't handle it.\n");
    }
}

PrepareResult prepareStatement(InputBuffer * ib, Statement * s) {
    if(strncmp(ib->buffer, "insert", 6)==0) {
        s->type = INSERT;
        return PREPARE_SUCCESS;
    }
    if(strncmp(ib->buffer, "select", 6) == 0) {
        s->type = SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void executeStatement(Statement * s) {
    switch(s->type) {
        case (INSERT): {
            printf("Insert...\n");
            break;
        }
        case (SELECT): {
            printf("Select...\n");
            break;
        }
    }
}

int main() {
    if (signal(SIGUSR1, signalHandler) == SIG_ERR) {
        // User defined signal, used for IPC etc.
        printf("cannot handle SIGUSR1.");
    };
    if (signal(SIGTERM, signalHandler) == SIG_ERR) {
        printf("cannot handle SIGTERM.");
    };
    if (signal(SIGINT, signalHandler) == SIG_ERR) {
        printf("cannot handle SIGINT.");
    };
    if (signal(SIGKILL, signalHandler) == SIG_ERR) {
        // This signal cannot be caught
        printf("cannot handle SIGKILL\n");
    };
    if (signal(SIGSTOP, signalHandler) == SIG_ERR) {
        // ctrl+z: moves program to suspended state; but this cannot be caught
        printf("cannot handle SIGSTOP\n");
    }

    InputBuffer *ib = newInputBuffer();
    for (int i = 0; i == 0; i += 0) {
        printPrompt();
        readInput(ib);


        if (ib->buffer[0] == '.') {
            switch (doMetaCommand(ib)) {
                case (META_COMMAND_SUCCESS):
                    continue;
                case (META_COMMAND_UNRECOGNIZED_COMMAND): {
                    printf("Unrecognized command '%s'\n", ib->buffer);
                    continue;
                }
            }
        }

        Statement s;
        switch (prepareStatement(ib, &s)) {
            case (PREPARE_SUCCESS):
                break;
            case (PREPARE_UNRECOGNIZED_STATEMENT): {
                printf("Unrecognized keyword at start of '%s'.\n", ib->buffer);
                continue;
            }
        }
        executeStatement(&s);
        printf("Executed.\n");
    }
    return 0;
}
