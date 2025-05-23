#include "file_utils.h"
#include <qdebug.h>
#include <QDir>
#include <QDebug>
#include "structures.h"
#include "table_operations.h"
#include <QFile>
#include <QByteArray>
#include <vector>
#include <cstring>
#include <QTextStream>


extern QString currentDB;

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



// 生成表文件路径（如 D:/DBMS_ROOT/data/db1/table1.tdf）
QString FileUtil::generateTableFilePath(const QString& dbName, const QString& tableName, const QString& suffix) {
    return QString("D:/DBMS_ROOT/data/%1/%2.%3").arg(dbName, tableName, suffix);
}



// 创建所有表文件
void FileUtil::createTableFiles(const CreateTableOperation* operation, const QString& dbName) {

    // 1. 生成表名
    QString tableName = QString::fromUtf8(operation->table_block.name);

    // 2. 生成文件路径
    QString tdfPath = generateTableFilePath(dbName, tableName, "tdf");
    QString trdPath = generateTableFilePath(dbName, tableName, "trd");

    // 3. 创建表定义文件 ([表名].tdf) 并写入字段信息
    QFile tdfFile(tdfPath);
    if (!tdfFile.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("无法创建表定义文件: " + tdfPath.toStdString());
    }
    for (const FieldBlock& field : operation->field_blocks) {
        tdfFile.write(reinterpret_cast<const char*>(&field), sizeof(FieldBlock));
    }
    tdfFile.close();

    // 4. 创建空记录文件 ([表名].trd)
    QFile trdFile(trdPath);
    if (!trdFile.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("无法创建记录文件: " + trdPath.toStdString());
    }
    trdFile.close();

    // 5. 更新表描述文件 ([数据库名].tb)
    appendTableRecord(operation->table_block, dbName);


    // 6. 创建完整性约束文件 ([表名].tic)
    QString ticPath = generateTableFilePath(dbName, tableName, "tic");
    createIntegrityFile(operation->constraints, ticPath);

    // 7. 创建索引描述文件 ([表名].tid) 和索引数据文件 ([表名].ix)
    QString tidPath = generateTableFilePath(dbName, tableName, "tid");
    QFile tidFile(tidPath);
    if (!tidFile.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("无法创建索引描述文件: " + tidPath.toStdString());
    }
    for (const IndexBlock& index : operation->indexes) {
        // 写入索引描述
        tidFile.write(reinterpret_cast<const char*>(&index), sizeof(IndexBlock));
        // 创建索引数据文件
        createIndexFile(index);
    }
    tidFile.close();
}


// 追加表记录到 [数据库名].tb
void FileUtil::appendTableRecord(const TableBlock& block, const QString& dbName) {
    QString tbPath = QString("D:/DBMS_ROOT/data/%1/%2.tb").arg(dbName, dbName);
    QFile file(tbPath);
    if (!file.open(QIODevice::Append)) {
        throw std::runtime_error("无法打开表描述文件: " + tbPath.toStdString());
    }
    file.write(reinterpret_cast<const char*>(&block), sizeof(TableBlock));
    file.close();
}


// 读取所有表记录
std::vector<TableBlock> FileUtil::readAllTableBlocks(const QString& dbName) {
    std::vector<TableBlock> blocks;
    QString tbPath = QString("D:/DBMS_ROOT/data/%1/%2.tb").arg(dbName, dbName);

    QFile file(tbPath);
    if (!file.exists()) return blocks;

    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("无法读取表描述文件: " + tbPath.toStdString());
    }

    // 读取整个文件
    QByteArray data = file.readAll();
    file.close();

    // 确保数据大小是 TableBlock 的整数倍
    if (data.size() % sizeof(TableBlock) != 0) {
        throw std::runtime_error("表描述文件损坏: 大小不匹配");
    }

    // 解析每个 TableBlock
    int blockCount = data.size() / sizeof(TableBlock);
    for (int i = 0; i < blockCount; i++) {
        TableBlock block;
        memcpy(&block, data.constData() + i * sizeof(TableBlock), sizeof(TableBlock));
        blocks.push_back(block);
    }

    return blocks;
}

