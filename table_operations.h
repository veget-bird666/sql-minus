#ifndef TABLE_OPERATIONS_H
#define TABLE_OPERATIONS_H
#include "operation.h"
#include "structures.h"
#include <vector>
#include <QString>
using namespace std;

class TableOperation : public Operation {

};

// 创建表的操作类
class CreateTableOperation : public TableOperation {
private:
    void extractIntegrityConstraints(const FieldBlock& field);
    IntegrityConstraint convertCheckConditionToConstraint(const FieldBlock& field, const char* constraintName = nullptr);
public:
    TableBlock table_block;                     // 表定义
    vector<FieldBlock> field_blocks;            // 字段
    vector<IntegrityConstraint> constraints;  // 新增完整性约束
    vector<IndexBlock> indexes;               // 新增索引
    QString dbName;                             // 所在数据库名称

    CreateTableOperation(QString, QList<FieldBlock>);
    void execute() override;
};

// 显示所有表子类
class ShowTablesOperation : public TableOperation {
public:
    explicit ShowTablesOperation() : TableOperation() {}
    void execute() override;
};


// 删除表的操作类
class DropTableOperation : public TableOperation {
public:
    QString dbName;
    QString tableName;

    explicit DropTableOperation(const QString& dbName, const QString& tableName)
        : dbName(dbName), tableName(tableName) {}

    void execute() override ;
};


// 添加字段操作类
class AddColumnOperation : public TableOperation {
public:
    QString dbName;
    QString tableName;
    FieldBlock newField;

    explicit AddColumnOperation(const QString& db, const QString& table, const FieldBlock& field)
        : dbName(db), tableName(table), newField(field) {}

    void execute() override;
};

// 删除字段操作类
class DropColumnOperation : public TableOperation {
public:
    QString dbName;
    QString tableName;
    QString columnName;

    explicit DropColumnOperation(const QString& db, const QString& table, const QString& column)
        : dbName(db), tableName(table), columnName(column) {}

    void execute() override;
};

// 修改字段操作类
class ModifyColumnOperation : public TableOperation {
public:
    QString dbName;
    QString tableName;
    QString oldColumnName;
    FieldBlock newField;

    explicit ModifyColumnOperation(const QString& db, const QString& table,
                                   const QString& oldName, const FieldBlock& field)
        : dbName(db), tableName(table), oldColumnName(oldName), newField(field) {}

    void execute() override;
};

// 描述表的操作类
class DescribeTableOperation : public TableOperation {
public:
    QString dbName;
    QString tableName;
    explicit DescribeTableOperation(const QString& dbName, const QString& tableName)
        : dbName(dbName), tableName(tableName) {}
    void execute() override;
};



#endif // TABLE_OPERATIONS_H
