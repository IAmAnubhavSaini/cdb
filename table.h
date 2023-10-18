//
// Created by anubhav on 18/10/23.
//

#ifndef CDB_TABLE_H
#define CDB_TABLE_H

const u_int32_t TABLE_PAGE_SIZE = 4 * 1024;
#define TABLE_MAX_PAGES 100
const u_int32_t ROWS_PER_PAGE = TABLE_PAGE_SIZE / ROW_SIZE;
const u_int32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct Pager {
    int fd;
    uint32_t length;
    void *pages[TABLE_MAX_PAGES];
} Pager;

typedef struct Table {
    u_int32_t numRows;
    Pager *pager;
} Table;

#endif //CDB_TABLE_H
