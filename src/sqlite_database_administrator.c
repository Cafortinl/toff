#include "sqlite_database_administrator.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char* operation_to_string(enum QUERY_FILTER_OPERATIONS operation) {
    switch (operation) {
        case QF_NOT:
            return (char*) "NOT";
        case QF_AND:
            return (char*) "AND";
        case QF_OR:
            return (char*) "OR";
        case QF_LT:
            return (char*) "<";
        case QF_LTE:
            return (char*) "<=";
        case QF_GT:
            return (char*) ">";
        case QF_GTE:
            return (char*) ">=";
        case QF_BETWEEN:
            return (char*) "BETWEEN";
        case QF_LIKE:
            return (char*) "LIKE";
        case QF_EQUALS:
            return (char*) "=";
        default:
          return (char*) "INVALID";
    }
}

char* data_type_to_string(enum SQLITE_DATA_TYPES data_type) {
    switch (data_type) {
        case SQLITE_DBA_INTEGER:
            return (char*) "INTEGER";
        case SQLITE_DBA_FLOAT:
            return (char*) "REAL";
        case SQLITE_DBA_TEXT:
            return (char*) "TEXT";
        case SQLITE_DBA_BLOB:
            return (char*) "BLOB";
        case SQLITE_DBA_NULL:
            return (char*) "NULL";
        default:
            return (char*) "INVALID";
    }
}

/**
 * Generates an `sqlite3_str` object containing a DFS of the sent
 * `query_filter_node` objects in order to generate the query filter
 * conditions.
 *
 * @param `filters` a pointer to a `query_filter_node` struct representing
 * the different conditions for the query.
 * @param `str_builder` a pointer to the `sqlite3_str` struct in which the
 * condition string will be built.
 */
void query_filter_to_string_builder(
    const query_filter_node *filters,
    sqlite3_str *str_builder
) {
    int 
        left_type = filters->node_types >> 4,
        right_type = filters->node_types & 15
    ;

    if (left_type == QF_NODE)
        query_filter_to_string_builder(filters->left.node, str_builder);

    if (left_type == QF_NONE)
        sqlite3_str_appendf(
            str_builder,
            " %s ",
            operation_to_string(filters->operation)
        );
    else
        sqlite3_str_appendf(
            str_builder,
            " %s %s",
            filters->left.text,
            operation_to_string(filters->operation)
        );

    if (right_type == QF_NODE)
        query_filter_to_string_builder(filters->right.node, str_builder);

    if (right_type == QF_TEXT)
        sqlite3_str_appendf( str_builder, " %s", filters->right.text);
}

char* sqlite_dba_query_filter_to_str(
    const sqlite_database_administrator *dba,
    const query_filter_node *filter
) {
    sqlite3_str *str_builder = sqlite3_str_new(dba->database);
    query_filter_to_string_builder(filter, str_builder);
    return sqlite3_str_finish(str_builder);
}

void sqlite_dba_query_result_free(query_result *results) {
    for (size_t i = 0; i < results->length; ++i) {
        free(results->columns[i].column_name);
        free(results->columns[i].value);
    }

    results->columns = NULL;
    results->length = 0;
    results->column_count = 0;

    free(results);
}

sqlite_database_administrator* sqlite_dba_connect_to_db(const char* path) {
    sqlite_database_administrator *dba = 
        (sqlite_database_administrator*) malloc(sizeof(sqlite_database_administrator));
    sqlite3 *database;

    int successfull_connection = sqlite3_open_v2(
        path,
        &database,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
        NULL
    );

    if (successfull_connection != SQLITE_OK) {
        database = NULL;
        fprintf(
            stderr,
            "An error occurred while trying to open the specified database.\nDatabase path: %s\nError code: %d\n",
            path,
            successfull_connection
        );
    }

    dba->database = database;
    dba->database_path = (char*) path;

    return dba;
}

bool sqlite_dba_disconnect_from_db(sqlite_database_administrator *dba) {
    if (!dba) {
        fprintf(stderr, "sqlite_dba_disconnect_from_db - Error: no valid sqlite_database_administrator struct was provided.\n");
        return false;
    }

    if (!dba->database) {
        fprintf(stderr, "sqlite_dba_disconnect_from_db - Error: no valid database connection.\n");
        return false;
    }

    int closed_correctly = sqlite3_close(dba->database);

    if (closed_correctly != SQLITE_OK) {
        fprintf(
            stderr,
            "An error occurred while trying to close the specified database.\nDatabase path: %s\nError code: %d\n",
            dba->database_path,
            closed_correctly
        );

        return false;
    }

    dba->database = NULL;
    dba->statement = NULL;
    dba->database_path = NULL;

    return true;
}

