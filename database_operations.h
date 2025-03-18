#ifndef DATABASE_OPERATIONS_H
#define DATABASE_OPERATIONS_H
#include <QString>

#include "operation.h"

// 数据库操作基类（可选）
class DatabaseOperation : public Operation {
public:
    explicit DatabaseOperation(){}
    explicit DatabaseOperation(const QString& dbName) : dbName(dbName) {}
    QString dbName;
};



// 创建数据库子类
class CreateDatabaseOperation : public DatabaseOperation {
public:
    explicit CreateDatabaseOperation(const QString& dbName) : DatabaseOperation(dbName) {}
    QString getDbName();
    void execute() override;
};

// 删除数据库子类
class DropDatabaseOperation : public DatabaseOperation {
public:
    explicit DropDatabaseOperation(const QString& dbName) : DatabaseOperation(dbName) {}
    void execute() override;
};

// 显示所有数据库子类
class ShowDatabasesOperation : public DatabaseOperation {
public:
    explicit ShowDatabasesOperation() :DatabaseOperation() {}
    void execute() override;
};


#endif // DATABASE_OPERATIONS_H
