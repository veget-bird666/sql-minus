#include "file_utils.h"
#include <qdebug.h>

QString root_path = "D:/DBMS_ROOT/";

// 创建数据库操作
void FileWriteUtil::createDatabase(QString dbName){
    qDebug()<<"successfully create database"<<dbName<<"in"<<root_path;
}

// 删除数据库操作
void FileWriteUtil::dropDatabase(QString dbName){
    qDebug()<<"successfully drop database"<<dbName;
}
