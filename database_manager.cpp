#include "database_manager.h"
#include "database_manager.h"
#include "file_utils.h"


void DatabaseManager::createDatabase(const QString& dbName) {
    // 参数校验（示例：名称长度限制）
    if (dbName.length() > 128) {
        throw std::invalid_argument("Database name too long");
    }
    // 调用存储模块
    // TO DO
}


