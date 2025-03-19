#include "database_operations.h"
#include "database_manager.h"

// 创建数据库操作
void CreateDatabaseOperation::execute(){
    DatabaseManager::createDatabase(this);
}

// 删除数据库操作
void DropDatabaseOperation::execute(){
    DatabaseManager::dropDatabase(this);
}

// 显示所有数据库操作
void ShowDatabasesOperation::execute(){
    DatabaseManager::showDatabases();
}
