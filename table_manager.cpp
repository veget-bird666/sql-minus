#include "table_manager.h"
#include "file_utils.h"
#include <QDate>
#include "widget.h"
extern Widget* widget;

// 管理创建表操作
void TableManager::createTable(const CreateTableOperation* operation) {

    QString tableName = operation->table_block.name;
    QString dbName = operation->dbName;
    TableBlock block = operation->table_block;

    // 参数校验
    if (dbName.isEmpty()) {
        throw std::invalid_argument("No database has been used");
    }
    if (tableName.length() > 128) {
        throw std::invalid_argument("Table name too long");
    }

    // 判断是否存在重复的表
    std::vector<TableBlock> testBlocks = FileUtil::readAllTableBlocks(dbName);
    for (TableBlock testBlock : testBlocks) {
        if (std::strcmp(testBlock.name, block.name) == 0) {
            throw std::invalid_argument("Table has existed");
        }
    }


    // 调用存储模块
    FileUtil::createTableFiles(operation, dbName);
    FileUtil::appendTableRecord(operation->table_block, dbName);

    // 返回信息
    widget->showMessage("成功创建表："+tableName+".");
}
