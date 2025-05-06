#include "tuple_manager.h"
#include "file_utils.h"
#include <QDate>

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

    // 3. 构造DataRow
    DataRow row;
    row.rowId = QDateTime::currentSecsSinceEpoch(); // 临时用时间戳作为行ID
    row.fieldCount = fields.size();
    // 动态分配内存并拷贝values（需手动管理内存）
    char* buffer = new char[sizeof(DataRow) + fields.size() * sizeof(FieldValue)];
    memcpy(buffer, &row, sizeof(DataRow));
    memcpy(buffer + sizeof(DataRow), op->values.data(), fields.size() * sizeof(FieldValue));

    // 4. 写入文件
    FileUtil::appendDataRow(op->dbName, op->tableName, *reinterpret_cast<DataRow*>(buffer));
    delete[] buffer; // 释放内存

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
}