bool sqlite_dba_prepare_statement(
    sqlite_database_administrator* dba,
    const char* query
) {
    if (!dba) {
        fprintf(stderr, "sqlite_dba_prepare_statement - Error: no valid sqlite_database_administrator struct was provided.\n");
        return false;
    }

    if (!dba->database) {
        fprintf(stderr, "sqlite_dba_prepare_statement - Error: no valid database connection.\n");
        return false;
    }

    int statement_created = sqlite3_prepare_v2(
        dba->database,
        query,
        strlen(query) + 1,
        &(dba->statement),
        NULL
    );

    if (statement_created != SQLITE_OK) {
        fprintf(
            stderr,
            "An error occurred while trying to prepare the statement.\nStatement: %s\nError code: %d\n",
            query,
            statement_created
        );
        
        sqlite3_finalize(dba->statement);
        dba->statement = NULL;
        return false;
    }

    return true;
}

query_result* sqlite_dba_execute_statement(sqlite_database_administrator* dba) {
    if (!dba) {
        fprintf(stderr, "sqlite_dba_execute_statement - Error: no valid sqlite_database_administrator struct was provided.\n");
        return NULL;
    }

    if (!dba->database) {
        fprintf(stderr, "sqlite_dba_execute_statement - Error: no valid database connection.\n");
        return NULL;
    }

    if (!dba->statement) {
        fprintf(stderr, "sqlite_dba_execute_statement: No statement to execute.");
        return NULL;
    }

    size_t column_count = sqlite3_column_count(dba->statement);
    size_t query_results_capacity = 256;

    query_result *results = (query_result*) malloc(sizeof(query_result));

    results->column_count = column_count;
    results->columns = (table_field_node*) malloc(sizeof(table_field_node) * column_count * query_results_capacity);

    int step, index = 0;
    while ((step = sqlite3_step(dba->statement)) != SQLITE_DONE) {
        for (size_t i = 0; i < column_count; ++i) {
            if (index == (int) query_results_capacity - 1) {
                query_results_capacity *= 2;
                results->columns = (table_field_node*) realloc(results->columns, sizeof(table_field_node) * query_results_capacity);
            }
            char* column_name = (char*) sqlite3_column_name(dba->statement, i);
            char* value = (char*) sqlite3_column_text(dba->statement, i);

            if (!value)
                value = (char*) "";

            //SQLite Fundamental Datatypes = enum SQLITE_DATA_TYPES + 1
            //https://sqlite.org/c3ref/c_blob.html
            results->columns[index].column_type = (enum SQLITE_DATA_TYPES) (sqlite3_column_type(dba->statement, i) + 1);

            results->columns[index].column_name = (char*) calloc(strlen(column_name) + 1, sizeof(char));
            memcpy(results->columns[index].column_name, column_name, strlen(column_name) + 1);

            results->columns[index].value = (char*) calloc(strlen(value) + 1, sizeof(char));
            memcpy(results->columns[index].value, value, strlen(value) + 1);
            ++index;
        }
    }
    results->length = index;

    int query_finalized = sqlite3_finalize(dba->statement);
    if (query_finalized != SQLITE_OK) {
        sqlite_dba_query_result_free(results);

        fprintf(
            stderr,
            "sqlite_dba_execute_statement: An error occurred while finalizing the statement.\nError code: %d\n",
            query_finalized
        );
        return NULL;
    }

    return results;
}

