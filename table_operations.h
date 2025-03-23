#ifndef TABLE_OPERATIONS_H
#define TABLE_OPERATIONS_H
#include "operation.h"
#include "structures.h"
#include <vector>
using namespace std;

class TableOperation : public Operation {

};


class CreateTableOperation : public TableOperation {
public:
    TableBlock table_block;                     // 表定义
    vector<FieldBlock> field_blocks;            // 字段
    vector<IntegrityConstraint> constraints;  // 新增完整性约束
    vector<IndexBlock> indexes;               // 新增索引
};


#endif // TABLE_OPERATIONS_H
