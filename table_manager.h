#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H
#include "manager.h"
#include "table_operations.h"

class TableManager : Manager{
public:
    static void createTable(const CreateTableOperation* operation);
    static void showTables();
    static void dropTable(const DropTableOperation* operation);

    static void addColumn(AddColumnOperation* operation);
    static void dropColumn(DropColumnOperation* operation);
    static void modifyColumn(ModifyColumnOperation* operation);
    static void describeTable(const DescribeTableOperation* operation);
};

#endif // TABLE_MANAGER_H
