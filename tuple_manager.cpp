#include "tuple_manager.h"
#include "file_utils.h"
#include <QDate>
#include "widget.h"
#include <QFile>
#include <QByteArray>
#include <vector>
#include <cstring>
#include <QDebug>
#include "structures.h"
#include <QRegularExpression>

extern Widget* widget;

// 插入记录
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

    // 去除所有聚合函数, 我认为聚合函数都在最后
    int j = 0;
    for (FieldBlock fieldBlock : fields) {
        if (!fieldBlock.isAggregateFunc) ++j;
    }
    // 去除所有聚合函数完成

    if (j != op->values.size()) {
        throw std::runtime_error("Field count mismatch");
    }
    for (size_t i = 0; i < j; i++) {
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
    // 写入日志
    FileUtil::appendLogRecord(op->dbName , op->logRecord);
}

// 查询所有记录
/*void TupleManager::selectAll(const SelectAllOperation* operation) {

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
        QString message = QString("SELECT_RESPONSE\n+");
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
}*/

void TupleManager::selectAll(const SelectAllOperation* operation) {
    QString dbName = operation->dbName;
    QString tableName = operation->tableName;

    qDebug() << "成功进入查询*的Manager";

    try {
        // 读取表的字段定义
        std::vector<FieldBlock> fields = FileUtil::readTableFields(dbName, tableName);
        // 读取表的所有记录
        std::vector<DataRow> rows = FileUtil::readAllDataRows(dbName, tableName);

        // 检查字段数是否匹配
        if (!fields.empty() && !rows.empty() && fields.size() != rows[0].header.fieldCount) {
            throw std::runtime_error("Field count mismatch between table definition and data");
        }

        /**************** 生成表格样式消息 ****************/
        QString message = "SELECT_RESPONSE\n";

        // 生成分隔线模板（例如：+---------------------+------------+----------+----------------+）
        auto generateSeparator = [&]() {
            QString separator = "+";
            for (const auto& field : fields) {
                // 根据字段名动态计算列宽（示例固定为20字符）
                separator += QString("-").repeated(20) + "+";
            }
            return separator + "\n";
        };

        // 首部分隔线
        message += generateSeparator();

        // 表头行
        message += "|";
        for (const auto& field : fields) {
            QString header = QString::fromUtf8(field.name);
            message += " " + header.leftJustified(18, ' ') + " |"; // 固定18字符宽度+两侧空格
        }
        message += "\n";

        // 表头后分隔线
        message += generateSeparator();

        // 数据行
        for (const auto& row : rows) {
            message += "|";
            for (int i = 0; i < row.header.fieldCount; i++) {
                FieldValue value = row.values[i];
                QString valueStr;

                // 类型转换（保持原有逻辑）
                switch (value.type) {
                case DT_INTEGER: valueStr = QString::number(value.intVal); break;
                case DT_BOOL: valueStr = value.boolVal ? "true" : "false"; break;
                case DT_DOUBLE: valueStr = QString::number(value.doubleVal, 'f', 2); break;
                case DT_VARCHAR: valueStr = QString::fromUtf8(value.varcharVal); break;
                case DT_DATETIME: valueStr = //value.varcharVal;
                    valueStr = QDateTime::fromSecsSinceEpoch(value.intVal).toString("yyyy-MM-dd HH:mm:ss");
                    break;
                default: valueStr = "UNKNOWN";
                }

                // 对齐数据（与表头格式一致）
                message += " " + valueStr.leftJustified(18, ' ') + " |";
            }
            message += "\n";
        }

        // 最终分隔线
        message += generateSeparator();

        // 显示消息
        widget->showMessage(message);
        // 写入日志
        FileUtil::appendLogRecord(operation->dbName , operation->logRecord);
    } catch (const std::exception& e) {
        widget->showMessage("Error: " + QString::fromStdString(e.what()));
    }
}






// 删除记录
void TupleManager::deleteRows(const DeleteOperation* op) {
    // 1. 读取表的所有记录
    std::vector<DataRow> allRows = FileUtil::readAllDataRows(op->dbName, op->tableName);

    // 2. 过滤出要保留的记录
    std::vector<DataRow> remainingRows;
    std::vector<FieldBlock> fields = FileUtil::readTableFields(op->dbName, op->tableName);

    for (const DataRow& row : allRows) {
        bool shouldDelete = true;

        // 检查是否满足所有条件
        for (const Condition& cond : op->conditions) {
            // 查找字段索引
            int fieldIndex = -1;
            for (size_t i = 0; i < fields.size(); i++) {
                if (strcmp(fields[i].name, cond.fieldName) == 0) {
                    fieldIndex = i;
                    break;
                }
            }

            if (fieldIndex == -1 || fieldIndex >= row.values.size()) {
                shouldDelete = false;
                break;
            }

            if (!cond.evaluate(row.values[fieldIndex])) {
                shouldDelete = false;
                break;
            }
        }

        if (!shouldDelete) {
            remainingRows.push_back(row);
        }
    }

    // 3. 更新表记录数
    auto tables = FileUtil::readAllTableBlocks(op->dbName);
    for (auto& table : tables) {
        if (strcmp(table.name, op->tableName.toUtf8().constData()) == 0) {
            table.record_num = remainingRows.size();
            table.mtime = QDateTime::currentSecsSinceEpoch();
            break;
        }
    }

    // 4. 重写.trd文件
    QString trdPath = FileUtil::generateTableFilePath(op->dbName, op->tableName, "trd");
    QFile trdFile(trdPath);
    if (!trdFile.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("无法打开记录文件");
    }

    for (const DataRow& row : remainingRows) {
        trdFile.write(reinterpret_cast<const char*>(&row.header), sizeof(DataRowHeader));
        trdFile.write(reinterpret_cast<const char*>(row.values.data()),
                      row.values.size() * sizeof(FieldValue));
    }

    trdFile.close();

    // 5. 更新.tb文件
    FileUtil::updateTableBlocks(op->dbName, tables);

    widget->showMessage(QString("成功删除%1条记录").arg(allRows.size() - remainingRows.size()));
    // 写入日志
    FileUtil::appendLogRecord(op->dbName , op->logRecord);

}

