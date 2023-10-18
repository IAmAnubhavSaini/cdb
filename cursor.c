#include "structures.h"
#include "cursor.h"

Cursor *newCursor() {
    Cursor *c = (Cursor *) malloc(sizeof(Cursor));
    c->table = NULL;
    c->rowNum = 0;
    c->EoT = false;
    return c;
}

Cursor *tableStart(Table *table) {
    Cursor *cursor = newCursor();
    cursor->table = table;
    cursor->rowNum = 0;
    cursor->EoT = (table->numRows == 0);
    return cursor;
}
