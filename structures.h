#ifndef STRUCTURES_H
#define STRUCTURES_H
#pragma once
#include <QtGlobal>
#include <vector>
#include <cstring>
// 此文件用来定义文件存储时序列化、反序列化的所有结构体
// 注：需求文件中的int值全部转为qin32 严格控制为32位



// ------------------------------
// 数据类型枚举 (对应需求文档 3.12.1)
// ------------------------------
enum DataType {
    DT_NULL = -1,      // 为处理代码而临时添加的空值类型，待修改
    DT_INTEGER = 0,    // 4字节整型
    DT_BOOL = 1,       // 1字节布尔
    DT_DOUBLE = 2,     // 8字节浮点
    DT_VARCHAR = 3,    // 变长字符串
    DT_DATETIME = 4    // 16字节时间（用qint64简化存储）
};

// ------------------------------
// 完整性约束类型枚举 (对应需求文档 3.12.2)
// ------------------------------
enum ConstraintType {
    CT_PRIMARY_KEY = 0,   // 主键
    CT_FOREIGN_KEY = 1,   // 外键
    CT_CHECK = 2,         // 检查约束
    CT_UNIQUE = 3,        // 唯一约束
    CT_NOT_NULL = 4,      // 非空
    CT_DEFAULT = 5,       // 默认值
    CT_IDENTITY = 6       // 自增
};



// ------------------------------
// 索引结构体 (对应 .tid 文件)
// ------------------------------
struct IndexBlock {
    char name[128];        // 索引名称
    bool is_unique;        // 是否唯一
    bool is_asc;           // 是否升序
    char field[128];       // 索引字段名
    char index_file[256];  // 索引数据文件路径（如 "D:/DBMS_ROOT/data/db1/table1_name_idx.ix"）
};


// ------------------------------
// 完整性约束结构体 (对应 .tic 文件)
// ------------------------------
struct IntegrityConstraint {
    char name[128];        // 约束名称
    char field[128];       // 作用的字段名
    qint32 type;           // ConstraintType 枚举值
    char params[256];      // 约束参数（如 CHECK 条件、DEFAULT 值）
};


// 数据库描述块 对应需求文件中的ruanko.db
#pragma pack(push, 1)
struct DatabaseBlock {
    char name[128];     // 数据库名称，CHAR[128]
    bool type;          // 类型（true=系统数据库，false=用户数据库）
    char filename[256]; // 数据库文件夹路径，CHAR[256]
    qint64 crtime;      // 创建时间（Unix 时间戳）

    // 固定 128 + 1 + 256 + 8 = 393 字节
};
#pragma pack(pop)



#pragma pack(push, 1)
// 表描述文件结构体（对应 [数据库名].tb 文件）
struct TableBlock {
    char name[128];       // 表名称
    qint32 record_num;    // 记录数（初始为0）
    qint32 field_num;     // 字段数
    char tdf[256];        // 表定义文件路径（如 D:/DBMS_ROOT/data/db1/table1.tdf）
    char trd[256];        // 记录文件路径（如 D:/DBMS_ROOT/data/db1/table1.trd）
    char tic[256];        // 完整性文件路径
    char tid[256];        // 索引文件路径
    qint64 crtime;        // 创建时间（Unix时间戳）
    qint64 mtime;         // 修改时间（初始等于创建时间）

};
#pragma pack(pop)


#pragma pack(push, 1)
// 聚合函数结构体
struct FunctionCall {
    char funcName[32];   // 函数名（COUNT/MAX/MIN等）
    char fieldName[128]; // 作用的字段名（或"*"）
    qint32 funcType;     // 函数类型枚举值
};
#pragma pack(pop)


#pragma pack(push, 1)
// 函数类型枚举
enum FunctionType {
    FT_COUNT,
    FT_MAX,
    FT_MIN,
    FT_AVG,
    FT_SUM
};
#pragma pack(pop)