// 格式化
// 在 selectColumns() 的结果格式化部分：
QString formatFunctionValue(const FieldValue& val) {
    switch (val.type) {
    case DT_INTEGER:
        return QString::number(val.intVal);
    case DT_DOUBLE:
        return QString::number(val.doubleVal, 'f',
                               (val.doubleVal == floor(val.doubleVal)) ? 0 : 2); // 自动判断小数位
    default:
        return "NULL";
    }
}
// 使用示例：
// message += QString("| %1 ")
//                .arg(formatFunctionValue(funcResult).leftJustified(15, ' '));

// 查询部分记录
// void TupleManager::selectColumns(const SelectColumnsOperation* op) {
//     // 1. 读取元数据
//     auto fields = FileUtil::readTableFields(op->dbName, op->tableName);
//     auto allRows = FileUtil::readAllDataRows(op->dbName, op->tableName);

//     std::vector<bool> isAggregateColumn;

//     // 2. 验证列名并构建索引映射
//     std::vector<int> columnIndices;
//     for (const QString& col : op->columns) {
//         bool found = false;
//         for (size_t i = 0; i < fields.size(); ++i) {
//             if (QString::fromUtf8(fields[i].name) == col.trimmed()) {
//                 columnIndices.push_back(i);
//                 isAggregateColumn.push_back(fields[i].isAggregateFunc);
//                 found = true;
//                 break;
//             }
//         }
//         if (!found) throw std::runtime_error("Column not found: " + col.toStdString());
//     }

//     // 2.5 Apply WHERE filter first
//     std::vector<DataRow> resultRows;
//     for (const auto& row : allRows) {
//         bool matchAllConditions = true;

//         // WHERE条件判断
//         for (const auto& cond : op->conditions) {
//             int fieldIdx = -1;
//             for (size_t i = 0; i < fields.size(); i++) {
//                 if (strcmp(fields[i].name, cond.fieldName) == 0) {
//                     fieldIdx = i;
//                     break;
//                 }
//             }

//             // if (fieldIdx == -1 || !cond.evaluate(row.values[fieldIdx])) {
//             //     matchAllConditions = false;
//             //     break;
//             // }  // 无条件则抛出异常
//             // if (fieldIdx == -1) continue; // 跳过无效条件
//             if (fieldIdx == -1) {
//                 matchAllConditions = false;
//                 break;
//             }
//         }

//         if (matchAllConditions) {
//             resultRows.push_back(row);
//         }
//     }

//     // Now decide whether to compute aggregates or just print values
//     if (std::any_of(isAggregateColumn.begin(), isAggregateColumn.end(), [](bool b){ return b; })) {
//         TupleManager manager;
//         manager.handleAggregateSelect(op, fields, allRows, columnIndices);
//     } else {
//         TupleManager manager;
//         manager.handleRegularSelect(op, fields, allRows, columnIndices);
//     }

//     // // 3. 过滤数据
//     // std::vector<DataRow> resultRows;
//     // for (const auto& row : allRows) {
//     //     bool matchAllConditions = true;

//     //     // WHERE条件判断
//     //     for (const auto& cond : op->conditions) {
//     //         int fieldIdx = -1;
//     //         for (size_t i = 0; i < fields.size(); i++) {
//     //             if (strcmp(fields[i].name, cond.fieldName) == 0) {
//     //                 fieldIdx = i;
//     //                 break;
//     //             }
//     //         }

//     //         // if (fieldIdx == -1 || !cond.evaluate(row.values[fieldIdx])) {
//     //         //     matchAllConditions = false;
//     //         //     break;
//     //         // }  // 无条件则抛出异常
//     //         if (fieldIdx == -1) continue; // 跳过无效条件
//     //     }

//     //     if (matchAllConditions) {
//     //         resultRows.push_back(row);
//     //     }
//     // }

//     // 4. 构建输出
//     QString message = QString("+-----------------").repeated(columnIndices.size()) + "+\n";

//     // 表头
//     for (const QString& col : op->columns) {
//         message += QString("| %1 ").arg(col.leftJustified(15, ' '));
//     }
//     message += "|\n" + QString("+-----------------").repeated(columnIndices.size()) + "+\n";

//     if (resultRows.empty()) {
//         message = "No rows match the conditions";
//     } // 友好提示

//     // 数据行
//     for (const auto& row : resultRows) {
//         for (int colIdx : columnIndices) {
//             const FieldValue& val = row.values[colIdx];;
//             /* 值格式化逻辑（见上文实现） */
//             TupleManager tempManager;
//             QString str = tempManager.formatFieldValue(row.values[colIdx]);
//             message += QString("| %1 ").arg(str.leftJustified(15, ' '));
//         }
//         message += "|\n";
//     }
//     message += QString("+-----------------").repeated(columnIndices.size()) + "+";

//     widget->showMessage(message);
// }

