#include <stdio.h>
#include "sqlite_database_administrator.h"
#include "employee.h"

int main(int argc, char** argv) {
    sqlite_database_administrator* dba = sqlite_dba_connect_to_db("assets/db.sqlite");
    if (!dba) {
        printf("There was an error while trying to connect to the database.\n");
        return 1;
    }
    
    bool table_created = sqlite_dba_create_table(dba, &employee_table);
    if (!table_created) {
        printf("There was an error creating the table.\n");
        return 2;
    }

    bool successfully_closed = sqlite_dba_disconnect_from_db(dba);
    if (!successfully_closed) {
        printf("There was an error while disconnecting from the database.\n");
        return 3;
    }

    return 0;
}