// 创建完整性约束文件
void FileUtil::createIntegrityFile(const vector<IntegrityConstraint>& constraints, const QString& ticPath) {
    QFile ticFile(ticPath);
    if (!ticFile.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("无法创建完整性文件: " + ticPath.toStdString());
    }
    for (const IntegrityConstraint& constraint : constraints) {
        ticFile.write(reinterpret_cast<const char*>(&constraint), sizeof(IntegrityConstraint));
    }
    ticFile.close();
}

// 创建索引数据文件（空文件，仅占位）
void FileUtil::createIndexFile(const IndexBlock& index) {
    QFile ixFile(QString::fromUtf8(index.index_file));
    if (!ixFile.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("无法创建索引数据文件: " + QString::fromUtf8(index.index_file).toStdString());
    }
    ixFile.close();
}


// 删除表的操作
void FileUtil::dropTable(const QString& dbName, const QString& tableName) {
    try {
        // 1. 读取所有表记录
        std::vector<TableBlock> tables = readAllTableBlocks(dbName);

        // 2. 查找目标表（正确的方式）
        bool tableExists = false;
        TableBlock targetBlock;
        for (const auto& block : tables) {
            if (QString::fromUtf8(block.name) == tableName) {
                tableExists = true;
                targetBlock = block;
                break;
            }
        }

        if (!tableExists) {
            throw std::runtime_error("表不存在: " + tableName.toStdString());
        }

        // 3. 删除物理文件
        QString tdfPath = QString::fromUtf8(targetBlock.tdf);
        QString trdPath = QString::fromUtf8(targetBlock.trd);
        QString ticPath = QString::fromUtf8(targetBlock.tic);
        QString tidPath = QString::fromUtf8(targetBlock.tid);

        QFile::remove(tdfPath);
        QFile::remove(trdPath);
        QFile::remove(ticPath);
        QFile::remove(tidPath);

        // 4. 从内存集合中移除该表
        tables.erase(
            std::remove_if(tables.begin(), tables.end(),
                           [&tableName](const TableBlock& block) {
                               return QString::fromUtf8(block.name) == tableName;
                           }),
            tables.end()
            );

        // 5. 更新.tb文件
        QString tbFilePath = QString("D:/DBMS_ROOT/data/%1/%2.tb").arg(dbName, dbName);
        QFile tbFile(tbFilePath);
        if (!tbFile.open(QIODevice::WriteOnly)) {
            throw std::runtime_error("无法更新表描述文件");
        }

        for (const auto& block : tables) {
            tbFile.write(reinterpret_cast<const char*>(&block), sizeof(TableBlock));
        }
        tbFile.close();

        // 6. 删除索引文件
        QFile tidFile(tidPath);
        if (tidFile.open(QIODevice::ReadOnly)) {
            QByteArray data = tidFile.readAll();
            const char* ptr = data.constData();
            while (ptr < data.constData() + data.size()) {
                IndexBlock index;
                memcpy(&index, ptr, sizeof(IndexBlock));
                QFile::remove(QString::fromUtf8(index.index_file));
                ptr += sizeof(IndexBlock);
            }
            tidFile.close();
        }

    } catch (const std::exception& e) {
        qCritical() << "删除表失败:" << e.what();
        throw;
    }
}

// 更新表定义文件
void FileUtil::updateTableDefinition(const QString& dbName, const QString& tableName,
                                     const std::vector<FieldBlock>& newFields) {
    QString tdfPath = generateTableFilePath(dbName, tableName, "tdf");
    QFile tdfFile(tdfPath);

    if (!tdfFile.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("无法更新表定义文件");
    }

    for (const auto& field : newFields) {
        tdfFile.write(reinterpret_cast<const char*>(&field), sizeof(FieldBlock));
    }

    tdfFile.close();

}

