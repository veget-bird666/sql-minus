#include "database_operations.h"
#include "file_utils.h"

// 创建数据库操作
void CreateDatabaseOperation::execute(){
    FileWriteUtil::createDatabase(dbName);
}

// 删除数据库操作
void DropDatabaseOperation::execute(){
    FileWriteUtil::dropDatabase(dbName);
}
