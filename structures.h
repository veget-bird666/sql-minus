#ifndef STRUCTURES_H
#define STRUCTURES_H
#pragma once
#include <QtGlobal>
#pragma pack(push, 1)

// 此文件用来定义文件存储时序列化、反序列化的所有结构体


// 数据库描述块（与需求文档严格对应）
struct DatabaseBlock {
    char name[128];     // 数据库名称，CHAR[128]
    bool type;          // 类型（true=系统数据库，false=用户数据库）
    char filename[256]; // 数据库文件夹路径，CHAR[256]
    qint64 crtime;      // 创建时间（Unix 时间戳）

    // 固定 128 + 1 + 256 + 8 = 393 字节
};






#pragma pack(pop)

#endif // STRUCTURES_H
