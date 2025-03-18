#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H
#include <QString>
#include "database_operations.h"

//
class DatabaseManager {
public:
    static void createDatabase(const CreateDatabaseOperation* operation);
    static void dropDatabase(const DropDatabaseOperation* operation);
    static void showDatabases();

};


#endif // DATABASE_MANAGER_H
