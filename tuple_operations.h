#ifndef TUPLE_OPERATIONS_H
#define TUPLE_OPERATIONS_H
#include "operation.h"
#include "structures.h"
#include <QString>
#include <vector>


class TupleOperation : public Operation{

};

// 插入数据操作
class InsertOperation : public TupleOperation {
public:
    QString dbName;          // 目标数据库
    QString tableName;       // 目标表
    std::vector<FieldValue> values; // 按字段顺序存储的值
    void execute() override;
};

// 查找数据操作
class SelectAllOperation : public TupleOperation {
public:
    QString dbName;
    QString tableName;

    SelectAllOperation(const QString& dbName, const QString& tableName)
        : dbName(dbName), tableName(tableName) {}

    void execute() override;
};



class DeleteOperation : public TupleOperation {
public:
    QString dbName;
    QString tableName;
    std::vector<Condition> conditions;

    DeleteOperation(const QString& db, const QString& table, const std::vector<Condition>& conds)
        : dbName(db), tableName(table), conditions(conds) {}

    void execute() override;
};

class SelectColumnsOperation : public TupleOperation {
    public:
        QString dbName;
        QString tableName;
        QStringList columns;  // 要查询的列名列表
        std::vector<Condition> conditions;  // WHERE条件
        
        explicit SelectColumnsOperation(
            const QString& db, 
            const QString& table, 
            const QStringList& cols
        ) : dbName(db), tableName(table), columns(cols) {}
        
        void execute() override;
    };

#endif
