#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

#define EXIT_SUCCESS 0
#define EXIT_SYSTEM_FAILURE 500
//#define EXIT_USER_FAILURE 400
#define EXIT_USER_SIGNAL 401
#define EXIT_USER_TERMINATE_SIGNAL 402
#define EXIT_USER_INTERRUPT_SIGNAL 403

#define COMMAND_REPL_EXIT ".exit"
#define COMMAND_REPL_INSERT "insert"
#define COMMAND_REPL_SELECT "select"
#define MESSAGE_REPL_DONE "done."
#define MESSAGE_REPL_TABLE_FULL "Error: Table full.\n"

#define REPL_PROMPT "\ndb > "

#define INSERT_ARGS_OUT_OF_BOUND 4
#define INSERT_COMPARE_LENGTH 6
#define SELECT_COMPARE_LENGTH 6

#define COLUMN_USERNAME_SIZE 64
#define COLUMN_EMAIL_SIZE 256
#define COLUMN_PASSWORD_SIZE 256

typedef struct Row {
    u_int32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
    char password[COLUMN_PASSWORD_SIZE + 1];
} Row;

const u_int32_t ID_SIZE = size_of_attribute(Row, id); // 4B
const u_int32_t USERNAME_SIZE = size_of_attribute(Row, username); // 64B
const u_int32_t EMAIL_SIZE = size_of_attribute(Row, email); // 256B
const u_int32_t PASSWORD_SIZE = size_of_attribute(Row, password); // 256B
const u_int32_t ID_OFFSET = 0; // 0
const u_int32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE; // 4
const u_int32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE; // 68
const u_int32_t PASSWORD_OFFSET = EMAIL_OFFSET + EMAIL_SIZE; // 324
const u_int32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE + PASSWORD_SIZE; // 580