void TupleManager::selectColumns(const SelectColumnsOperation* op) {
    auto fields = FileUtil::readTableFields(op->dbName, op->tableName);
    auto rows = FileUtil::readAllDataRows(op->dbName, op->tableName);

    std::vector<int> realColumnIndices;     // Indices of real table fields
    std::vector<FieldBlock> virtualFields;   // Virtual fields like MAX(age)
    std::vector<bool> isAggregateColumn;

    // 1. Parse all requested columns
    for (const QString& col : op->columns) {
        bool found = false;

        // First try to match column name against actual schema
        for (size_t i = 0; i < fields.size(); ++i) {
            if (QString::fromUtf8(fields[i].name) == col.trimmed()) {
                realColumnIndices.push_back(i);
                isAggregateColumn.push_back(fields[i].isAggregateFunc);
                found = true;
                break;
            }
        }

        // If not found, check if it's an aggregate function
        if (!found) {
            QRegularExpression funcRegex("(\\w+)\\(\\s*([\\w\\*]+)\\s*\\)");
            QRegularExpressionMatch match = funcRegex.match(col);

            if (match.hasMatch()) {
                FieldBlock virtualField;
                memset(&virtualField, 0, sizeof(FieldBlock));

                QString funcName = match.captured(1).toUpper();
                QString fieldName = match.captured(2).trimmed();

                strncpy(virtualField.name, col.toUtf8().constData(), 127);
                virtualField.isAggregateFunc = true;

                // Fill FunctionCall struct
                strncpy(virtualField.func.funcName, funcName.toUtf8().constData(), 31);
                strncpy(virtualField.func.fieldName, fieldName.toUtf8().constData(), 127);

                if (funcName == "COUNT") virtualField.func.funcType = FT_COUNT;
                else if (funcName == "MAX") virtualField.func.funcType = FT_MAX;
                else if (funcName == "MIN") virtualField.func.funcType = FT_MIN;
                else if (funcName == "AVG") virtualField.func.funcType = FT_AVG;
                else if (funcName == "SUM") virtualField.func.funcType = FT_SUM;

                virtualField.type = (funcName == "COUNT") ? DT_INTEGER : DT_DOUBLE;

                virtualFields.push_back(virtualField);
                isAggregateColumn.push_back(true);
                found = true;
            }
        }

        if (!found) {
            throw std::runtime_error("Column not found: " + col.toStdString());
        }
    }

    // 2. Filter rows using WHERE clause
    std::vector<DataRow> resultRows;
    TupleManager temp;
    for (const auto& row : rows) {
        if (temp.matchesWhereConditions(row, fields, op->conditions)) {
            resultRows.push_back(row);
        }
    }

    // 3. Decide whether to compute aggregates
    bool hasAggregates = std::any_of(isAggregateColumn.begin(), isAggregateColumn.end(),
                                     [](bool b){ return b; });

    if (hasAggregates) {
        temp.handleAggregateSelect(op, fields, virtualFields, resultRows);
    } else {
        temp.handleRegularSelect(op, fields, resultRows, realColumnIndices);
    }
    // 写入日志
    FileUtil::appendLogRecord(op->dbName , op->logRecord);
}
// void TupleManager::selectColumns(const SelectColumnsOperation* op) {
//     auto fields = FileUtil::readTableFields(op->dbName, op->tableName);
//     auto rows = FileUtil::readAllDataRows(op->dbName, op->tableName);

//     std::vector<int> columnIndices;
//     std::vector<bool> isAggregateColumn;
//     std::vector<FieldBlock> virtualFields; // For aggregates only

//     for (const QString& col : op->columns) {
//         bool found = false;
//         if (is virtual field) {
//             virtualFields.push_back(...);
//         }

//         // First try to find in actual table schema
//         for (size_t i = 0; i < fields.size(); ++i) {
//             if (QString::fromUtf8(fields[i].name) == col.trimmed()) {
//                 columnIndices.push_back(i);
//                 isAggregateColumn.push_back(fields[i].isAggregateFunc);
//                 found = true;
//                 break;
//             }
//         }

//         if (std::any_of(isAggregateColumn.begin(), isAggregateColumn.end(),
//                         [](bool b){ return b; })) {
//             temp.handleAggregateSelect(op, fields, virtualFields, resultRows);
//         } else {
//             temp.handleRegularSelect(op, fields, resultRows, columnIndices);
//         }

//         // If not found, check if it's a function expression
//         if (!found) {
//             QRegularExpression funcRegex("(\\w+)\\(\\s*([\\w\\*]+)\\s*\\)");
//             QRegularExpressionMatch match = funcRegex.match(col);

//             if (match.hasMatch()) {
//                 FieldBlock virtualField;
//                 memset(&virtualField, 0, sizeof(FieldBlock));

//                 QString funcName = match.captured(1).toUpper();
//                 QString fieldName = match.captured(2).trimmed();

//                 strncpy(virtualField.name, col.toUtf8().constData(), 127);
//                 virtualField.isAggregateFunc = true;

//                 // Fill FunctionCall struct
//                 strncpy(virtualField.func.funcName, funcName.toUtf8().constData(), 31);
//                 strncpy(virtualField.func.fieldName, fieldName.toUtf8().constData(), 127);

//                 if (funcName == "COUNT") virtualField.func.funcType = FT_COUNT;
//                 else if (funcName == "MAX") virtualField.func.funcType = FT_MAX;
//                 else if (funcName == "MIN") virtualField.func.funcType = FT_MIN;
//                 else if (funcName == "AVG") virtualField.func.funcType = FT_AVG;
//                 else if (funcName == "SUM") virtualField.func.funcType = FT_SUM;

