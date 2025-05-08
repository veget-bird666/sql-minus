#include "table_manager.h"
#include "file_utils.h"
#include <QDate>
#include "widget.h"
#include <qfile.h>

extern Widget* widget;
extern QString currentDB;
// 管理创建表操作
void TableManager::createTable(const CreateTableOperation* operation) {
    // 校验字段
    for (const auto& field : operation->field_blocks) {
        if (field.isAggregateFunc) {
            throw std::runtime_error(
                "Aggregate function cannot be used in table definition: " +
                QString(field.name).toStdString()
                );
        }
    }

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

    // 返回信息
    widget->showMessage("成功创建表："+tableName+".");
}


// 管理展示所有表的操作
void TableManager::showTables() {
    QString dbName = currentDB;

    if (dbName.isEmpty()) {
        throw std::invalid_argument("No database selected. Please use 'USE DATABASE' first.");
    }

    try {
        std::vector<TableBlock> tables = FileUtil::readAllTableBlocks(dbName);

        if (tables.empty()) {
            widget->showMessage("No tables found in database: " + dbName);
            return;
        }

        QString message = QString("+----------------------+------------+---------------------+\n"
                                "| Table Name           | Fields     | Created At          |\n"
                                "+----------------------+------------+---------------------+\n");

        for (const auto& table : tables) {
            QDateTime createTime = QDateTime::fromSecsSinceEpoch(table.crtime);
            message += QString("| %1 | %2         | %3 |\n")
                           .arg(QString::fromUtf8(table.name).leftJustified(20, ' '))
                           .arg(QString::number(table.field_num).leftJustified(10, ' ')) // 将整数转换为字符串
                           .arg(createTime.toString("yyyy-MM-dd HH:mm").leftJustified(19, ' '));
        }

        message += "+----------------------+------------+---------------------+";
        widget->showMessage(message);
    } catch (const std::exception& e) {
        widget->showMessage("Error showing tables: " + QString::fromStdString(e.what()));
    }
}

// 管理删除表的操作
void TableManager::dropTable(const DropTableOperation* operation) {
    QString dbName = operation->dbName;
    QString tableName = operation->tableName;

    // 校验数据库是否已选中
    if (dbName.isEmpty()) {
        throw std::invalid_argument("未选择数据库");
    }

    // 调用 FileUtil 删除表
    FileUtil::dropTable(dbName, tableName);

    // 提示用户
    widget->showMessage("成功删除表: " + tableName);
}


// 管理描述表字段的操作
void TableManager::describeTable(const DescribeTableOperation* operation) {
    QString dbName = operation->dbName;
    QString tableName = operation->tableName;

    if (dbName.isEmpty()) {
        throw std::invalid_argument("未选择数据库");
    }

    // 读取表字段信息
    std::vector<FieldBlock> fields = FileUtil::readTableFields(dbName, tableName);

    // 格式化输出
    QString message ="DESCRIBE_RESPONSE\n"
        "+---------------------+------------+----------+----------------+\n"
        "| Field Name          | Type       | Length   | Constraints    |\n"
        "+---------------------+------------+----------+----------------+\n";

    for (const FieldBlock& field : fields) {
        QString typeStr;
        switch (field.type) {
        case DT_INTEGER: typeStr = "INTEGER"; break;
        case DT_BOOL:    typeStr = "BOOL";    break;
        case DT_DOUBLE:  typeStr = "DOUBLE";  break;
        case DT_VARCHAR: typeStr = "VARCHAR"; break;
        case DT_DATETIME: typeStr = "DATETIME"; break;
        default: typeStr = "UNKNOWN";
        }

        QString constraints;
        if (field.integrities &(1 << CT_PRIMARY_KEY)) constraints += "PK ";
        if (field.integrities &(1 << CT_NOT_NULL)   ) constraints += "NN ";
        if (field.integrities &(1 << CT_UNIQUE)     ) constraints += "UQ ";
        if (field.integrities &(1 << CT_DEFAULT)    ) constraints += "DE ";
        if (field.integrities &(1 << CT_FOREIGN_KEY)) constraints += "FK ";

        message += QString("| %1 | %2 | %3 | %4 |\n")
                       .arg(QString::fromUtf8(field.name).leftJustified(20, ' '))
                       .arg(typeStr.leftJustified(10, ' '))
                       .arg(QString::number(field.param).leftJustified(8, ' '))
                       .arg(constraints.leftJustified(14, ' '));
    }
    message += "+---------------------+------------+----------+----------------+";

    widget->showMessage(message);
}

