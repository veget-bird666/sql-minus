#include "table_operations.h"
#include "table_manager.h"
#include <QTime>
extern QString currentDB;
// 创建数据库操作
CreateTableOperation::CreateTableOperation(QString tableName, QList<FieldBlock> fields) {
    // 获取表定义文件
    for (FieldBlock fieldBlock : fields) {
        if(fieldBlock.isAggregateFunc) {
            throw std::runtime_error("创建操作中不能有聚合函数");
        }
        field_blocks.push_back(fieldBlock);
        extractIntegrityConstraints(fieldBlock);
        // if (strlen(fieldBlock.ref_field)){
        //     IntegrityConstraint fk;
        //     memset(&fk, 0, sizeof(IntegrityConstraint));
        //     strncpy(fk.name, ("FK_" + tableName + "_" + fieldBlock.ref_table).toUtf8().constData(), 128);
        //     strncpy(fk.field, fieldBlock.name, 128);
        //     fk.type = CT_FOREIGN_KEY;
        //     memcpy(fk.params, fieldBlock.ref_table, 128);
        //     memcpy(fk.params + 128, fieldBlock.ref_field, 128);
        //     constraints.push_back(fk);
        // }
        //fieldBlock.integrities;
    }
    dbName = currentDB;

    // 创建字段
    TableBlock newTableBlock;
    memset(&newTableBlock, 0, sizeof(TableBlock));

    // 填充字段
    strncpy(newTableBlock.name, tableName.toUtf8().constData(), 128);
    newTableBlock.record_num = 0;
    newTableBlock.field_num = fields.size();

    strncpy(newTableBlock.tdf, ("D:/DBMS_ROOT/data/" + dbName + "/" + tableName + ".tdf").toUtf8().constData(), 256);
    strncpy(newTableBlock.trd, ("D:/DBMS_ROOT/data/" + dbName + "/" + tableName + ".trd").toUtf8().constData(), 256);
    strncpy(newTableBlock.tic, ("D:/DBMS_ROOT/data/" + dbName + "/" + tableName + ".tic").toUtf8().constData(), 256);
    strncpy(newTableBlock.tid, ("D:/DBMS_ROOT/data/" + dbName + "/" + tableName + ".tid").toUtf8().constData(), 256);
    newTableBlock.crtime = QDateTime::currentSecsSinceEpoch();
    newTableBlock.mtime = newTableBlock.crtime;
    table_block = newTableBlock;
}

void CreateTableOperation::execute(){
    TableManager::createTable(this);
}

/**
 * 将 FieldBlock 中的 check_condition 转换为 IntegrityConstraint 结构体
 *
 * @param field 包含检查条件的字段块
 * @param constraintName 要分配给约束的名称（可为空）
 * @return 转换后的完整性约束结构体
 */
IntegrityConstraint CreateTableOperation::convertCheckConditionToConstraint(const FieldBlock& field, const char* constraintName) {
    IntegrityConstraint constraint;

    // 设置约束名称（如果未提供则使用默认名称）
    if (constraintName && strlen(constraintName) > 0) {
        strncpy(constraint.name, constraintName, sizeof(constraint.name) - 1);
    } else {
        snprintf(constraint.name, sizeof(constraint.name),
                 "CHECK_%s_%lld", field.name, field.mtime);
    }
    constraint.name[sizeof(constraint.name) - 1] = '\0'; // 确保终止

    // 设置字段名
    strncpy(constraint.field, field.name, sizeof(constraint.field) - 1);
    constraint.field[sizeof(constraint.field) - 1] = '\0';

    // 设置约束类型
    constraint.type = CT_CHECK;

    // 复制检查条件到params
    strncpy(constraint.params, field.check_condition, sizeof(constraint.params) - 1);
    constraint.params[sizeof(constraint.params) - 1] = '\0';

    return constraint;
}

/**
 * 从 FieldBlock 提取所有完整性约束（包括检查约束）
 *
 * @param field 要处理的字段块
 * @return 包含所有完整性约束的QList
 */
void CreateTableOperation::extractIntegrityConstraints(const FieldBlock& field) {


    // 处理检查约束
    if (strlen(field.check_condition) > 0) {
        constraints.push_back(convertCheckConditionToConstraint(field));
    }

    // 这里可以添加处理其他类型约束的逻辑
    // 例如：if (field.integrities & CT_PRIMARY_KEY) {...}
}


// 显示所有表的操作类
void ShowTablesOperation::execute() {
    TableManager::showTables();
}


// 删除表的操作类
void DropTableOperation::execute(){
    TableManager::dropTable(this);
}

