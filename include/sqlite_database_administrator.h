/**
 * File - sqlite_database_administrator.h
 * Author - C. Fortín (cafortinl)
 *
 * Manage connections and queries to SQLite databases.
 */
#ifndef SQLITE_DATABASE_ADMINISTRATOR_H
#define SQLITE_DATABASE_ADMINISTRATOR_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sqlite3.h>

// Section: Query Filters
#define SET_NODETYPES(left, right) (((left) << 4) | (right))

enum QUERY_FILTER_NODE_TYPE {
    QF_NONE,
    QF_TEXT,
    QF_NODE,
    QUERY_FILTER_NODE_TYPE_COUNT
};

enum QUERY_FILTER_OPERATIONS {
    QF_NOT,
    QF_AND,
    QF_OR,
    QF_LT,
    QF_LTE,
    QF_GT,
    QF_GTE,
    QF_BETWEEN,
    QF_LIKE,
    QF_EQUALS,
    QUERY_FILTER_OPERATIONS_COUNT

};

typedef struct filter_node {
    union {
        char* text;
        struct filter_node* node;
    } left;

    union {
        char* text;
        struct filter_node* node;
    } right;

    //<< 4 -> left node type
    //& 15 -> right node type
    uint8_t node_types;

    enum QUERY_FILTER_OPERATIONS operation;
} query_filter_node;

// EndSection: Query Filters

enum SQLITE_DATA_TYPES {
    SQLITE_DBA_INTEGER,
    SQLITE_DBA_FLOAT,
    SQLITE_DBA_TEXT,
    SQLITE_DBA_BLOB,
    SQLITE_DBA_NULL,
    SQLITE_DATA_TYPES_COUNT
};

typedef struct {
    char* column_name;
    char* value;
    enum SQLITE_DATA_TYPES colum_type; 
} table_field_node;

typedef struct {
    char* table_name;
    table_field_node* columns;
    size_t column_count;
} table_definition;

typedef struct {
    table_field_node* columns;
    size_t column_count;
    size_t length;
} query_result;

/**
 * The `sqlite_database_administrator` struct is used to manage the connections and
 * queries to an SQLite database located in `database_path`.
 */
typedef struct {
    sqlite3* database;
    sqlite3_stmt* statement;
    char* database_path;
} sqlite_database_administrator;

/**
* Open the connection to an SQLite database.
*
* The `sqlite_dba_connect_to_db` function creates an
* `sqlite_database_administrator` and either loads the database specified by
* `database_path` or creates it in case that it does not exist.
*
* @param `path` The absolute path to the database to manage.
* @returns a pointer to a `sqlite_database_administrator` struct.
*/
sqlite_database_administrator* sqlite_dba_connect_to_db(const char* path);


/**
 * Closes the connection to an SQLite database.
 *
 * The `sqlite_dba_disconnect_from_db` function closes the connection to the
 * SQLite database file defined in `database_path` as well as freeing the
 * struct and all its members.
 *
 * @param `dba` a pointer to the `sqlite_database_administrator` stuct to be
 * freed.
 */
bool sqlite_dba_disconnect_from_db(sqlite_database_administrator* dba);


/**
 * Prepare a SQLite query.
 *
 * Turn a query string into a `sqlite3_stmnt` in order to execute it
 * eventually. The created statement is stored in the `statement`
 * member.
 *
 * The function checks if there were any problems preparing the
 * statement. If there were, the appropriate error message is stored
 * in the `status_message` member.
 *
 * @param `dba` A pointer to the `sqlite_database_administrator` struct
 * containing the connection to the database.
 * @param `query` The query string to be prepared.
 * @returns A `boolean` indicating if the statement was able to be
 * prepared.
 */
bool sqlite_dba_prepare_statement(sqlite_database_administrator* dba, const char* query);


/**
 * Executes an SQLite prepared statement.
 *
 * If a `query_result*` is returned, it needs to be freed by the `sqlite_dba_query_result_free`
 * function in order to free its resources.
 *
 * @param `dba` The pointer to the `sqlite_database_administrator` struct
 * containing the connection to the database.
 * @returns a pointer to a `query_result` struct.
 */
query_result* sqlite_dba_execute_statement(sqlite_database_administrator* dba);

/**
 * Inserts an item to a table.
 *
 * If a `query_result*` is returned, it needs to be freed by the `sqlite_dba_query_result_free`
 * function in order to free its resources.
 *
 * @param `dba` The pointer to the `sqlite_database_administrator` struct.
 * @param `table_name` The name of the table to modify.
 * @param `values` A pointer to the list of values (columns) to be inserted.
 * @param `values_length` The number of values (columns) to be inserted.
 * @returns a pointer to a `query_result` struct.
 */
query_result* sqlite_dba_insert_item(sqlite_database_administrator* dba, const char* table_name, const table_field_node* values, const size_t values_length);

/**
 * Fetches items from table.
 *
 * If a `query_result*` is returned, it needs to be freed by the `sqlite_dba_query_result_free`
 * function in order to free its resources.
 *
 * @param `dba` The pointer to the `sqlite_database_administrator` struct.
 * @param `table_name` The name of the table to modify.
 * @param `fields` A pointer to the list of columns to be fetched. If `fields` is `NULL` the query will return all the columns in the table.
 * @param `fields_length` The number of columns to be fetched.
 * @param `filters` A pointer to a list of `query_filter_node` structs to filter the table.
 * @returns a pointer to a `query_result` struct.
 */
query_result* sqlite_dba_fetch_items(sqlite_database_administrator* dba, const char* table_name, const table_field_node* fields, const size_t fields_length, const query_filter_node* filters);

/**
 * Updates an item in a table.
 *
 * If a `query_result*` is returned, it needs to be freed by the `sqlite_dba_query_result_free`
 * function in order to free its resources.
 *
 * @param `dba` The pointer to the `sqlite_database_administrator` struct.
 * @param `table_name` The name of the table to modify.
 * @param `values` A pointer to the list of values (columns) to be updated.
 * @param `values_length` The number of values (columns) to be updated.
 * @param `filters` A pointer to a list of `query_filter_node` structs to filter the table.
 * @returns a pointer to a `query_result` struct.
 */
query_result* sqlite_dba_update_item(sqlite_database_administrator* dba, const char* table_name, const table_field_node* values, const size_t values_length, const query_filter_node* filters);

/**
 * Deletes an item from a table.
 *
 * If a `query_result*` is returned, it needs to be freed by the `sqlite_dba_query_result_free`
 * function in order to free its resources.
 *
 * @param `dba` The pointer to the `sqlite_database_administrator` struct.
 * @param `table_name` The name of the table to modify.
 * @param `filters` A pointer to a list of `query_filter_node` structs to filter the table.
 * @returns a pointer to a `query_result` struct.
 */
query_result* sqlite_dba_delete_item(sqlite_database_administrator* dba, const char* table_name, const query_filter_node* filters);

/**
 * Checks if a table is present in the database.
 *
 * @param `dba` The pointer to the `sqlite_database_administrator` struct.
 * @param `table_name` The name of the table to be checked.
 * @returns a `boolean` indicating if the table is present or not.
 */
bool sqlite_dba_check_if_table_exists(sqlite_database_administrator *dba, const char* table_name);

void sqlite_dba_query_result_free(query_result *results);
#endif
