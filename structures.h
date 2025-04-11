#ifndef STRUCTURES_H
#define STRUCTURES_H
#pragma once
#include <QtGlobal>

// 此文件用来定义文件存储时序列化、反序列化的所有结构体
// 注：需求文件中的int值全部转为qin32 严格控制为32位



#pragma pack(push, 1)

// ------------------------------
// 数据类型枚举 (对应需求文档 3.12.1)
// ------------------------------
enum DataType {
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
    qint32 field_num;     // 字段数（初始为0）
    char tdf[256];        // 表定义文件路径（如 D:/DBMS_ROOT/data/db1/table1.tdf）
    char trd[256];        // 记录文件路径（如 D:/DBMS_ROOT/data/db1/table1.trd）
    char tic[256];        // 完整性文件路径（占位）
    char tid[256];        // 索引文件路径（占位）
    qint64 crtime;        // 创建时间（Unix时间戳）
    qint64 mtime;         // 修改时间（初始等于创建时间）

};



// 表定义文件结构体（对应 [表名].tdf 文件）
struct FieldBlock {
    qint32 order;         // 字段顺序（从1开始）
    char name[128];       // 字段名称
    qint32 type;          // 字段类型（如0=INTEGER, 1=VARCHAR）
    qint32 param;         // 类型参数（如VARCHAR长度）
    qint64 mtime;         // 最后修改时间
    qint32 integrities;   // 完整性约束（占位）

    // 外键约束相关
    char ref_table[128];  // 引用的表名 (外键约束)
    char ref_field[128];  // 引用的字段名 (外键约束)

    // 检查约束相关
    char check_condition[256];  // 检查约束条件
};

#pragma pack(pop)





#endif // STRUCTURES_H