typedef enum MetaCommandResult {
    META_COMMAND_SUCCESS, META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum PrepareResult {
    PREPARE_SUCCESS, PREPARE_STRING_TOO_LONG, PREPARE_SYNTAX_ERROR, PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum StatementType {
    INSERT, SELECT
} StatementType;

typedef enum ExecuteResult {
    EXECUTE_NONE, EXECUTE_SUCCESS, EXECUTE_TABLE_FULL
} ExecuteResult;

typedef struct Statement {
    StatementType type;
    Row rowToInsert;
} Statement;

typedef struct InputBuffer {
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

void closeInputBuffer(InputBuffer *ib) {
//    ib->buffer = ""; // does it even work, correctly?

// zero out everything
    memset(ib->buffer, '\0', ib->bufferLength);
    ib->inputLength = 0;
    ib->bufferLength = 0;

    free(ib->buffer);
    free(ib);
}

MetaCommandResult doMetaCommand(InputBuffer *ib) {
    if (strcmp(ib->buffer, COMMAND_REPL_EXIT) == 0) {
        closeInputBuffer(ib);
        exit(EXIT_SUCCESS);
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

void printPrompt() {
    printf(REPL_PROMPT);
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

PrepareResult prepareInsert(InputBuffer *ib, Statement *s) {
    s->type = INSERT;

    char *keyword = strtok(ib->buffer, " ");
    char *idStr = strtok(NULL, " ");
    char *username = strtok(NULL, " ");
    char *email = strtok(NULL, " ");
    char *password = strtok(NULL, " ");

    if (idStr == NULL || username == NULL || email == NULL || password == NULL) {
        return PREPARE_SYNTAX_ERROR;
    }

    /*
     * Clang-Tidy: 'atoi' used to convert a string to an integer value, but function will not report conversion errors; consider using 'strtol' instead
     * int id = atoi(idStr);
     */
    char *err;
    int tmp = (int) strtol(idStr, &err, 10);
    if (*err) {
        tmp = -1;
    }
    int id = tmp;

    if (strlen(username) > COLUMN_USERNAME_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }
    if (strlen(email) > COLUMN_EMAIL_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }

    s->rowToInsert.id = id;
    strcpy(s->rowToInsert.username, username);
    strcpy(s->rowToInsert.email, email);
    strcpy(s->rowToInsert.password, password);

}

PrepareResult prepareStatement(InputBuffer *ib, Statement *s) {
    if (strncmp(ib->buffer, COMMAND_REPL_INSERT, INSERT_COMPARE_LENGTH) == 0) {
        return prepareInsert(ib, s);
    }
    if (strncmp(ib->buffer, COMMAND_REPL_SELECT, SELECT_COMPARE_LENGTH) == 0) {
        s->type = SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void serializeRow(Row *src, void *dst) {
    memcpy((char *) dst + ID_OFFSET, &(src->id), ID_SIZE);
    memcpy((char *) dst + USERNAME_OFFSET, &(src->username), USERNAME_SIZE);
    memcpy((char *) dst + EMAIL_OFFSET, &(src->email), EMAIL_SIZE);
    memcpy((char *) dst + PASSWORD_OFFSET, &(src->password), PASSWORD_SIZE);
}

void deserializeRow(void *src, Row *dst) {
    memcpy(&(dst->id), (char *) src + ID_OFFSET, ID_SIZE);
    memcpy(&(dst->username), (char *) src + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(dst->email), (char *) src + EMAIL_OFFSET, EMAIL_SIZE);
    memcpy(&(dst->password), (char *) src + PASSWORD_OFFSET, PASSWORD_SIZE);
}

const u_int32_t TABLE_PAGE_SIZE = 4 * 1024;
#define TABLE_MAX_PAGES 100
const u_int32_t ROWS_PER_PAGE = TABLE_PAGE_SIZE / ROW_SIZE;
const u_int32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct Table {
    u_int32_t numRows;
    void *pages[TABLE_MAX_PAGES];
} Table;

Table *newTable() {
    Table *t = (Table *) calloc(1, sizeof(Table));
    t->numRows = 0;
    for (u_int32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        t->pages[i] = NULL;
    }
    return t;
}

void closeTable(Table *t) {
//    memset(t->pages, '\0', sizeof(Table));
    t->numRows = 0;
    for (int i = 0; t->pages[i]; i++) {
        memset(t->pages[i], '\0', TABLE_PAGE_SIZE);
        free(t->pages[i]);
    }
    t->numRows = 0;
    free(t);
}

void *rowSlot(Table *table, u_int32_t rowNum) {
    u_int32_t pageNum = rowNum / ROWS_PER_PAGE;
    void *page = table->pages[pageNum];
    if (page == NULL) {
        page = table->pages[pageNum] = malloc(TABLE_PAGE_SIZE);
    }
    u_int32_t rowOffset = rowNum % ROWS_PER_PAGE;
    u_int32_t byteOffset = rowOffset * ROW_SIZE;
    return (char *) page + byteOffset;
}

void printRow(Row *r) {
    printf("[%d, %s, %s, %s]\n", r->id, r->username, r->email, r->password);
}

ExecuteResult executeInsert(Statement *stmt, Table *t) {
    if (t->numRows >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }
    Row *rowToInsert = &(stmt->rowToInsert);
    serializeRow(rowToInsert, rowSlot(t, t->numRows));
    t->numRows += 1;

    return EXECUTE_SUCCESS;
}

ExecuteResult executeSelect(Table *t) {
    Row row;
    for (u_int32_t i = 0; i < t->numRows; i++) {
        deserializeRow(rowSlot(t, i), &row);
        printRow(&row);
    }

    return EXECUTE_SUCCESS;
}

ExecuteResult executeStatement(Statement *s, Table *t) {
    switch (s->type) {
        case (INSERT): {
//            printf("Insert...\t");
            return executeInsert(s, t);
        }
        case (SELECT): {
//            printf("Select...\t");
            return executeSelect(t);
        }
        default: {
            printf("Unknown statement.\n");
            return EXECUTE_NONE;
        }
    }
}

int main() {
    if (signal(SIGUSR1, signalHandler) == SIG_ERR) {
        // User defined signal, used for IPC etc.
        printf("cannot handle SIGUSR1.");
    }
    if (signal(SIGTERM, signalHandler) == SIG_ERR) {
        printf("cannot handle SIGTERM.");
    }
    if (signal(SIGINT, signalHandler) == SIG_ERR) {
        printf("cannot handle SIGINT.");
    }
//    Cannot handle SIGKILL
//    if (signal(SIGKILL, signalHandler) == SIG_ERR) {
//        // This signal cannot be caught
//        printf("cannot handle SIGKILL\n");
//    }
//    Cannot handle SIGSTOP
//    if (signal(SIGSTOP, signalHandler) == SIG_ERR) {
//        // ctrl+z: moves program to suspended state; but this cannot be caught
//        printf("cannot handle SIGSTOP\n");
//    }

    printf("\nCOMMAND: insert ID(number) username(string) email(string) password(string)");
    printf("\nCOMMAND: select");
    printf("\nCOMMAND: .exit\n");

    Table *t = newTable();

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
            case (PREPARE_STRING_TOO_LONG):
                printf("Input error: String is too long.\n");
                continue;
            case (PREPARE_SYNTAX_ERROR):
                printf("Syntax error. Could not parse statement.\n");
                continue;
            case (PREPARE_UNRECOGNIZED_STATEMENT): {
                printf("Unrecognized keyword at start of '%s'.\n", ib->buffer);
                continue;
            }
        }
//        printf("Executed.\t");
        switch (executeStatement(&s, t)) {
            case (EXECUTE_SUCCESS):
                printf(MESSAGE_REPL_DONE);
                break;
            case (EXECUTE_TABLE_FULL):
                printf(MESSAGE_REPL_TABLE_FULL);
                break;
            case EXECUTE_NONE:
                printf("Error: Unknown command.\n");
                break;
        }
    }
    return 0;
}
