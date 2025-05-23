#ifndef TUPLE_MANAGER_H
#define TUPLE_MANAGER_H

#include "manager.h"
#include "structures.h"
#include <vector>
#include <stdexcept>
#include "tuple_operations.h"


class TupleManager : public Manager {
public:
    // 插入记录
    static void insert(const InsertOperation* op);
    // 查询所有记录
    static void selectAll(const SelectAllOperation* operation);
    // 删除记录
    static void deleteRows(const DeleteOperation* op);
    // 更新记录
    static void update(const UpdateOperation* op);
    // 查询部分记录
    static void selectColumns(const SelectColumnsOperation* op);
private:
    int compareFieldValues(const FieldValue& v1, const FieldValue& v2) const;
    FieldValue calculateFunction(
        const FieldBlock& funcField,
        const std::vector<FieldBlock>& fields,
        const std::vector<DataRow>& rows
        );
    QString formatValue(const FieldValue& val, DataType type);
    void handleAggregateSelect(
        const SelectColumnsOperation* op,
        const std::vector<FieldBlock>& allFields,
        const std::vector<FieldBlock>& virtualFields,
        const std::vector<DataRow>& rows);
    QString formatFunctionValue(const FieldValue& val);
    QString formatFieldValue(const FieldValue& val);
    void handleRegularSelect(
        const SelectColumnsOperation* op,
        const std::vector<FieldBlock>& fields,
        const std::vector<DataRow>& rows,
        const std::vector<int>& columnIndices);
    bool matchesWhereConditions(const DataRow& row,
                                const std::vector<FieldBlock>& fields,
                                const std::vector<Condition>& conditions) const;
};

#endif // TUPLE_MANAGER_H