//                 virtualField.type = (funcName == "COUNT") ? DT_INTEGER : DT_DOUBLE;

//                 virtualFields.push_back(virtualField);
//                 isAggregateColumn.push_back(true);
//                 columnIndices.push_back(-1); // Placeholder for real field index
//                 found = true;
//             }
//         }

//         if (!found) {
//             throw std::runtime_error("Column not found: " + col.toStdString());
//         }
//     }

//     // for (const QString& col : op->columns) {
//     //     bool found = false;
//     //     for (size_t i = 0; i < fields.size(); ++i) {
//     //         if (QString::fromUtf8(fields[i].name) == col.trimmed()) {
//     //             columnIndices.push_back(i);
//     //             isAggregateColumn.push_back(fields[i].isAggregateFunc);
//     //             found = true;
//     //             break;
//     //         }
//     //     }
//     //     if (!found) throw std::runtime_error("Column not found: " + col.toStdString());
//     // }

//     // Filter rows using WHERE clause
//     std::vector<DataRow> resultRows;
//     TupleManager temp;
//     for (const auto& row : rows) {

//         if (temp.matchesWhereConditions(row, fields, op->conditions)) {
//             resultRows.push_back(row);
//         }
//     }

//     // Decide whether to compute aggregates
//     if (std::any_of(isAggregateColumn.begin(), isAggregateColumn.end(),
//                     [](bool b){ return b; })) {
//         temp.handleAggregateSelect(op, fields, resultRows, columnIndices);
//     } else {
//         temp.handleRegularSelect(op, fields, resultRows, columnIndices);
//     }
// }

// simplied sample
// void TupleManager::selectColumns(const SelectColumnsOperation* op) {
//     auto fields = FileUtil::readTableFields(op->dbName, op->tableName);
//     auto allRows = FileUtil::readAllDataRows(op->dbName, op->tableName);

//     std::vector<int> columnIndices;
//     std::vector<bool> isAggregateColumn;

//     for (const QString& col : op->columns) {
//         bool found = false;
//         for (size_t i = 0; i < fields.size(); ++i) {
//             if (QString::fromUtf8(fields[i].name) == col.trimmed()) {
//                 columnIndices.push_back(i);
//                 isAggregateColumn.push_back(fields[i].isAggregateFunc);
//                 found = true;
//                 break;
//             }
//         }
//         if (!found) throw std::runtime_error("Column not found: " + col.toStdString());
//     }

//     // Filter rows based on WHERE condition
//     std::vector<DataRow> resultRows;
//     for (const auto& row : allRows) {
//         if (matchesWhereConditions(row, fields, op->conditions)) {
//             resultRows.push_back(row);
//         }
//     }

//     // Decide between aggregate and regular select
//     if (std::any_of(isAggregateColumn.begin(), isAggregateColumn.end(), [](bool b){ return b; })) {
//         handleAggregateSelect(op, fields, resultRows, columnIndices);
//     } else {
//         handleRegularSelect(op, fields, resultRows, columnIndices);
//     }
// }

// QString TupleManager::formatFieldValue(const FieldValue& val) {
//     switch (val.type) {
//     case DT_INTEGER: return QString::number(val.intVal);
//     case DT_BOOL: return val.boolVal ? "TRUE" : "FALSE";
//     case DT_DOUBLE: return QString::number(val.doubleVal, 'f',
//                                (val.doubleVal == floor(val.doubleVal)) ? 0 : 2);
//     case DT_VARCHAR: return QString::fromUtf8(val.varcharVal);
//     case DT_DATETIME: return QDateTime::fromSecsSinceEpoch(val.intVal).toString(Qt::ISODate);
//     default: return "NULL";
//     }
// }

// // 新增函数计算结果的方法
// FieldValue TupleManager::calculateFunction(
//     const FunctionCall& func,
//     const std::vector<FieldBlock>& fields,
//     const std::vector<DataRow>& rows
//     ) {



//     FieldValue result;
//     memset(&result, 0, sizeof(FieldValue));

//     // 查找目标字段索引
//     int targetFieldIndex = -1;
//     if (strcmp(func.fieldName, "*") != 0) {
//         for (size_t i = 0; i < fields.size(); i++) {
//             if (strcmp(fields[i].name, func.fieldName) == 0) {
//                 targetFieldIndex = i;
//                 break;
//             }
//         }
//         if (targetFieldIndex == -1) return result; // 无效字段
//     }

//     // 安全检查
//     DataType expectedType;
//     switch (func.funcType) {
//     case FT_AVG:
//     case FT_SUM:
//         switch (fields[targetFieldIndex].type) {
//         case DT_INTEGER: expectedType = DT_INTEGER;
//         case DT_DOUBLE: expectedType = DT_DOUBLE;
//         default: expectedType = DT_NULL;
//         }

//         // expectedType = (fields[targetFieldIndex].type == DT_INTEGER ||
//         //                 fields[targetFieldIndex].type == DT_DOUBLE) ?
//         //                    fields[targetFieldIndex].type : DT_NULL;
//         if (expectedType == DT_NULL) {
//             throw std::runtime_error(
//                 QString("Function %1 cannot apply to non-numeric field: %2")
//                     .arg(func.funcName).arg(func.fieldName).toStdString()
//                 );
//         }
//         break;
//         // ...其他函数检查
//     }

//     // 执行计算
//     switch (func.funcType) {
//     case FT_COUNT:
//         result.type = DT_INTEGER;
//         result.intVal = (targetFieldIndex == -1) ?
//                             rows.size() : // COUNT(*)
//                             std::count_if(rows.begin(), rows.end(),
//                                           [&](const DataRow& row) {
//                                               return row.values[targetFieldIndex].type != DT_NULL;
//                                           });
//         break;

