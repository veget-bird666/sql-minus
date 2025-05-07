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

};

#endif // TUPLE_MANAGER_H
