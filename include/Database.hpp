#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <map>
#include <sqlite3.h>
#include <string>
#include <vector>

class Database {
    private:
        sqlite3 *database;
        sqlite3_stmt *statement;
        std::string databasePath, errorMessage;
        std::vector<std::map<std::string, std::string>> queryResults;

    public:
        Database(std::string databasePath);
        ~Database();

        std::string getErrorMessage();

        /**
         * Prepare a SQLite query.
         *
         * Turn a query string into a `sqlite3_stmnt` in order to execute it
         * eventually. The created statement is stored in the `statement`
         * attribute.
         *
         * The function checks if there where any problems preparing the
         * statement. If there were, the appropriate error message is stored
         * in the `errorMessage` attribute.
         *
         * @param query The query string to be prepared.
         * @param method The name of the (inner) function that called this
         * function.
         * @returns A boolean indicating if the statement was able to be
         * prepared.
         */
        bool prepareStatement (
            std::string query,
            std::string method
        );

        bool executeStatement();

        bool insertItem (
            std::string tableName,
            std::map<std::string, std::string> values
        );

        std::vector<std::map<std::string, std::string>> getItems (
            std::string tableName,
            std::map<std::string, std::vector<std::string>> filters
        );

        bool updateItem (
            std::string tableName,
            int id,
            std::map<std::string, std::string> values
        );

        bool deleteItem (
            std::string tableName,
            int id
        );
};
#endif
