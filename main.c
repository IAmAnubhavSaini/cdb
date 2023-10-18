#include "structures.h"



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
    if (tmp <= 0) {
        return PREPARE_NON_POSITIVE_ID;
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
    strncpy((char *) dst + USERNAME_OFFSET, src->username, USERNAME_SIZE);
    strncpy((char *) dst + EMAIL_OFFSET, src->email, EMAIL_SIZE);
    strncpy((char *) dst + PASSWORD_OFFSET, src->password, PASSWORD_SIZE);
}

void deserializeRow(void *src, Row *dst) {
    memcpy(&(dst->id), (char *) src + ID_OFFSET, ID_SIZE);
    strncpy(dst->username, (char *) src + USERNAME_OFFSET, USERNAME_SIZE);
    strncpy(dst->email, (char *) src + EMAIL_OFFSET, EMAIL_SIZE);
    strncpy(dst->password, (char *) src + PASSWORD_OFFSET, PASSWORD_SIZE);
}





Pager *pagerOpen(const char *filename) {
    int fd = open(filename, O_RDWR | O_CREAT, // Read/Write or Create
                  S_IWUSR | S_IRUSR // write/read permission
    );

    if (fd == -1) {
        printf("File System Error: Cannot open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    off_t fileLength = lseek(fd, 0, SEEK_END);

    Pager *pager = (Pager *) (malloc(sizeof(Pager) * 1));
    pager->fd = fd;
    pager->length = fileLength;

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        pager->pages[i] = NULL;
    }

    return pager;
}



Table *dbOpen(const char *filename) {
    Pager *pager = pagerOpen(filename);
    uint32_t numRows = pager->length / ROW_SIZE;
    Table *table = (Table *) (malloc(sizeof(Table) * 1));
    table->pager = pager;
    table->numRows = numRows;
    return table;
}

void * getPage(Pager * pager, uint32_t pageNum) {
    if(pageNum > TABLE_MAX_PAGES) {
        printf("Range error: Tried to fetch page number out of bounds. %d > %d\n", pageNum, TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }

    if(pager->pages[pageNum] == NULL) {
        // CACHE miss
        void * page = (void*)(malloc(TABLE_PAGE_SIZE));
        uint32_t numberOfPages = pager->length / TABLE_PAGE_SIZE;

        // Partial page save at End of File
        if(pager->length % TABLE_PAGE_SIZE){
            numberOfPages += 1;
        }

        if(pageNum <= numberOfPages){
            //unistd.h # define SEEK_SET	0	/* Seek from beginning of file.  */
            lseek(pager->fd, pageNum * TABLE_PAGE_SIZE, SEEK_SET);
            ssize_t bytesRead = read(pager->fd, page, TABLE_PAGE_SIZE);
            if(bytesRead == -1){
                printf("Error reading file: %d\n", errno);
                exit(EXIT_FAILURE);
            }
        }
        pager->pages[pageNum] = page;

    }
    return pager->pages[pageNum];
}

void flushPager(Pager * pager, uint32_t pageNum, uint32_t size){
    if(pager->pages[pageNum] == NULL) {
        printf("Error: Tried to flush NULL page\n");
        exit(EXIT_FAILURE);
    }

    off_t offset = lseek(pager->fd, pageNum * TABLE_PAGE_SIZE, SEEK_SET);
    if(offset == -1) {
        printf("Error: Cannot seek from the beginning. %d\n", errno);
        exit(EXIT_FAILURE);
    }

    ssize_t bytesWritten = write(pager->fd, pager->pages[pageNum], size);
    if(bytesWritten == -1) {
        printf("Error: Cannot write. %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void dbClose(Table * table) {
    Pager * pager = table->pager;
    uint32_t numFullPages = table->numRows / ROWS_PER_PAGE;

    for(uint32_t i = 0; i < numFullPages; i++){
        if(pager->pages[i] == NULL) {
            continue;
        }
        flushPager(pager, i, TABLE_PAGE_SIZE);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    uint32_t numPartialPageRows = table->numRows % ROWS_PER_PAGE;
    if(numPartialPageRows > 0) {
        uint32_t pageNum = numFullPages;
        if(pager->pages[pageNum] != NULL) {
            flushPager(pager, pageNum, numPartialPageRows * ROW_SIZE);
            free(pager->pages[pageNum]);
            pager->pages[pageNum] = NULL;
        }
    }

    int result = close(pager->fd);
    if(result == -1) {
        printf("Error closing db.\n");
        exit(EXIT_FAILURE);
    }
    for(uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        void * page = pager->pages[i];
        if(page) {
            free(page);
            pager->pages[i] = NULL;
        }
    }
    free(pager);
    free(table);

}

MetaCommandResult doMetaCommand(InputBuffer *ib, Table *table) {
    if (strcmp(ib->buffer, COMMAND_REPL_EXIT) == 0) {
        closeInputBuffer(ib);
        dbClose(table);
        exit(EXIT_SUCCESS);
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

void *rowSlot(Table *table, u_int32_t rowNum) {
    u_int32_t pageNum = rowNum / ROWS_PER_PAGE;
    void *page = getPage(table->pager, pageNum);
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

int main(int argc, char * argv[]) {
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

    if(argc < 2) {
        printf("Error: Must supply a database filename.\n");
        exit(EXIT_FAILURE);
    }

    char * filename = argv[1];
    Table * t = dbOpen(filename);

    InputBuffer *ib = newInputBuffer();
    for (int i = 0; i == 0; i += 0) {
        printPrompt();
        readInput(ib);


        if (ib->buffer[0] == '.') {
            switch (doMetaCommand(ib, t)) {
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
            case (PREPARE_NON_POSITIVE_ID):
                printf("Input error: ID must be positive.\n");
                continue;
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