// 管理添加字段
void TableManager::addColumn(AddColumnOperation* operation) {
    auto fields = FileUtil::readTableFields(operation->dbName, operation->tableName);

    // 检查字段是否已存在
    for (const auto& f : fields) {
        if (QString::fromUtf8(f.name) == QString::fromUtf8(operation->newField.name)) {
            throw std::invalid_argument("字段已存在");
        }
    }

    // 更新字段顺序
    operation->newField.order = fields.size() + 1;
    fields.push_back(operation->newField);

    // 更新表定义
    FileUtil::updateTableDefinition(operation->dbName, operation->tableName, fields);

    // 更新表元数据
    auto tables = FileUtil::readAllTableBlocks(operation->dbName);
    for (auto& table : tables) {
        if (QString::fromUtf8(table.name) == operation->tableName) {
            table.field_num++;
            table.mtime = QDateTime::currentSecsSinceEpoch();
            break;
        }
    }

    // 更新.tb文件
    QString tbPath = QString("D:/DBMS_ROOT/data/%1/%2.tb").arg(operation->dbName, operation->dbName);
    QFile tbFile(tbPath);
    tbFile.open(QIODevice::WriteOnly);
    for (const auto& table : tables) {
        tbFile.write(reinterpret_cast<const char*>(&table), sizeof(TableBlock));
    }

    widget->showMessage("成功添加字段: " + QString::fromUtf8(operation->newField.name));
}

// 管理删除字段
void TableManager::dropColumn(DropColumnOperation* operation) {
    auto fields = FileUtil::readTableFields(operation->dbName, operation->tableName);

    // 查找并删除字段
    auto it = std::remove_if(fields.begin(), fields.end(),
                             [&](const FieldBlock& f) {
                                 return QString::fromUtf8(f.name) == operation->columnName;
                             });

    if (it == fields.end()) {
        throw std::invalid_argument("字段不存在");
    }

    fields.erase(it, fields.end());

    // 更新字段顺序
    for (int i=0; i<fields.size(); i++) {
        fields[i].order = i+1;
    }

    FileUtil::updateTableDefinition(operation->dbName, operation->tableName, fields);

    // 更新元数据（类似addColumn逻辑）
    auto tables = FileUtil::readAllTableBlocks(operation->dbName);
    for (auto& table : tables) {
        if (QString::fromUtf8(table.name) == operation->tableName) {
            table.field_num--;
            table.mtime = QDateTime::currentSecsSinceEpoch();
            break;
        }
    }

    // 更新.tb文件
    QString tbPath = QString("D:/DBMS_ROOT/data/%1/%2.tb").arg(operation->dbName, operation->dbName);
    QFile tbFile(tbPath);
    tbFile.open(QIODevice::WriteOnly);
    for (const auto& table : tables) {
        tbFile.write(reinterpret_cast<const char*>(&table), sizeof(TableBlock));
    }

    widget->showMessage("成功删除字段: " + operation->columnName);
}

// 管理修改字段
void TableManager::modifyColumn(ModifyColumnOperation* operation) {
    auto fields = FileUtil::readTableFields(operation->dbName, operation->tableName);

    bool found = false;
    for (auto& field : fields) {
        if (QString::fromUtf8(field.name) == operation->oldColumnName) {
            field = operation->newField;
            found = true;
            break;
        }
    }

    if (!found) throw std::invalid_argument("字段不存在");

    FileUtil::updateTableDefinition(operation->dbName, operation->tableName, fields);
    widget->showMessage("成功修改字段: " + operation->oldColumnName);
}






