//
// Created by anubhav on 18/10/23.
//

#ifndef CDB_ROW_H
#define CDB_ROW_H


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


#endif //CDB_ROW_H
