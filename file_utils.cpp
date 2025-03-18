#include "file_utils.h"
#include <qdebug.h>
#include <QFile>
#include <QDir>
#include <QDebug>
#include "structures.h"


QString root_path = "D:/DBMS_ROOT";

// 创建数据库操作
void FileUtil::createDatabaseFiles(QString dbName){

    // 定义路径
    QString rootPath = "D:/DBMS_ROOT";
    QString dataPath = rootPath + "/data";
    QString dbPath = dataPath + "/" + dbName;

    // 1. 检查数据库文件夹是否已存在
    QDir dbDir(dbPath);
    if (dbDir.exists()) {
        throw std::runtime_error("数据库已存在：" + dbName.toStdString());
    }

    // 2. 创建数据库文件夹
    if (!dbDir.mkpath(".")) {
        throw std::runtime_error("无法创建数据库文件夹：" + dbPath.toStdString());
    }

    // 3. 创建表描述文件 [数据库名].tb
    QString tbFilePath = dbPath + "/" + dbName + ".tb";
    QFile tbFile(tbFilePath);
    if (!tbFile.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("无法创建表描述文件：" + tbFilePath.toStdString());
    }
    tbFile.close();

    // 4. 创建日志文件 [数据库名].log
    QString logFilePath = dbPath + "/" + dbName + ".log";
    QFile logFile(logFilePath);
    if (!logFile.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("无法创建日志文件：" + logFilePath.toStdString());
    }
    logFile.close();

}

// 删除数据库操作
void FileUtil::dropDatabase(QString dbName){

    // TO DO
    qDebug()<<"successfully drop database"<<dbName;

}



// 加入数据库记录操作
void FileUtil::appendDatabaseRecord(const DatabaseBlock& block) {
    // 定义文件路径
    QString rootPath = "D:/DBMS_ROOT";
    QString dbMetaPath = rootPath + "/ruanko.db";

    // 1. 确保根目录存在
    QDir rootDir(rootPath);
    if (!rootDir.exists()) {
        if (!rootDir.mkpath(".")) {
            throw std::runtime_error("Failed to create root directory: " + rootPath.toStdString());
        }
    }

    // 2. 打开文件（追加模式）
    QFile file(dbMetaPath);
    if (!file.open(QIODevice::Append)) {
        throw std::runtime_error("Failed to open ruanko.db: " + file.errorString().toStdString());
    }

    // 3. 序列化结构体为二进制数据
    const char* blockData = reinterpret_cast<const char*>(&block);
    qint64 bytesToWrite = sizeof(DatabaseBlock);

    // 4. 写入文件
    qint64 bytesWritten = file.write(blockData, bytesToWrite);
    file.close();

    // 5. 验证写入是否完整
    if (bytesWritten != bytesToWrite) {
        throw std::runtime_error("Failed to write full database record.");
    }
}


// 读取ruanko.db的所有数据库记录，返回DatabaseBlock集合
std::vector<DatabaseBlock> FileUtil::readAllDatabaseBlocks() {
    std::vector<DatabaseBlock> blocks;
    QString rootPath = "D:/DBMS_ROOT";
    QString dbMetaPath = rootPath + "/ruanko.db";

    // 1. 检查文件是否存在
    QFile file(dbMetaPath);
    if (!file.exists()) {
        return blocks; // 文件不存在，返回空列表
    }

    // 2. 打开文件
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("Failed to open ruanko.db: " + file.errorString().toStdString());
    }

    // 3. 读取全部数据
    QByteArray data = file.readAll();
    file.close();

    // 4. 验证数据完整性
    qint64 totalBytes = data.size();
    if (totalBytes % sizeof(DatabaseBlock) != 0) {
        qWarning() << "ruanko.db 文件可能已损坏（数据块不完整）";
    }

    // 5. 逐条解析数据
    for (qint64 i = 0; i < totalBytes; i += sizeof(DatabaseBlock)) {
        // 检查剩余数据是否足够构成一个 DatabaseBlock
        if (i + sizeof(DatabaseBlock) > totalBytes) {
            break;
        }

        // 反序列化
        DatabaseBlock block;
        memcpy(&block, data.constData() + i, sizeof(DatabaseBlock));
        blocks.push_back(block);
    }

    return blocks;
}

// 移除ruanko.db中的数据库记录
void FileUtil::removeDatabaseRecord(const QString& dbName) {
    // 1. 读取所有记录
    std::vector<DatabaseBlock> blocks = readAllDatabaseBlocks();

    // 2. 过滤掉目标数据库
    auto it = std::remove_if(blocks.begin(), blocks.end(),
                             [&dbName](const DatabaseBlock& block) {
                                 return QString::fromUtf8(block.name) == dbName;
                             });
    blocks.erase(it, blocks.end());

    // 3. 覆盖写入 ruanko.db
    QString dbMetaPath = "D:/DBMS_ROOT/ruanko.db";
    QFile file(dbMetaPath);
    if (!file.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("无法更新元数据文件");
    }

    for (const auto& block : blocks) {
        file.write(reinterpret_cast<const char*>(&block), sizeof(DatabaseBlock));
    }
    file.close();
}


// 删除数据库目录下的文件夹
void FileUtil::deleteDatabaseDirectory(const QString& dbName) {
    QString dbPath = "D:/DBMS_ROOT/data/" + dbName;
    QDir dbDir(dbPath);

    if (!dbDir.exists()) {
        throw std::runtime_error("数据库文件夹不存在");
    }

    // 递归删除文件夹及其内容
    if (!dbDir.removeRecursively()) {
        throw std::runtime_error("无法删除数据库文件夹");
    }
}
