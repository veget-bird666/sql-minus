#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H
#include <QString>

class DatabaseManager {
public:
    static void createDatabase(const QString& dbName);
};


#endif // DATABASE_MANAGER_H
