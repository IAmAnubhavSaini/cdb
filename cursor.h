//
// Created by anubhav on 18/10/23.
//

#ifndef CDB_CURSOR_H
#define CDB_CURSOR_H

typedef struct Cursor {
    Table *table;
    uint32_t rowNum;
    bool EoT; // EndOfTable; location after last row.
} Cursor;

#endif //CDB_CURSOR_H