//     case FT_MAX: {

//         if (targetFieldIndex == -1) break;
//         auto maxIt = std::max_element(rows.begin(), rows.end(),
//                                       [&](const DataRow& a, const DataRow& b) {
//                                           return compareFieldValues(
//                                                      a.values[targetFieldIndex],
//                                                      b.values[targetFieldIndex]) < 0;
//                                       });
//         result = maxIt->values[targetFieldIndex];
//         break;
//     }
//         // ...其他函数实现
//     case FT_MIN: {

//         if (targetFieldIndex == -1) break;
//         auto minIt = std::min_element(rows.begin(), rows.end(),
//                                       [&](const DataRow& a, const DataRow& b) {
//                                           return compareFieldValues(
//                                                      a.values[targetFieldIndex],
//                                                      b.values[targetFieldIndex]) < 0;
//                                       });
//         result = minIt->values[targetFieldIndex];
//         break;
//     }

//     case FT_AVG: {
//         if (targetFieldIndex == -1) break;

//         double total = 0.0;
//         int validCount = 0;

//         for (const auto& row : rows) {
//             const FieldValue& val = row.values[targetFieldIndex];
//             if (val.type == DT_NULL) continue;

//             switch (val.type) {
//             case DT_INTEGER: total += val.intVal; break;
//             case DT_DOUBLE:  total += val.doubleVal; break;
//             default: continue; // 非数值类型跳过
//             }
//             validCount++;
//         }

//         if (validCount > 0) {
//             result.type = DT_DOUBLE;
//             result.doubleVal = total / validCount;
//         }
//         break;
//     }

//     case FT_SUM: {
//         if (targetFieldIndex == -1) break;

//         double sum = 0.0;
//         bool hasValue = false;

//         for (const auto& row : rows) {
//             const FieldValue& val = row.values[targetFieldIndex];
//             if (val.type == DT_NULL) continue;

//             switch (val.type) {
//             case DT_INTEGER: sum += val.intVal; break;
//             case DT_DOUBLE:  sum += val.doubleVal; break;
//             default: continue;
//             }
//             hasValue = true;
//         }

//         if (hasValue) {
//             result.type = (fields[targetFieldIndex].type == DT_INTEGER) ?
//                               DT_INTEGER : DT_DOUBLE;
//             if (result.type == DT_INTEGER) result.intVal = static_cast<qint32>(sum);
//             else result.doubleVal = sum;
//         }
//         break;
//     }
//     }

//     return result;
// }
FieldValue TupleManager::calculateFunction(
    const FieldBlock& funcField,
    const std::vector<FieldBlock>& allFields,
    const std::vector<DataRow>& rows)
{
    FieldValue result;
    memset(&result, 0, sizeof(FieldValue));

    if (!funcField.isAggregateFunc) return result;

    const FunctionCall& func = funcField.func;
    int targetIndex = -1;

    if (strcmp(func.fieldName, "*") != 0) {
        for (size_t i = 0; i < allFields.size(); ++i) {
            if (strcmp(allFields[i].name, func.fieldName) == 0) {
                targetIndex = i;
                break;
            }
        }
        if (targetIndex == -1) {
            throw std::runtime_error("Field not found");
        }
    }


    switch (func.funcType) {
    case FT_COUNT:
        if (targetIndex == -1) {
            result.type = DT_INTEGER;
            result.intVal = rows.size();
        } else {
            result.type = DT_INTEGER;
            result.intVal = std::count_if(rows.begin(), rows.end(),
                                          [&](const DataRow& r) {
                                              return r.values[targetIndex].type != DT_NULL;
                                          });
        }
        break;

    case FT_MAX:
        if (targetIndex >= 0 && !rows.empty()) {
            auto maxIt = std::max_element(rows.begin(), rows.end(),
                                          [targetIndex](const DataRow& a, const DataRow& b) {
                                              TupleManager temp;
                                              return temp.compareFieldValues(a.values[targetIndex],
                                                                             b.values[targetIndex]) < 0;
                                          });
            result = maxIt->values[targetIndex];
        }
        break;

    case FT_MIN:
        if (targetIndex >= 0 && !rows.empty()) {
            auto minIt = std::min_element(rows.begin(), rows.end(),
                                          [targetIndex](const DataRow& a, const DataRow& b) {
                                              TupleManager temp;
                                              return temp.compareFieldValues(a.values[targetIndex],
                                                                             b.values[targetIndex]) < 0;
                                          });
            result = minIt->values[targetIndex];
        }
        break;

    case FT_AVG:
    case FT_SUM:
        double total = 0.0;
        int count = 0;
        for (const auto& row : rows) {
            FieldValue val = row.values[targetIndex];
            if (val.type == DT_INTEGER) {
                total += val.intVal;
                count++;
            } else if (val.type == DT_DOUBLE) {
                total += val.doubleVal;
                count++;
            }
        }
        if (count > 0) {
            result.type = (func.funcType == FT_AVG) ? DT_DOUBLE : DT_INTEGER;
            result.doubleVal = (func.funcType == FT_AVG) ? total / count : total;
        }
        break;

        // default:
        //     throw std::runtime_error("Unsupported function"); // 什么鬼问题
    }

    return result;
}

