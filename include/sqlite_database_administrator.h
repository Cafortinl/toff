/**
 * File - sqlite_database_administrator.h
 * Author - C. Fortín (cafortinl)
 *
 * Manage connections and queries to SQLite databases.
 */
#ifdef __cplusplus
extern "C" {
#endif

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

enum QUERY_FILTER_STATES {
    QF_STATE_INIT,
    QF_STATE_LEVAL,
    QF_STATE_LPRINT,
    QF_STATE_OPRINT,
    QF_STATE_REVAL,
    QF_STATE_RPRINT,
    QF_STATE_FINAL,
    QF_STATE_COUNT
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

// Section: Table Information

//Column Constraint Constants
#define CC_PRIMARY_KEY 0x1
#define CC_FOREIGN_KEY 0x2
#define CC_UNIQUE 0x4
#define CC_DEFAULT 0x8
#define CC_COLLATION 0x10
#define CC_ON_DELETE 0x20
#define CC_CASCADE 0x40

//Column Constraint Functions
#define CC_IS_PRIMARY_KEY(x) (x & CC_PRIMARY_KEY)
#define CC_IS_FOREIGN_KEY(x) (x & CC_FOREIGN_KEY)
#define CC_IS_UNIQUE(x)      (x & CC_UNIQUE)
#define CC_IS_DEFAULT(x)     (x & CC_DEFAULT)
#define CC_IS_COLLATION(x)   (x & CC_COLLATION)
#define CC_IS_ON_DELETE(x)   (x & CC_ON_DELETE)
#define CC_IS_CASCADE(x)     (x & CC_CASCADE)

enum SQLITE_DATA_TYPES {
    SQLITE_DBA_INTEGER,
    SQLITE_DBA_FLOAT,
    SQLITE_DBA_TEXT,
    SQLITE_DBA_BLOB,
    SQLITE_DBA_NULL,
    SQLITE_DATA_TYPES_COUNT
};

/*
 * Create a constraint for a specific table column.
 *
 * The available constraints include:
 * Primary key constraints - Set by using the `CC_PRIMARY_KEY` macro.
 * Foreign key constraints - Set by using the `CC_FOREIGN_KEY_KEY` macro.
 * On delete behaviour - Set by using the `CC_ON_DELETE` macro. Default value `DO NOTHING`.
 * Cascade behaviour - Set by using the `CC_CASCADE` macro.
 * Unique constraints - Set by using the `CC_UNIQUE` macro.
 * Default constraints - Set by using the `CC_DEFAULT` macro.
 * Collation constraints - Set by using the `CC_COLLATION` macro.
 *
 * To evaluate the different constraints the different `CC_IS_XXXX` macros are used, where
 * `XXXX` is replaced by the different constraint macros (e.g. `PRIMARY_KEY`, `DEFAULT`, etc).
 */
typedef struct {
    /*
     * bit 1: is_primary_key    - Mask: 0x1
     * bit 2: is_foreign_key    - Mask: 0x2
     * bit 3: is_unique         - Mask: 0x4
     * bit 4: has_default_value - Mask: 0x8
     * bit 5: has_collation     - Mask: 0x10
     * bit 6: has_ond_delete    - Mask: 0x20
     * bit 7: cascade           - Mask: 0x40
     */
    uint8_t constraint_information;
    char* value;
} column_constraints;

typedef struct {
    enum SQLITE_DATA_TYPES type;
    union {
        int integer_value;
        double float_value;
        char* text_value;
        struct {
            const void* data;
            size_t size;
        } blob_value;
    } as;
} column_information;

typedef struct {
    char* column_name;
    column_information value;
    column_constraints* constraints;
} table_field_node;

typedef struct {
    char* table_name;
    table_field_node* columns;
    size_t column_count;
} table_definition;

// EndSection: Table Information

// Section: DBA
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

/**
 * Frees the resources allocated by the `sqlite_dba_execute_statement` function.
 *
 * @param `results` a pointer to the list of `query_result` structs to be freed.
 */
void sqlite_dba_query_result_free(query_result *results);

/**
 * Creates a table in the working database.
 *
 * The `sqlite_dba_create_table` function utilizes the information found in a `table_definition`
 * struct to generate the code needed for a table creation, including (some) constraints.
 *
 * The creation of a `table_definition` struct involves `table_field_node` to define the different
 * columns as well as `column_constraints` (when it applies) to set the constraints needed by the
 * different functions.
 *
 * When the table already exists the function returns false.
 *
 * @param `dba` The pointer to the `sqlite_database_administrator` struct.
 * @param `table` a pointer to a `table_definition` struct, defining the table to be created.
 * @returns a `boolean` indicating if the table was able to be created or not.
 *
 * @see `sqlite_dba_check_if_table_exists`
 * @see `table_definition`
 * @see `table_field_node`
 * @see `column_constraints`
 */
bool sqlite_dba_create_table(sqlite_database_administrator *dba, const table_definition *table);
// EndSection: DBA
#endif

#ifdef __cplusplus
}
#endif
