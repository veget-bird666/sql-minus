#include "tuple_manager.h"
#include "file_utils.h"
#include <QDate>
#include "widget.h"
#include <QFile>
#include <QByteArray>
#include <vector>
#include <cstring>
#include <QDebug>

extern Widget* widget;

void TupleManager::insert(const InsertOperation* op) {
    // 1. 校验表是否存在
    auto tables = FileUtil::readAllTableBlocks(op->dbName);
    auto it = std::find_if(tables.begin(), tables.end(),
                           [&](const TableBlock& tb) { return tb.name == op->tableName; });
    if (it == tables.end()) {
        throw std::runtime_error("Table not found");
    }

    // 2. 校验字段数量和类型
    auto fields = FileUtil::readTableFields(op->dbName, op->tableName);
    if (fields.size() != op->values.size()) {
        throw std::runtime_error("Field count mismatch");
    }
    for (size_t i = 0; i < fields.size(); i++) {
        if (fields[i].type != op->values[i].type) {
            throw std::runtime_error("Field type mismatch");
        }
    }

    // 构造DataRow
    DataRow row;
    row.header.rowId = QDateTime::currentSecsSinceEpoch();
    row.header.fieldCount = op->values.size();
    row.values = op->values; // 复制字段值

    // 写入文件
    FileUtil::appendDataRow(op->dbName, op->tableName, row);

    // 5. 更新表元数据（记录数+1）
    for (auto& table : tables) {
        if (strcmp(table.name, op->tableName.toUtf8().constData()) == 0) {
            table.record_num++; // 记录数+1
            table.mtime = QDateTime::currentSecsSinceEpoch(); // 更新修改时间
            break;
        }
    }
    // 重新写入.tb文件（需实现FileUtil::updateTableBlocks）
    FileUtil::updateTableBlocks(op->dbName, tables);
    widget->showMessage("插入数据成功");
}


void TupleManager::selectAll(const SelectAllOperation* operation) {

    QString dbName = operation->dbName;
    QString tableName = operation->tableName;

    qDebug()<<"成功进入查询*的Manager";

    try {
        // 读取表的字段定义
        std::vector<FieldBlock> fields = FileUtil::readTableFields(dbName, tableName);

        // 读取表的所有记录
        std::vector<DataRow> rows = FileUtil::readAllDataRows(dbName, tableName);

        // 检查字段数是否匹配
        if (!fields.empty() && !rows.empty() && fields.size() != rows[0].header.fieldCount) {
            throw std::runtime_error("Field count mismatch between table definition and data");
        }
        // 格式化输出
        QString message = QString("+");
        for (const auto& field : fields) {
            message += QString("-").repeated(20) + "+";
        }
        message += "\n";
        qDebug()<<"111";
        // 表头
        for (const auto& field : fields) {
            message += QString("| %1 ").arg(QString::fromUtf8(field.name)).leftJustified(18, ' ') + "|";
        }
        message += "\n" + QString("+").repeated(fields.size() * 21 + 1) + "\n";

        qDebug()<<"表头为"<<message;

        // 数据行
        for (const auto& row : rows) {
            message += "|";
            for (int i = 0; i < row.header.fieldCount; i++) {
                FieldValue value = row.values[i];
                QString valueStr;

                switch (value.type) {
                case DT_INTEGER:
                    valueStr = QString::number(value.intVal);
                    break;
                case DT_BOOL:
                    valueStr = value.boolVal ? "true" : "false";
                    break;
                case DT_DOUBLE:
                    valueStr = QString::number(value.doubleVal, 'f', 2);
                    break;
                case DT_VARCHAR:
                    valueStr = QString::fromUtf8(value.varcharVal);
                    break;
                case DT_DATETIME:
                    valueStr = QDateTime::fromSecsSinceEpoch(value.intVal).toString("yyyy-MM-dd HH:mm:ss");
                    break;
                default:
                    valueStr = "UNKNOWN";
                }
                message += " " + valueStr.leftJustified(18, ' ') + "|";
            }
            message += "\n";
        }

        message += QString("+").repeated(fields.size() * 21 + 1) + "\n";
        widget->showMessage(message);

    } catch (const std::exception& e) {
        widget->showMessage("Error: " + QString::fromStdString(e.what()));
    }
}