// 更新数据
void TupleManager::update(const UpdateOperation* op) {
    // 1. 读取表的所有记录
    std::vector<DataRow> allRows = FileUtil::readAllDataRows(op->dbName, op->tableName);
    std::vector<FieldBlock> fields = FileUtil::readTableFields(op->dbName, op->tableName);

    // 2. 找出要更新的行并更新
    int updatedCount = 0;
    for (auto& row : allRows) {
        TupleManager temp;
        if (temp.matchesWhereConditions(row, fields, op->conditions)) {
            // 更新匹配的行
            for (const auto& setClause : op->setClauses) {
                // 查找字段索引
                int fieldIndex = -1;
                for (size_t i = 0; i < fields.size(); i++) {
                    if (strcmp(fields[i].name, setClause.fieldName) == 0) {
                        fieldIndex = i;
                        break;
                    }
                }

                if (fieldIndex != -1 && fieldIndex < row.values.size()) {
                    // 执行更新
                    row.values[fieldIndex] = setClause.newValue;
                    updatedCount++;
                }
            }
        }
    }

    // 3. 将更新后的数据写回文件
    QString trdPath = FileUtil::generateTableFilePath(op->dbName, op->tableName, "trd");
    QFile trdFile(trdPath);
    if (!trdFile.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("无法打开记录文件");
    }

    for (const DataRow& row : allRows) {
        trdFile.write(reinterpret_cast<const char*>(&row.header), sizeof(DataRowHeader));
        trdFile.write(reinterpret_cast<const char*>(row.values.data()),
                      row.values.size() * sizeof(FieldValue));
    }
    trdFile.close();

    // 4. 更新表的修改时间
    auto tables = FileUtil::readAllTableBlocks(op->dbName);
    for (auto& table : tables) {
        if (strcmp(table.name, op->tableName.toUtf8().constData()) == 0) {
            table.mtime = QDateTime::currentSecsSinceEpoch();
            break;
        }
    }
    FileUtil::updateTableBlocks(op->dbName, tables);

    widget->showMessage(QString("成功更新%1条记录").arg(updatedCount));
    // 写入日志
    FileUtil::appendLogRecord(op->dbName , op->logRecord);
}

// FieldValue TupleManager::calculateFunction(
//     const FieldBlock& funcField,
//     const std::vector<FieldBlock>& allFields,
//     const std::vector<DataRow>& rows
//     ) {
//     FieldValue result;
//     memset(&result, 0, sizeof(FieldValue));

//     // 如果不是聚合函数字段，直接返回空值
//     if (!funcField.isAggregateFunc) return result;

//     // 获取函数信息（直接从FieldBlock中读取）
//     const FunctionCall& func = funcField.func;

//     // 查找目标字段索引（COUNT(*) 除外）
//     int targetFieldIndex = -1;
//     if (strcmp(func.fieldName, "*") != 0) {
//         for (size_t i = 0; i < allFields.size(); i++) {
//             if (strcmp(allFields[i].name, func.fieldName) == 0) {
//                 targetFieldIndex = i;
//                 break;
//             }
//         }
//         if (targetFieldIndex == -1) {
//             throw std::runtime_error(
//                 QString("Field not found: %1").arg(func.fieldName).toStdString()
//                 );
//         }
//     }



//     // 类型安全检查
//     switch (func.funcType) {
//     case FT_COUNT:
//         // COUNT 不限制字段类型
//         break;

//     case FT_MAX:
//     case FT_MIN:
//         // 可比较类型：数值、字符串、日期时间
//         if (targetFieldIndex != -1 &&
//             allFields[targetFieldIndex].type == DT_BOOL) {
//             throw std::runtime_error(
//                 QString("Function %1 cannot apply to boolean field: %2")
//                     .arg(func.funcName).arg(func.fieldName).toStdString()
//                 );
//         }
//         break;

//     case FT_AVG:
//     case FT_SUM:
//         // 仅限数值类型
//         if (targetFieldIndex != -1 &&
//             !(allFields[targetFieldIndex].type == DT_INTEGER ||
//               allFields[targetFieldIndex].type == DT_DOUBLE)) {
//             throw std::runtime_error(
//                 QString("Function %1 requires numeric field: %2")
//                     .arg(func.funcName).arg(func.fieldName).toStdString()
//                 );
//         }
//         break;
//     }

//     // 执行计算
//     switch (func.funcType) {
//     case FT_COUNT: {
//         result.type = DT_INTEGER;
//         if (targetFieldIndex == -1) {
//             // COUNT(*)
//             result.intVal = rows.size();
//         } else {
//             // COUNT(field)
//             result.intVal = std::count_if(rows.begin(), rows.end(),
//                                           [&](const DataRow& row) {
//                                               return row.values[targetFieldIndex].type != DT_NULL;
//                                           });
//         }
//         break;
//     }

//     case FT_MAX: {
//         if (targetFieldIndex == -1 || rows.empty()) break;

//         auto maxIt = std::max_element(rows.begin(), rows.end(),
//                                       [&](const DataRow& a, const DataRow& b) {
//                                           return compareFieldValues(
//                                                      a.values[targetFieldIndex],
//                                                      b.values[targetFieldIndex]) < 0;
//                                       });

//         // 处理全NULL的情况
//         if (maxIt->values[targetFieldIndex].type != DT_NULL) {
//             result = maxIt->values[targetFieldIndex];
//         }
//         break;
//     }

//     case FT_MIN: {
//         if (targetFieldIndex == -1 || rows.empty()) break;

//         auto minIt = std::min_element(rows.begin(), rows.end(),
//                                       [&](const DataRow& a, const DataRow& b) {
//                                           return compareFieldValues(
//                                                      a.values[targetFieldIndex],
//                                                      b.values[targetFieldIndex]) < 0;
//                                       });