// 描述表的操作类
void DescribeTableOperation::execute() {
    TableManager::describeTable(this);
}


// 添加字段操作类
void AddColumnOperation::execute() {
    TableManager::addColumn(this);
}

// 删除字段操作类
void DropColumnOperation::execute() {
    TableManager::dropColumn(this);
}

// 修改字段操作类
void ModifyColumnOperation::execute() {
    TableManager::modifyColumn(this);
}



// // 表定义文件结构体（对应 [表名].tdf 文件）
// struct FieldBlock {
//     qint32 order;         // 字段顺序（从1开始）
//     char name[128];       // 字段名称
//     qint32 type;          // 字段类型（如0=INTEGER, 1=VARCHAR）
//     qint32 param;         // 类型参数（如VARCHAR长度）
//     qint64 mtime;         // 最后修改时间
//     qint32 integrities;   // 完整性约束（占位）
// };
// enum DataType {
//     DT_INTEGER = 0,    // 4字节整型
//     DT_BOOL = 1,       // 1字节布尔
//     DT_DOUBLE = 2,     // 8字节浮点
//     DT_VARCHAR = 3,    // 变长字符串
//     DT_DATETIME = 4    // 16字节时间（用qint64简化存储）
// };

// // ------------------------------
// // 完整性约束类型枚举 (对应需求文档 3.12.2)
// // ------------------------------
// enum ConstraintType {
//     CT_PRIMARY_KEY = 0,   // 主键
//     CT_FOREIGN_KEY = 1,   // 外键
//     CT_CHECK = 2,         // 检查约束
//     CT_UNIQUE = 3,        // 唯一约束
//     CT_NOT_NULL = 4,      // 非空
//     CT_DEFAULT = 5,       // 默认值
//     CT_IDENTITY = 6       // 自增
// };



// // ------------------------------
// // 索引结构体 (对应 .tid 文件)
// // ------------------------------
// struct IndexBlock {
//     char name[128];        // 索引名称
//     bool is_unique;        // 是否唯一
//     bool is_asc;           // 是否升序
//     char field[128];       // 索引字段名
//     char index_file[256];  // 索引数据文件路径（如 "D:/DBMS_ROOT/data/db1/table1_name_idx.ix"）
// };
// // 表描述文件结构体（对应 [数据库名].tb 文件）
// struct TableBlock {
//     char name[128];       // 表名称
//     qint32 record_num;    // 记录数（初始为0）
//     qint32 field_num;     // 字段数（初始为0）
//     char tdf[256];        // 表定义文件路径（如 D:/DBMS_ROOT/data/db1/table1.tdf）
//     char trd[256];        // 记录文件路径（如 D:/DBMS_ROOT/data/db1/table1.trd）
//     char tic[256];        // 完整性文件路径（占位）
//     char tid[256];        // 索引文件路径（占位）
//     qint64 crtime;        // 创建时间（Unix时间戳）
//     qint64 mtime;         // 修改时间（初始等于创建时间）
// struct IndexBlock {
//     char name[128];        // 索引名称
//     bool is_unique;        // 是否唯一
//     bool is_asc;           // 是否升序
//     char field[128];       // 索引字段名
//     char index_file[256];  // 索引数据文件路径（如 "D:/DBMS_ROOT/data/db1/table1_name_idx.ix"）
// };


// // ------------------------------
// // 完整性约束结构体 (对应 .tic 文件)
// // ------------------------------
// struct IntegrityConstraint {
//     char name[128];        // 约束名称
//     char field[128];       // 作用的字段名
//     qint32 type;           // ConstraintType 枚举值
//     char params[256];      // 约束参数（如 CHECK 条件、DEFAULT 值）
// };


// // 数据库描述块 对应需求文件中的ruanko.db
// #pragma pack(push, 1)
// struct DatabaseBlock {
//     char name[128];     // 数据库名称，CHAR[128]
//     bool type;          // 类型（true=系统数据库，false=用户数据库）
//     char filename[256]; // 数据库文件夹路径，CHAR[256]
//     qint64 crtime;      // 创建时间（Unix 时间戳）

//     // 固定 128 + 1 + 256 + 8 = 393 字节
// };
// #pragma pack(pop)

// };
// // 创建数据库记录
// DatabaseBlock block;
// memset(&block, 0, sizeof(DatabaseBlock));

// // 填充字段
// strncpy(block.name, dbName.toUtf8().constData(), 128);
// block.type = true; // 用户数据库
// strncpy(block.filename, ("D:/DBMS_ROOT/data/" + dbName).toUtf8().constData(), 256);
// block.crtime = QDateTime::currentSecsSinceEpoch();