query_result* sqlite_dba_insert_item(
    sqlite_database_administrator *dba,
    const char* table_name,
    const table_field_node* values,
    const size_t values_length
) {
    if (!dba) {
        fprintf(stderr, "sqlite_dba_insert_item - Error: no valid sqlite_database_administrator struct was provided.\n");
        return NULL;
    }

    if (!dba->database) {
        fprintf(stderr, "sqlite_dba_insert_item - Error: no valid database connection.\n");
        return NULL;
    }

    if (!table_name) {
        fprintf(stderr, "sqlite_dba_insert_item - Error: no table name was given.\n");
        return NULL;
    }

    if (!values) {
        fprintf(stderr, "sqlite_dba_insert_item - Error: no values were given.\n");
        return NULL;
    }

    if (values && values_length <= 0) {
        fprintf(stderr, "sqlite_dba_insert_item - Error: the number of values to insert was not given.\n");
        return NULL;
    }

    sqlite3_str *query_builder = sqlite3_str_new(dba->database);
    sqlite3_str *fields_builder = sqlite3_str_new(dba->database);
    sqlite3_str *field_values_builder = sqlite3_str_new(dba->database);

    for (size_t i = 0; i < values_length; ++i) {
        sqlite3_str_appendf(
            fields_builder,
            "%s%s",
            sqlite3_str_length(fields_builder) == 0 ? "" : ", ",
            values[i].column_name
        );

        sqlite3_str_appendf(
            field_values_builder,
            "%s%s",
            sqlite3_str_length(field_values_builder) == 0 ? "" : ", ",
            values[i].value
        );
    }
    
    char *query, *fields, *field_values;
    fields = sqlite3_str_finish(fields_builder);
    field_values = sqlite3_str_finish(field_values_builder);

    sqlite3_str_appendf(
        query_builder,
        "INSERT INTO %s (%s) VALUES (%s);",
        table_name,
        fields,
        field_values
    );
    sqlite3_free(fields);
    sqlite3_free(field_values);

    query = sqlite3_str_finish(query_builder);
    bool statement_prepared = sqlite_dba_prepare_statement(dba, query);

    sqlite3_free(query);

    if (!statement_prepared)
        return NULL;

    query_result *results = sqlite_dba_execute_statement(dba);

    if (!results)
        return NULL;

    return results;
}

query_result* sqlite_dba_fetch_items(
    sqlite_database_administrator* dba,
    const char* table_name,
    const table_field_node* fields,
    const size_t fields_length,
    const query_filter_node* filters
) {
    if (!dba) {
        fprintf(stderr, "sqlite_dba_fetch_items - Error: no valid sqlite_database_administrator struct was provided.\n");
        return NULL;
    }

    if (!dba->database) {
        fprintf(stderr, "sqlite_dba_fetch_items - Error: no valid database connection.\n");
        return NULL;
    }

    if (!table_name) {
        fprintf(stderr, "sqlite_dba_fetch_items - Error: no table name was given.\n");
        return NULL;
    }

    char *columns, *where_clauses, *query;
    sqlite3_str *query_builder = sqlite3_str_new(dba->database);
    sqlite3_str *columns_builder = sqlite3_str_new(dba->database);

    for (size_t i = 0; i < fields_length; ++i) {
        sqlite3_str_appendf(
            columns_builder,
            "%s%s",
            sqlite3_str_length(columns_builder) == 0 ? "" : ", ",
            fields[i].value
        );
    }

    columns = sqlite3_str_length(columns_builder) > 0 ? sqlite3_str_finish(columns_builder) : (char*) "*";

    if (filters) {
        where_clauses = sqlite_dba_query_filter_to_str(dba, filters);
        sqlite3_str_appendf(
            query_builder,
            "SELECT %s FROM %s WHERE %s;",
            columns,
            table_name,
            where_clauses
        );
        sqlite3_free(where_clauses);
    } else {
        sqlite3_str_appendf(
            query_builder,
            "SELECT %s FROM %s;",
            columns,
            table_name
        );
    }

    if (strcmp(columns, "*") != 0)
        sqlite3_free(columns);

    query = sqlite3_str_finish(query_builder);

    bool statement_created = sqlite_dba_prepare_statement(dba, query);
    sqlite3_free(query);

    if (!statement_created)
        return NULL;

    query_result *results = sqlite_dba_execute_statement(dba);

    if (!results)
        return NULL;

    return results;
}