//         if (minIt->values[targetFieldIndex].type != DT_NULL) {
//             result = minIt->values[targetFieldIndex];
//         }
//         break;
//     }

//     case FT_AVG: {
//         if (targetFieldIndex == -1) break;

//         double total = 0.0;
//         int validCount = 0;
//         bool isIntegerSource = (allFields[targetFieldIndex].type == DT_INTEGER);

//         for (const auto& row : rows) {
//             const FieldValue& val = row.values[targetFieldIndex];
//             if (val.type == DT_NULL) continue;

//             switch (val.type) {
//             case DT_INTEGER:
//                 total += val.intVal;
//                 validCount++;
//                 break;
//             case DT_DOUBLE:
//                 total += val.doubleVal;
//                 validCount++;
//                 break;
//             default:
//                 continue;
//             }
//         }

//         if (validCount > 0) {
//             result.type = DT_DOUBLE;
//             result.doubleVal = total / validCount;
//             // 如果源都是整数且能整除，转为整数
//             if (isIntegerSource && floor(result.doubleVal) == result.doubleVal) {
//                 result.type = DT_INTEGER;
//                 result.intVal = static_cast<qint32>(result.doubleVal);
//             }
//         }
//         break;
//     }

//     case FT_SUM: {
//         if (targetFieldIndex == -1) break;

//         double sum = 0.0;
//         bool hasValue = false;
//         bool isIntegerSource = (allFields[targetFieldIndex].type == DT_INTEGER);
//         bool overflow = false;

//         for (const auto& row : rows) {
//             const FieldValue& val = row.values[targetFieldIndex];
//             if (val.type == DT_NULL) continue;

//             switch (val.type) {
//             case DT_INTEGER:
//                 if (isIntegerSource) {
//                     // 检查整数溢出
//                     if (val.intVal > 0 &&
//                         sum > std::numeric_limits<qint32>::max() - val.intVal) {
//                         overflow = true;
//                     } else if (val.intVal < 0 &&
//                                sum < std::numeric_limits<qint32>::min() - val.intVal) {
//                         overflow = true;
//                     }
//                 }
//                 sum += val.intVal;
//                 hasValue = true;
//                 break;
//             case DT_DOUBLE:
//                 sum += val.doubleVal;
//                 hasValue = true;
//                 break;
//             default:
//                 continue;
//             }
//         }

//         if (hasValue) {
//             if (isIntegerSource && !overflow) {
//                 result.type = DT_INTEGER;
//                 result.intVal = static_cast<qint32>(sum);
//             } else {
//                 result.type = DT_DOUBLE;
//                 result.doubleVal = sum;
//             }
//         }
//         break;
//     }

//     default:
//         // result.type = DT_NULL;
//         throw std::runtime_error(
//             QString("Unsupported function: %1").arg(func.funcName).toStdString()
//             );
//     }

//     return result;
// }

// 比较两个FieldValue
int TupleManager::compareFieldValues(const FieldValue& v1, const FieldValue& v2) const{
    if (v1.type != v2.type) return -1; // 类型不同

    switch (v1.type) {
    case DT_INTEGER:
        return v1.intVal - v2.intVal;
    case DT_BOOL:
        return v1.boolVal - v2.boolVal;
    case DT_DOUBLE:
        return (v1.doubleVal < v2.doubleVal) ? -1 : (v1.doubleVal > v2.doubleVal) ? 1 : 0;
    case DT_VARCHAR:
        return strcmp(v1.varcharVal, v2.varcharVal);
    case DT_DATETIME:
        return (v1.intVal < v2.intVal) ? -1 : (v1.intVal > v2.intVal) ? 1 : 0;
    default:
        return -1;
    }
    // Edge Case
    //             If types don't match (v1.type != v2.type), returns -1. This could lead to incorrect comparisons like "5" < 3 being true.

    //             Fix suggestion:
    //                              Try type coercion before comparing different types (if needed by your SQL dialect).
}

// int compareFieldValues(const FieldValue& a, const FieldValue& b) {
//     if (a.type != b.type) return a.type - b.type;

//     switch (a.type) {
//     case DT_INTEGER: return (a.intVal < b.intVal) ? -1 : (a.intVal > b.intVal) ? 1 : 0;
//     case DT_DOUBLE: return (a.doubleVal < b.doubleVal) ? -1 : (a.doubleVal > b.doubleVal) ? 1 : 0;
//     case DT_VARCHAR: return strcmp(a.varcharVal, b.varcharVal);
//     default: return 0;
//     }
// }

QString TupleManager::formatValue(const FieldValue& val, DataType type) {
    switch (type) {
    case DT_INTEGER: return QString::number(val.intVal);
    case DT_DOUBLE:  return QString::number(val.doubleVal, 'f', 2);
    case DT_VARCHAR: return QString::fromUtf8(val.varcharVal);
    // ... 其他类型处理 ...
    default: return "NULL";
    }
}



// void TupleManager::handleAggregateSelect(
//     const SelectColumnsOperation* op,
//     const std::vector<FieldBlock>& fields,
//     const std::vector<DataRow>& rows,
//     const std::vector<int>& columnIndices)
// {
//     QStringList headerLabels;
//     QList<FieldValue> results;

//     for (int idx : columnIndices) {
//         const FieldBlock& field = fields[idx];
//         if (field.isAggregateFunc) {
//             headerLabels << field.name;
//             results.append(calculateFunction(field, fields, rows));
//         }
//     }