// 读取表的所有字段
std::vector<FieldBlock> FileUtil::readTableFields(const QString& dbName, const QString& tableName) {
    std::vector<FieldBlock> fields;
    QString tdfPath = QString("D:/DBMS_ROOT/data/%1/%2.tdf").arg(dbName, tableName);

    QFile tdfFile(tdfPath);
    if (!tdfFile.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("无法打开表定义文件: " + tdfPath.toStdString());
    }

    QByteArray data = tdfFile.readAll();
    tdfFile.close();

    // 验证数据大小是 FieldBlock 的整数倍
    if (data.size() % sizeof(FieldBlock) != 0) {
        throw std::runtime_error("表定义文件损坏: 大小不匹配");
    }

    int blockCount = data.size() / sizeof(FieldBlock);
    for (int i = 0; i < blockCount; i++) {
        FieldBlock field;
        memcpy(&field, data.constData() + i * sizeof(FieldBlock), sizeof(FieldBlock));
        fields.push_back(field);
    }
    return fields;
}

// 插入数据
void FileUtil::appendDataRow(
    const QString& dbName,
    const QString& tableName,
    const DataRow& row
    ) {
    QString trdPath = QString("D:/DBMS_ROOT/data/%1/%2.trd").arg(dbName, tableName);

    QFile file(trdPath);
    if (!file.open(QIODevice::Append)) {
        throw std::runtime_error("Failed to open .trd file");
    }

    // 先写入头部
    file.write(reinterpret_cast<const char*>(&row.header), sizeof(DataRowHeader));

    // 然后写入所有字段值
    file.write(reinterpret_cast<const char*>(row.values.data()),
               row.values.size() * sizeof(FieldValue));

    file.close();
}

void FileUtil::updateTableBlocks(const QString& dbName, const std::vector<TableBlock>& tables) {
    // 1. 构造文件路径
    QString tbPath = QString("D:/DBMS_ROOT/data/%1/%2.tb").arg(dbName, dbName);

    // 2. 打开文件（覆盖模式）
    QFile tbFile(tbPath);
    if (!tbFile.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("Failed to open table metadata file for update");
    }

    // 3. 写入所有TableBlock
    for (const auto& table : tables) {
        if (tbFile.write(reinterpret_cast<const char*>(&table), sizeof(TableBlock)) != sizeof(TableBlock)) {
            tbFile.close();
            throw std::runtime_error("Failed to write table metadata");
        }
    }

    // 4. 关闭文件
    tbFile.close();
}


// 读取表中所有记录
std::vector<DataRow> FileUtil::readAllDataRows(const QString& dbName, const QString& tableName) {
    std::vector<DataRow> rows;
    QString trdPath = generateTableFilePath(dbName, tableName, "trd");

    QFile file(trdPath);
    if (!file.exists()) {
        throw std::runtime_error("Table data file not found");
    }

    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("Failed to open table data file");
    }

    QByteArray data = file.readAll();
    file.close();

    const char* ptr = data.constData();
    const char* endPtr = ptr + data.size();

    while (ptr < endPtr) {
        // 1. 读取头部
        if (ptr + sizeof(DataRowHeader) > endPtr) {
            throw std::runtime_error("Truncated data row header");
        }

        DataRow row;
        memcpy(&row.header, ptr, sizeof(DataRowHeader));
        ptr += sizeof(DataRowHeader);

        // 2. 检查字段数是否合理
        qDebug()<<row.header.fieldCount;
        if (row.header.fieldCount <= 0 || row.header.fieldCount > 1000) {
            throw std::runtime_error("Invalid field count in row");
        }

        // 3. 读取字段值
        size_t valuesSize = row.header.fieldCount * sizeof(FieldValue);
        if (ptr + valuesSize > endPtr) {
            throw std::runtime_error("Truncated field values in row");
        }

        row.values.resize(row.header.fieldCount);
        memcpy(row.values.data(), ptr, valuesSize);
        ptr += valuesSize;

        rows.push_back(row);
    }

    return rows;
}


// 附加日志记录
void FileUtil::appendLogRecord(const QString& dbName, const LogRecord& log) {
    // 日志文件路径：D:/DBMS_ROOT/data/dbName/dbName.log
    QString logPath = QString("D:/DBMS_ROOT/data/%1/%1.log").arg(dbName);

    QFile logFile(logPath);
    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        throw std::runtime_error("无法打开日志文件");
    }

    QTextStream out(&logFile);
    out << log.time.toString("yyyy-MM-dd HH:mm:ss") << "##"
        << log.type << "##"
        << log.sql << "##"
        << log.rollbackToken << "\n";

    logFile.close();
}
