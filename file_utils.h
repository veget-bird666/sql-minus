#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#include <QString>
#include "structures.h"
#include <vector>

class FileUtil{
public:

    // 创建数据库操作
    static void createDatabaseFiles(QString dbName);

    // 删除数据库操作
    static void dropDatabase(QString dbName);

    // 加入数据库记录操作
    static void appendDatabaseRecord(const DatabaseBlock& block);

    // 读取所有数据库记录
    static std::vector<DatabaseBlock> readAllDatabaseBlocks();

    // 删除 ruanko.db 中的指定数据库记录
    static void removeDatabaseRecord(const QString& dbName);

    // 递归删除数据库文件夹
    static void deleteDatabaseDirectory(const QString& dbName);
};






#endif // FILE_UTILS_H
