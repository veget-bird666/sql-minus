#ifndef TUPLE_OPERATIONS_H
#define TUPLE_OPERATIONS_H
#include "operation.h"
#include "structures.h"
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

#endif // TUPLE_OPERATIONS_H


