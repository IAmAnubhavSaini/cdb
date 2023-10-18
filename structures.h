//
// Created by anubhav on 18/10/23.
//

#ifndef CDB_STRUCTURES_H
#define CDB_STRUCTURES_H

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

#include "commands.h"
#include "row.h"
#include "table.h"

#define EXIT_SUCCESS 0
#define EXIT_SYSTEM_FAILURE 500
//#define EXIT_USER_FAILURE 400
#define EXIT_USER_SIGNAL 401
#define EXIT_USER_TERMINATE_SIGNAL 402
#define EXIT_USER_INTERRUPT_SIGNAL 403

#define REPL_PROMPT "\ndb > "

#define INSERT_ARGS_OUT_OF_BOUND 4
#define INSERT_COMPARE_LENGTH 6
#define SELECT_COMPARE_LENGTH 6




typedef enum MetaCommandResult {
    META_COMMAND_SUCCESS, META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum PrepareResult {
    PREPARE_SUCCESS,
    PREPARE_NON_POSITIVE_ID,
    PREPARE_STRING_TOO_LONG,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT
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



typedef struct Cursor {
    Table *table;
    uint32_t rowNum;
    bool EoT; // EndOfTable; location after last row.
} Cursor;

#endif //CDB_STRUCTURES_H
