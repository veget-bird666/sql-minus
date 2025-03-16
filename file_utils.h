#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#include <QString>

class FileReadUtil{
public:

};

class FileWriteUtil{
public:
    // 创建数据库操作
    static void createDatabase(QString dbName);

    // 删除数据库操作
    static void dropDatabase(QString dbName);

};






#endif // FILE_UTILS_H