//     // Format and display the result
//     QString message = "+-" + QString("-+-").repeated(headerLabels.size() - 1) + "-+\n";
//     message += "| " + headerLabels.join(" | ") + " |\n";
//     message += "+-" + QString("-+-").repeated(headerLabels.size() - 1) + "-+\n";


//     message += "| ";
//     for (const FieldValue& val : results) {
//         message += formatFunctionValue(val) + " | ";
//     }
//     message += "\n+-------------------------------------------+\n";

//     widget->showMessage(message);
// }
// void TupleManager::handleAggregateSelect(
//     const SelectColumnsOperation* op,
//     const std::vector<FieldBlock>& fields,
//     const std::vector<DataRow>& rows,
//     const std::vector<int>& columnIndices)
// {
//     QStringList headerLabels;
//     QList<FieldValue> results;

//     for (int idx : columnIndices) {
//         const FieldBlock& field = fields[idx];
//         if (field.isAggregateFunc) {
//             headerLabels << field.name;
//             results.append(calculateFunction(field, fields, rows));
//         }
//     }

//     // Format output
//     QString message = "+-" + QString("-+-").repeated(headerLabels.size() - 1) + "-+\n";
//     message += "| " + headerLabels.join(" | ") + " |\n";
//     message += "+-" + QString("-+-").repeated(headerLabels.size() - 1) + "-+\n";

//     message += "| ";
//     for (const FieldValue& val : results) {
//         message += formatFunctionValue(val) + " | ";
//     }
//     message += "\n+-------------------------------------------+\n";

//     widget->showMessage(message);
// }
void TupleManager::handleAggregateSelect(
    const SelectColumnsOperation* op,
    const std::vector<FieldBlock>& allFields,
    const std::vector<FieldBlock>& virtualFields,
    const std::vector<DataRow>& rows)
{
    QStringList headerLabels;
    QList<FieldValue> results;

    // Process each virtual field
    for (const auto& field : virtualFields) {
        headerLabels << field.name;
        results.append(calculateFunction(field, allFields, rows));
    }

    // Format result
    QString message = "+-" + QString("-+-").repeated(headerLabels.size() - 1) + "-+\n";
    message += "| " + headerLabels.join(" | ") + " |\n";
    message += "+-" + QString("-+-").repeated(headerLabels.size() - 1) + "-+\n";

    message += "| ";
    for (const FieldValue& val : results) {
        message += formatFunctionValue(val) + " | ";
    }
    message += "\n+-------------------------------------------+\n";

    widget->showMessage(message);
}

QString TupleManager::formatFunctionValue(const FieldValue& val) {
    switch (val.type) {
    case DT_INTEGER: return QString::number(val.intVal);
    case DT_DOUBLE: return QString::number(val.doubleVal, 'f', (val.doubleVal == floor(val.doubleVal)) ? 0 : 2);
    default: return "NULL";
    }
}

QString TupleManager::formatFieldValue(const FieldValue& val) {
    switch (val.type) {
    case DT_INTEGER:
        return QString::number(val.intVal);
    case DT_BOOL:
        return val.boolVal ? "TRUE" : "FALSE";
    case DT_DOUBLE:
        return QString::number(val.doubleVal, 'f',
                               (val.doubleVal == floor(val.doubleVal)) ? 0 : 2);
    case DT_VARCHAR:
        return QString::fromUtf8(val.varcharVal);
    case DT_DATETIME:
        // Assuming datetime is stored as string or Unix timestamp
        return QDateTime::fromSecsSinceEpoch(val.intVal).toString(Qt::ISODate);
    default:
        return "NULL";
    }
    // Note:
    //     DATETIME assumes stored as qint64 Unix timestamp — that’s fine if consistent.
}

void TupleManager::handleRegularSelect(
    const SelectColumnsOperation* op,
    const std::vector<FieldBlock>& fields,
    const std::vector<DataRow>& rows,
    const std::vector<int>& columnIndices)
{
    if (rows.empty()) {
        widget->showMessage("No rows match the conditions.");
        return;
    }

    QStringList headerLabels;
    for (int idx : columnIndices) {
        headerLabels << QString::fromUtf8(fields[idx].name);
    }

    // Build message with borders
    QString message = "+-" + QString("-+-").repeated(headerLabels.size() - 1) + "-+\n";
    message += "| " + headerLabels.join(" | ") + " |\n";
    message += "+-" + QString("-+-").repeated(headerLabels.size() - 1) + "-+\n";

    for (const auto& row : rows) {
        message += "| ";
        for (int idx : columnIndices) {
            message += formatFieldValue(row.values[idx]) + " | ";
        }
        message += "\n";
    }

    // message += "+-" + QString("-+-").repeated(headerLabels.size() - 1) + "-+"; // 可能出现错误
    int dashes = qMax(1, headerLabels.size() - 1);
    message += "+-" + QString("-+-").repeated(dashes) + "-+";

    widget->showMessage(message);
}

bool TupleManager::matchesWhereConditions(const DataRow& row,
                                          const std::vector<FieldBlock>& fields,
                                          const std::vector<Condition>& conditions) const {
    for (const auto& cond : conditions) {
        int fieldIdx = -1;

        // Find the index of the field by name
        for (size_t i = 0; i < fields.size(); ++i) {
            if (strcmp(fields[i].name, cond.fieldName) == 0) {
                fieldIdx = i;
                break;
            }
        }

        // If field not found, skip condition
        if (fieldIdx == -1 || fieldIdx >= static_cast<int>(row.values.size())) {
            continue;
        }

        // Evaluate the condition against the row value
        if (!cond.evaluate(row.values[fieldIdx])) {
            return false; // Condition failed
        }
    }

    return true; // All conditions passed
}
