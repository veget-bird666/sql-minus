#include "database_manager.h"
#include "database_manager.h"
#include "file_utils.h"
#include <QDate>
#include "widget.h"
extern Widget* widget;

QString DatabaseManager::currentDB = "";

// 管理创建数据库的操作
void DatabaseManager::createDatabase(const CreateDatabaseOperation* operation) {

    QString dbName = operation->dbName;

    // 参数校验
    if (dbName.length() > 128) {
        throw std::invalid_argument("Database name too long");
    }

    // 创建记录
    DatabaseBlock block;
    memset(&block, 0, sizeof(DatabaseBlock));

    // 填充字段
    strncpy(block.name, dbName.toUtf8().constData(), 128);
    block.type = true; // 用户数据库
    strncpy(block.filename, ("D:/DBMS_ROOT/data/" + dbName).toUtf8().constData(), 256);
    block.crtime = QDateTime::currentSecsSinceEpoch();

    // 调用存储模块
    FileUtil::createDatabaseFiles(dbName);
    FileUtil::appendDatabaseRecord(block);

    // 返回信息
    widget->showMessage("成功创建数据库："+dbName+".");
}



// 管理删除数据库的操作
void DatabaseManager::dropDatabase(const DropDatabaseOperation* operation) {
    QString dbName = operation->dbName;
    try {
        // 1. 检查是否为系统数据库（如 Ruanko）
        // if (dbName == "Ruanko") {
        //     throw std::runtime_error("系统数据库不可删除");
        // }

        // 2. 检查数据库是否存在
        std::vector<DatabaseBlock> databases = FileUtil::readAllDatabaseBlocks();
        bool exists = std::any_of(databases.begin(), databases.end(),
                                  [&dbName](const DatabaseBlock& block) {
                                      return QString::fromUtf8(block.name) == dbName;
                                  });
        if (!exists) {
            throw std::runtime_error("数据库不存在");
        }

        // 3. 删除元数据记录
        FileUtil::removeDatabaseRecord(dbName);

        // 4. 删除数据库文件夹
        FileUtil::deleteDatabaseDirectory(dbName);

        // 5. 提示用户
        widget->showMessage("数据库删除成功：" + dbName);
    } catch (const std::exception& e) {
        widget->showMessage("数据库删除失败：" + QString::fromStdString(e.what()));
    }
}


// 展示所有数据库
void DatabaseManager::showDatabases(){
    try {
        std::vector<DatabaseBlock> databases = FileUtil::readAllDatabaseBlocks();
        for (const auto& db : databases) {
            qDebug() << "数据库名称:" << db.name
                     << "类型:" << (db.type ? "系统" : "用户")
                     << "路径:" << db.filename;

        }

        // 格式化数据库列表
        QString message = "数据库列表：\n";
        for (const auto& db : databases) {
            QString dbName = QString::fromUtf8(db.name);
            QString dbType = db.type ? "系统数据库" : "用户数据库";
            QString dbPath = QString::fromUtf8(db.filename);
            QDateTime createTime = QDateTime::fromSecsSinceEpoch(db.crtime);

            message += QString("名称: %1\n类型: %2\n路径: %3\n创建时间: %4\n")
                           .arg(dbName)
                           .arg(dbType)
                           .arg(dbPath)
                           .arg(createTime.toString("yyyy-MM-dd HH:mm:ss"));
        }

        // 显示消息
        widget->showMessage(message);

    } catch (const std::exception& e) {
        qCritical() << "读取数据库列表失败:" << e.what();
    }
}

void DatabaseManager::useDatabase(const UseDatabaseOperation* operation){
    QString dbName = operation->dbName;
    try {
        // 1. 检查是否为系统数据库（如 Ruanko）
        // if (dbName == "Ruanko") {
        //     throw std::runtime_error("系统数据库不可删除");
        // }

        // 2. 检查数据库是否存在
        std::vector<DatabaseBlock> databases = FileUtil::readAllDatabaseBlocks();
        bool exists = std::any_of(databases.begin(), databases.end(),
                                  [&dbName](const DatabaseBlock& block) {
                                      return QString::fromUtf8(block.name) == dbName;
                                  });
        if (!exists) {
            throw std::runtime_error("数据库不存在");
        }
        widget->showMessage("数据库使用成功" + dbName);
    } catch (const std::exception& e) {
        widget->showMessage("数据库使用失败：" + QString::fromStdString(e.what()));
    }

    currentDB = operation->dbName;
}


