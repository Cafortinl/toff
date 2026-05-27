#include <stdio.h>

#include "sqlite_database_administrator.h"
#include "branch.h"
#include "department.h"
#include "employee.h"
#include "position.h"

int main(int argc, char** argv) {
    sqlite_database_administrator *dba = sqlite_dba_connect_to_db("assets/db.sqlite");
    if (!dba) {
        printf("There was an error while trying to connect to the database.\n");
        return 1;
    }

    bool table_created;

    if (!sqlite_dba_check_if_table_exists(dba, "Position")) {
        table_created = sqlite_dba_create_table(dba, &position_table);

        if (!table_created) {
            printf("There was an error while trying to create the 'Position' table.\n");
            return 2;
        }
    }

    if (!sqlite_dba_check_if_table_exists(dba, "Branch")) {
        table_created = sqlite_dba_create_table(dba, &branch_table);

        if (!table_created) {
            printf("There was an error while trying to create the 'Branch' table.\n");
            return 2;
        }
    }

    if (!sqlite_dba_check_if_table_exists(dba, "Department")) {
        table_created = sqlite_dba_create_table(dba, &department_table);

        if (!table_created) {
            printf("There was an error while trying to create the 'Department' table.\n");
            return 2;
        }
    }

    if (!sqlite_dba_check_if_table_exists(dba, "Employee")) {
        table_created = sqlite_dba_create_table(dba, &employee_table);

        if (!table_created) {
            printf("There was an error while trying to create the 'Employee' table.\n");
            return 2;
        }
    }

    bool db_connection_closed = sqlite_dba_disconnect_from_db(dba);
    if (!db_connection_closed) {
        printf("There was an error while trying to disconnect from the database.\n");
        return 3;
    }

    return 0;
}
