#include "Database.hpp"
#include <sstream>

Database::Database(std::string path) {
    databasePath = path;
    
    int successfulConnection = sqlite3_open_v2(
        path.c_str(),
        &database,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
        , NULL
    );

    if (successfulConnection != SQLITE_OK) {
        database = NULL;

        std::stringstream messageBuilder;
        messageBuilder
            << "An error occurred while trying to open the specified database."
            << "\nDatabase path: " << databasePath
            << "\nError code: " << successfulConnection
        ;
        errorMessage = messageBuilder.str();
    };
}

Database::~Database() {
    int closedCorrectly = sqlite3_close(database);

    if (closedCorrectly != SQLITE_OK) {
        std::stringstream messageBuilder;
        messageBuilder
            << "An error occurred while trying to close the specified database."
            << "\nDatabase path: " << databasePath
            << "\nError code: " << closedCorrectly
        ;
        errorMessage = messageBuilder.str();
        return;
    }

    database = NULL;
}

bool Database::prepareStatement(std::string query, std::string method) {
    int statementCreated = sqlite3_prepare_v2(
        database,
        query.c_str(),
        query.size() + 1,
        &statement,
        NULL
    );

    if (statementCreated != SQLITE_OK) {
        std::stringstream messageBuilder;
        messageBuilder
            << method << ": "
            << "An error occurred while trying to prepare the statement."
            << "\nError code: " << statementCreated
        ;
        errorMessage = messageBuilder.str();

        return false;
    }

    return true;
}

bool Database::executeStatement() {
    if (statement == NULL) {
        std::stringstream messageBuilder;
        messageBuilder
            << "Database::executeStatement: No statement to execute.";
        errorMessage = messageBuilder.str();
        return false;
    }

    queryResults.clear();

    int step;
    size_t columnCount;
    std::map<std::string, std::string> item;

    while ((step = sqlite3_step(statement) != SQLITE_DONE)) {
        columnCount = sqlite3_column_count(statement);

        item.clear();
        for (size_t i = 0; i < columnCount; i++) {
            item.emplace(
                sqlite3_column_name(statement, i),
                sqlite3_column_text(statement, i)
            );
        }
        queryResults.push_back(item);
    }

    int queryFinalized = sqlite3_finalize(statement);
    if (queryFinalized != SQLITE_OK) {
        std::stringstream messageBuilder;
        messageBuilder
            << "Database::executeStatement: An error occurred while finalizing statement."
            << "\nError code: " << queryFinalized
        ;
        errorMessage = messageBuilder.str();
        return false;
    }

    return true;
}

bool Database::insertItem (
    std::string tableName,
    std::map<std::string, std::string> values
) {
    errorMessage = "";
    std::stringstream query, fields, fieldValues;
    
    for (auto it = values.begin(); it != values.end(); ++it) {
        if (!fields.str().empty())
            fields << ", ";

        if (!fieldValues.str().empty())
            fieldValues << ", ";

        fields << it->first;
        fieldValues << it->second;
    }

    query
        << "INSERT INTO " << tableName << "(" << fields.str() << ") VALUES ("
        << fieldValues.str() << ");"
    ;

    bool statementCreated
        = prepareStatement(query.str(), "Database::insertItem");

    if (!statementCreated)
        return false;

    bool queryExecuted = executeStatement();

    if (!queryExecuted)
        return false;

    return true;
}

std::vector<std::map<std::string, std::string>> Database::getItems (
) {
}
