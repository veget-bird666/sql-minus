#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H
#include "manager.h"
#include "table_operations.h"

class TableManager : Manager{
public:
    static void createTable(const CreateTableOperation* operation);
    // static void dropTable(const DropTableOperation* operation);

};

#endif // TABLE_MANAGER_H
