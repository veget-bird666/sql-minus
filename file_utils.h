#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#include <QString>
#include "structures.h"
#include "table_operations.h"
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


    // 创建表文件（核心逻辑）
    static void createTableFiles(const CreateTableOperation* operation, const QString& dbName);

    // 追加表记录到 [数据库名].tb 文件
    static void appendTableRecord(const TableBlock& block, const QString& dbName);

    // 读取数据库的所有表记录（用于校验表名重复）
    static std::vector<TableBlock> readAllTableBlocks(const QString& dbName);


private:

    // 生成表文件路径（辅助方法）
    static QString generateTableFilePath(const QString& dbName, const QString& tableName, const QString& suffix);

    // 生成索引数据文件（.ix）
    static void createIndexFile(const IndexBlock& index);

    // 生成完整性约束文件（.tic）
    static void createIntegrityFile(const vector<IntegrityConstraint>& constraints, const QString& ticPath);

};

#endif // FILE_UTILS_H
