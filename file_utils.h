#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#include <QString>
#include "structures.h"
#include "table_operations.h"
#include <vector>
#include <cstring>

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

    // 生成表文件路径（辅助方法）
    static QString generateTableFilePath(const QString& dbName, const QString& tableName, const QString& suffix);

    // 创建表文件（核心逻辑）
    static void createTableFiles(const CreateTableOperation* operation, const QString& dbName);

    // 追加表记录到 [数据库名].tb 文件
    static void appendTableRecord(const TableBlock& block, const QString& dbName);

    // 读取数据库的所有表记录（用于校验表名重复）
    static std::vector<TableBlock> readAllTableBlocks(const QString& dbName);

    // 删除表的操作
    static void dropTable(const QString& dbName, const QString& tableName);

    // 更新表定义文件
    static void updateTableDefinition(const QString& dbName, const QString& tableName,
                                      const std::vector<FieldBlock>& newFields);

    // 读取表字段定义
    static std::vector<FieldBlock> readTableFields(const QString& dbName, const QString& tableName);


        // 追加数据行到.trd文件
        static void appendDataRow(
            const QString& dbName,
            const QString& tableName,
            const DataRow& row
            );

        // // 读取表的字段定义（用于校验插入数据的类型）
        // static std::vector<FieldBlock> readTableFields(
        //     const QString& dbName,
        //     const QString& tableName
        //     );
        // 更新
        static void updateTableBlocks(const QString& dbName, const std::vector<TableBlock>& tables);


        // 新增方法：读取表的所有记录
        static std::vector<DataRow> readAllDataRows(const QString& dbName, const QString& tableName);
private:



    // 生成索引数据文件（.ix）
    static void createIndexFile(const IndexBlock& index);

    // 生成完整性约束文件（.tic）
    static void createIntegrityFile(const vector<IntegrityConstraint>& constraints, const QString& ticPath);


};

#endif // FILE_UTILS_H