query_result* sqlite_dba_update_item(
    sqlite_database_administrator* dba,
    const char* table_name,
    const table_field_node* values,
    const size_t values_length,
    const query_filter_node* filters
) {
    if (!dba) {
        fprintf(stderr, "sqlite_dba_update_item - Error: no valid sqlite_database_administrator struct was provided.\n");
        return NULL;
    }

    if (!dba->database) {
        fprintf(stderr, "sqlite_dba_update_item - Error: no valid database connection.\n");
        return NULL;
    }

    if (!table_name) {
        fprintf(stderr, "sqlite_dba_update_item - Error: no table name was given.\n");
        return NULL;
    }

    if (!values) {
        fprintf(stderr, "sqlite_dba_update_item - Error: no fields were defined.\n");
        return NULL;
    }

    sqlite3_str
        *query_builder = sqlite3_str_new(dba->database),
        *field_updates_builder = sqlite3_str_new(dba->database)
    ;
    
    char *query, *field_updates, *where_clauses;

    for (size_t i = 0; i < values_length; ++i) {
        sqlite3_str_appendf(
            field_updates_builder,
            "%s%s=%s",
            sqlite3_str_length(field_updates_builder) == 0 ? "" : ", ",
            values[i].column_name,
            values[i].value
        );
    }
    field_updates = sqlite3_str_finish(field_updates_builder);

    if (filters) {
        where_clauses = sqlite_dba_query_filter_to_str(dba, filters);
        sqlite3_str_appendf(
            query_builder,
            "UPDATE %s SET %s WHERE %s;",
            table_name,
            field_updates,
            where_clauses
        );
        sqlite3_free(where_clauses);
    } else {
        sqlite3_str_appendf(
            query_builder,
            "UPDATE %s SET %s;",
            table_name,
            field_updates
        );
    }
    query = sqlite3_str_finish(query_builder);

    bool statement_created = sqlite_dba_prepare_statement(dba, query);
    sqlite3_free(query);

    if (!statement_created)
        return NULL;

    query_result *results = sqlite_dba_execute_statement(dba);

    if (!results)
        return NULL;

    return results;
}

query_result* sqlite_dba_delete_item(
    sqlite_database_administrator* dba,
    const char* table_name,
    const query_filter_node* filters
) {
    if (!dba) {
        fprintf(stderr, "sqlite_dba_delete_item - Error: no valid sqlite_database_administrator struct was provided.\n");
        return NULL;
    }

    if (!dba->database) {
        fprintf(stderr, "sqlite_dba_delete_item - Error: no valid database connection.\n");
        return NULL;
    }

    if (!table_name) {
        fprintf(stderr, "sqlite_dba_delete_item - Error: no table name was given.\n");
        return NULL;
    }

    sqlite3_str *query_builder = sqlite3_str_new(dba->database);
    char *query, *where_clauses;

    if (filters) {
        where_clauses = sqlite_dba_query_filter_to_str(dba, filters);
        sqlite3_str_appendf(
            query_builder,
            "DELETE FROM %s WHERE %s;",
            table_name,
            where_clauses
        );
        sqlite3_free(where_clauses);
    } else {
        sqlite3_str_appendf(
            query_builder,
            "DELETE FROM %s;",
            table_name
        );
    }
    query = sqlite3_str_finish(query_builder);

    bool statement_created = sqlite_dba_prepare_statement(dba, query);
    sqlite3_free(query);

    if (!statement_created)
        return NULL;

    query_result *results = sqlite_dba_execute_statement(dba);

    if (!results)
        return NULL;

    return results;
}

bool sqlite_dba_check_if_table_exists(sqlite_database_administrator *dba, const char* table_name) {
    if (!dba) {
        fprintf(stderr, "sqlite_dba_check_if_table_exists - Error: no valid sqlite_database_administrator struct was provided.\n");
        return false;
    }

    if (!dba->database) {
        fprintf(stderr, "sqlite_dba_check_if_table_exists - Error: no valid database connection.\n");
        return false;
    }

    if (!table_name) {
        fprintf(stderr, "sqlite_dba_check_if_table_exists - Error: no table name was given.\n");
        return false;
    }

    sqlite3_str *query_builder = sqlite3_str_new(dba->database);
    sqlite3_str_appendf(
        query_builder,
        "SELECT name FROM sqlite_master WHERE type='table' and name='%s';",
        table_name
    );
    char* query = sqlite3_str_finish(query_builder);

    bool query_prepared = sqlite_dba_prepare_statement(dba, query);
    sqlite3_free(query);

    if (!query_prepared)
        return false;

    query_result* results = sqlite_dba_execute_statement(dba);

    if (!results)
        return false;

    int results_length = results->length;
    sqlite_dba_query_result_free(results);

    if (!results_length)
        return false;

    return true;
}