#pragma pack(push, 1)
// 表定义文件结构体（对应 [表名].tdf 文件）
struct FieldBlock {
    qint32 order;         // 字段顺序（从1开始）
    char name[128];       // 字段名称
    qint32 type;          // 字段类型（如0=INTEGER, 1=VARCHAR）
    qint32 param;         // 类型参数（如VARCHAR长度）
    qint64 mtime;         // 最后修改时间
    qint32 integrities;   // 完整性约束

    // // 外键约束相关
    // char ref_table[128];  // 引用的表名 (外键约束)
    // char ref_field[128];  // 引用的字段名 (外键约束)

    // 检查约束相关
    char check_condition[256];  // 检查约束条件

    // 新增聚合函数信息（共用体节省空间）
    union {
        // struct {
        //     char funcName[32];    // 对齐 FunctionCall.funcName
        //     char funcField[128];  // 对齐 FunctionCall.fieldName
        //     qint32 funcType;      // 对齐 FunctionCall.funcType
        // };
        FunctionCall func;
        struct {
            char ref_table[128];   // 与原外键字段共用空间
            char ref_field[128];
            qint32 reserved;      // 占位对齐
        };
    };

    // 标记是否为函数字段(默认不是）
    bool isAggregateFunc = false;
};

#pragma pack(pop)



// 字段值（支持动态类型）
struct FieldValue {
    qint32 type;   // 数据类型（DataType枚举）
    union {
        qint32 intVal;
        bool boolVal;
        double doubleVal;
        char varcharVal[256]; // 变长字符串需按实际长度存储
    };
};

#pragma pack(push, 1)
struct DataRowHeader {
    qint64 rowId;       // 行ID（自增）
    qint32 fieldCount;  // 字段数
};
#pragma pack(pop)

// 注意：DataRow 不再使用 #pragma pack，使用默认对齐
struct DataRow {
    DataRowHeader header;
    std::vector<FieldValue> values;

    // 计算总大小
    size_t size() const {
        return sizeof(DataRowHeader) + values.size() * sizeof(FieldValue);
    }
};





// 条件结构体，用于where后条件的判断
#pragma pack(push, 1)
struct Condition {
    char fieldName[128];      // 字段名
    qint32 operatorType;      // 操作符类型(0=, 1<, 2>, 3<=, 4>=, 5!=)
    FieldValue compareValue;  // 比较值

    // 评估条件是否满足
    bool evaluate(const FieldValue& rowValue) const {
        switch (operatorType) {
        case 0: // =
            return compareFieldValues(rowValue, compareValue) == 0;
        case 1: // <
            return compareFieldValues(rowValue, compareValue) < 0;
        case 2: // >
            return compareFieldValues(rowValue, compareValue) > 0;
        case 3: // <=
            return compareFieldValues(rowValue, compareValue) <= 0;
        case 4: // >=
            return compareFieldValues(rowValue, compareValue) >= 0;
        case 5: // !=
            return compareFieldValues(rowValue, compareValue) != 0;
        default:
            return false;
        }
    }


    // 比较两个FieldValue
    int compareFieldValues(const FieldValue& v1, const FieldValue& v2) const {
        if (v1.type != v2.type) return -1; // 类型不同

        switch (v1.type) {
        case DT_INTEGER:
            return v1.intVal - v2.intVal;
        case DT_BOOL:
            return v1.boolVal - v2.boolVal;
        case DT_DOUBLE:
            return (v1.doubleVal < v2.doubleVal) ? -1 : (v1.doubleVal > v2.doubleVal) ? 1 : 0;
        case DT_VARCHAR:
            return strcmp(v1.varcharVal, v2.varcharVal);
        case DT_DATETIME:
            return (v1.intVal < v2.intVal) ? -1 : (v1.intVal > v2.intVal) ? 1 : 0;
        default:
            return -1;
        }
    }
};
#pragma pack(pop)




#endif // STRUCTURES_H


