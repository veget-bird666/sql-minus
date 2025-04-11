#ifndef TABLE_OPERATIONS_H
#define TABLE_OPERATIONS_H
#include "operation.h"
#include "structures.h"
#include <vector>
#include <QString>
using namespace std;

class TableOperation : public Operation {

};


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



#endif // TABLE_OPERATIONS_H