bool sqlite_dba_create_table(sqlite_database_administrator *dba, const table_definition *table) {
    if (!dba) {
        fprintf(stderr, "sqlite_dba_create_table - Error: no valid sqlite_database_administrator struct was provided.\n");
        return false;
    }

    if (!dba->database) {
        fprintf(stderr, "sqlite_dba_create_table - Error: no valid database connection.\n");
        return false;
    }

    if (!table) {
        fprintf(stderr, "sqlite_dba_create_table - Error: no valid table_definition struct was given.\n");
        return false;
    }

    if (sqlite_dba_check_if_table_exists(dba, table->table_name)) {
        fprintf(stderr, "sqlite_dba_create_table - Error: table already exists.\n");
        return false;
    }

    sqlite3_str *query_builder = sqlite3_str_new(dba->database);
    sqlite3_str *inline_constraint_builder;
    sqlite3_str *append_constraint_builder = NULL;
    char* inline_constraints = NULL;
    char* append_constraints = NULL;

    sqlite3_str_appendf(
        query_builder,
        "CREATE TABLE %s (",
        table->table_name
    );

    for (size_t i = 0; i < table->column_count; ++i) {
        table_field_node current_column = table->columns[i];

        if (inline_constraints) {
            sqlite3_free(inline_constraints);
            inline_constraints = NULL;
        }

        if (current_column.constraints) {
            column_constraints current_constraints = *(current_column.constraints);
            inline_constraint_builder = sqlite3_str_new(dba->database);

            if (CC_IS_PRIMARY_KEY(current_constraints.constraint_information)) {
                sqlite3_str_appendf(
                    inline_constraint_builder,
                    " PRIMARY KEY %s",
                    current_constraints.value
                );
            } else if (CC_IS_FOREIGN_KEY(current_constraints.constraint_information)) {
                if (!append_constraint_builder)
                    append_constraint_builder = sqlite3_str_new(dba->database);

                char *constraint_values = malloc(sizeof(char) * (strlen(current_constraints.value) + 1));
                char *referenced_table, *referenced_column;

                strcpy(constraint_values, current_constraints.value);

                referenced_table = strtok(constraint_values, "|");
                referenced_column = strtok(NULL, "|");

                sqlite3_str_appendf(
                    append_constraint_builder,
                    ", FOREIGN KEY (%s) REFERENCES %s(%s)",
                    current_column.column_name,
                    referenced_table,
                    referenced_column
                );
                free(constraint_values);

                if (CC_IS_ON_DELETE(current_constraints.constraint_information)) {
                    sqlite3_str_appendf(
                        append_constraint_builder,
                        " ON DELETE %s",
                        CC_IS_CASCADE(current_constraints.constraint_information) ? "CASCADE" : "NO ACTION"
                    );
                }
            } else if (CC_IS_UNIQUE(current_constraints.constraint_information)) {
                sqlite3_str_appendf(
                    inline_constraint_builder,
                    " UNIQUE"
                );
            } else if (CC_IS_DEFAULT(current_constraints.constraint_information)) {
                sqlite3_str_appendf(
                    inline_constraint_builder,
                    " DEFAULT %s",
                    current_constraints.value
                );
            } else if (CC_IS_COLLATION(current_constraints.constraint_information)) {
                sqlite3_str_appendf(
                    inline_constraint_builder,
                    " COLLATE %s",
                    current_constraints.value
                );
            }

            inline_constraints = sqlite3_str_finish(inline_constraint_builder);
        }

        sqlite3_str_appendf(
            query_builder,
            "%s %s%s%s",
            current_column.column_name,
            data_type_to_string(current_column.column_type),
            inline_constraints ? inline_constraints : "",
            i < (table->column_count - 1) ? ", " : ""
        );
    }

    if (append_constraint_builder)
        append_constraints = sqlite3_str_finish(append_constraint_builder);

    sqlite3_str_appendf(
        query_builder,
        "%s);",
        append_constraints ? append_constraints : ""
    );

    if (append_constraints)
        sqlite3_free(append_constraints);

    char* query = sqlite3_str_finish(query_builder);

    bool statement_prepared = sqlite_dba_prepare_statement(dba, query);
    sqlite3_free(query);

    if (!statement_prepared) {
        fprintf(stderr, "sqlite_dba_create_table - Error: there was an error while trying to prepare the statement. Statement: %s\n", query);
        return false;
    }

    query_result* statement_executed = sqlite_dba_execute_statement(dba);
    if (!statement_executed) {
        fprintf(stderr, "sqlite_dba_create_table - Error: the statement was not able to be executed.\n");
        return false;
    }

    sqlite_dba_query_result_free(statement_executed);

    return true;
}
