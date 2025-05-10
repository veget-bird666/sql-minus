#include "sqlparser.h"
#include "database_operations.h"
#include "table_operations.h"
#include "tuple_operations.h"
#include <QRegularExpression>
#include "structures.h"
#include "file_utils.h"
#include <QDateTime>
#include <cstring>

extern QString currentDB;

SqlParser::SqlParser() {}

// 解析SQL语句
Operation* SqlParser::parse(const QString& sql) {
    // 正则匹配 CREATE DATABASE
    static QRegularExpression createDbRegex(
        "^CREATE\\s+DATABASE\\s+(\\w+)\\s*;$",
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption
        );

    // 正则匹配 DROP DATABASE
    static QRegularExpression dropDbRegex(
        "^DROP\\s+DATABASE\\s+(\\w+)\\s*;$",
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption
        );

    // 匹配 CREATE DATABASE
    QRegularExpressionMatch createMatch = createDbRegex.match(sql);
    if (createMatch.hasMatch()) {
        QString dbName = createMatch.captured(1).trimmed();
        return new CreateDatabaseOperation(dbName);
    }

    // 匹配 DROP DATABASE
    QRegularExpressionMatch dropMatch = dropDbRegex.match(sql);
    if (dropMatch.hasMatch()) {
        QString dbName = dropMatch.captured(1).trimmed();
        return new DropDatabaseOperation(dbName);
    }

    // 匹配 SHOW DATABASES
    static QRegularExpression showDatabasesRegex(
        "^\\s*SHOW\\s+DATABASES\\s*;?\\s*$",
        QRegularExpression::CaseInsensitiveOption
        );

    QRegularExpressionMatch showMatch = showDatabasesRegex.match(sql);
    if (showMatch.hasMatch()) {
        return new ShowDatabasesOperation(); // 返回对应的操作类
    }
    // 匹配 USE DATABASE
    static QRegularExpression useDbRegex(
        "^USE\\s+DATABASE\\s+(\\w+)\\s*;$",
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption
        );

    QRegularExpressionMatch useMatch = useDbRegex.match(sql);
    if (useMatch.hasMatch()) {
        QString dbName = useMatch.captured(1).trimmed();
        return new UseDatabaseOperation(dbName); // 返回对应的操作对象
    }

    // 匹配DROP TABLE
    static QRegularExpression dropTableRegex(
        "^DROP\\s+TABLE\\s+(\\w+)\\s*;$",
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption
        );

    QRegularExpressionMatch dropTableMatch = dropTableRegex.match(sql);
    if (dropTableMatch.hasMatch()) {
        QString tableName = dropTableMatch.captured(1).trimmed();
        QString dbName = currentDB;
        return new DropTableOperation(dbName, tableName);
    }

    // 匹配 SHOW TABLES
    static QRegularExpression showTablesRegex(
        "^\\s*SHOW\\s+TABLES\\s*;?\\s*$",
        QRegularExpression::CaseInsensitiveOption
        );

    QRegularExpressionMatch showTablesMatch = showTablesRegex.match(sql);
    if (showTablesMatch.hasMatch()) {
        return new ShowTablesOperation();
    }

    // 匹配 CREATE TABLE
    static QRegularExpression createTableRegex(
        "^CREATE\\s+TABLE\\s+(\\w+)\\s*\\((.+)\\)\\s*;$",
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption
        );

    QRegularExpressionMatch createTableMatch = createTableRegex.match(sql);
    if (createTableMatch.hasMatch()) {
        QString tableName = createTableMatch.captured(1).trimmed();
        QString tableDefinition = createTableMatch.captured(2).trimmed();

        // 提取字段信息
        QList<FieldBlock> fields = extractFields(tableDefinition);

        // 返回一个包含表名和字段信息的操作对象
        return new CreateTableOperation(tableName, fields);
    }


    // 匹配 ALTER TABLE ADD COLUMN
    static QRegularExpression addColumnRegex(
        "^ALTER\\s+TABLE\\s+(\\w+)\\s+ADD\\s+COLUMN\\s+(.+);$",
        QRegularExpression::CaseInsensitiveOption
        );

    QRegularExpressionMatch addMatch = addColumnRegex.match(sql);
    if (addMatch.hasMatch()) {
        QString tableName = addMatch.captured(1).trimmed();
        QList<FieldBlock> fields = extractFields(addMatch.captured(2));

        // if (fields.size() != 1) throw std::invalid_argument("只能添加一个字段"); // 不用throw
        return new AddColumnOperation(currentDB, tableName, fields.first());
    }

    // 匹配 ALTER TABLE DROP COLUMN
    static QRegularExpression dropColumnRegex(
        "^ALTER\\s+TABLE\\s+(\\w+)\\s+DROP\\s+COLUMN\\s+(\\w+);$",
        QRegularExpression::CaseInsensitiveOption
        );

    QRegularExpressionMatch dropColMatch = dropColumnRegex.match(sql);
    if (dropColMatch.hasMatch()) {
        QString tableName = dropColMatch.captured(1).trimmed();
        QString columnName = dropColMatch.captured(2).trimmed();
        return new DropColumnOperation(currentDB, tableName, columnName);
    }

    // 匹配 ALTER TABLE MODIFY COLUMN
    static QRegularExpression modifyColumnRegex(
        "^ALTER\\s+TABLE\\s+(\\w+)\\s+MODIFY\\s+COLUMN\\s+(.+);$",
        QRegularExpression::CaseInsensitiveOption
        );

    QRegularExpressionMatch modifyMatch = modifyColumnRegex.match(sql);
    if (modifyMatch.hasMatch()) {
        QString tableName = modifyMatch.captured(1).trimmed();
        QList<FieldBlock> fields = extractFields(modifyMatch.captured(2));
        // if (fields.size() != 1) throw std::invalid_argument("只能修改一个字段"); // 不用throw
        return new ModifyColumnOperation(currentDB, tableName,
                                         QString::fromUtf8(fields.first().name), fields.first());
    }

    // 匹配 DESC 或 DESCRIBE 表名
    static QRegularExpression descTableRegex(
        "^\\s*(DESC|DESCRIBE)\\s+(\\w+)\\s*;?\\s*$",
        QRegularExpression::CaseInsensitiveOption
        );
    QRegularExpressionMatch descMatch = descTableRegex.match(sql);
    if (descMatch.hasMatch()) {
        QString tableName = descMatch.captured(2).trimmed();
        return new DescribeTableOperation(currentDB, tableName);
    }

    // 匹配 INSERT 语句（格式：INSERT INTO table VALUES (v1, v2, ...);）
    static QRegularExpression insertRegex(
        "^INSERT INTO (\\w+) VALUES \\((.+)\\);$",
        QRegularExpression::CaseInsensitiveOption
        );

    QRegularExpressionMatch match = insertRegex.match(sql);
    if (match.hasMatch()) {
        QString tableName = match.captured(1).trimmed();
        QStringList values = match.captured(2).split(',', Qt::SkipEmptyParts);

        // 构造InsertOperation
        InsertOperation* op = new InsertOperation();
        op->dbName = currentDB;
        op->tableName = tableName;

        // 转换值类型（需根据表字段定义动态判断，此处简化）
        for (const QString& valStr : values) {
            FieldValue val;
            if (valStr.contains("'")) { // 字符串
                val.type = DT_VARCHAR;
                strncpy(val.varcharVal, valStr.trimmed().mid(1, valStr.length() - 2).toUtf8(), 256);
            } else if (valStr.contains(".")) { // 浮点数
                val.type = DT_DOUBLE;
                val.doubleVal = valStr.toDouble();
            } else { // 整数
                val.type = DT_INTEGER;
                val.intVal = valStr.toInt();
            }
             op->values.push_back(val);
        }
        return op;
    }

    // 匹配 DELETE FROM 表名 WHERE 条件
    static QRegularExpression deleteRegex(
        "^DELETE\\s+FROM\\s+(\\w+)(?:\\s+WHERE\\s+(.+))?\\s*;$",
        QRegularExpression::CaseInsensitiveOption
        );

    QRegularExpressionMatch deleteMatch = deleteRegex.match(sql);
    if (deleteMatch.hasMatch()) {
        QString tableName = deleteMatch.captured(1).trimmed();
        QString whereClause = deleteMatch.captured(2).trimmed();

        // 读取表字段定义
        std::vector<FieldBlock> fields = FileUtil::readTableFields(currentDB, tableName);

        // 解析WHERE条件
        std::vector<Condition> conditions;
        if (!whereClause.isEmpty()) {
            conditions = parseWhereClause(whereClause, fields);
        }

        return new DeleteOperation(currentDB, tableName, conditions);
    }


    // 匹配 SELECT * 语句
    static QRegularExpression selectAllRegex(
        "^SELECT\\s+\\*\\s+FROM\\s+(\\w+)\\s*;$",
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption
        );

    QRegularExpressionMatch selectMatch = selectAllRegex.match(sql);
    if (selectMatch.hasMatch()) {
        QString tableName = selectMatch.captured(1).trimmed();
        QString dbName = currentDB;
        return new SelectAllOperation(dbName, tableName);
    }

    // 匹配 SELECT [属性] WHERE 语句
    // 在 SqlParser::parse() 中添加新的匹配模式
static QRegularExpression selectColumnsRegex(
    "^SELECT\\s+(.+?)\\s+FROM\\s+(\\w+)(?:\\s+WHERE\\s+(.+))?\\s*;$",
        QRegularExpression::CaseInsensitiveOption |
            QRegularExpression::MultilineOption
);


// 解析示例：SELECT id, name FROM users WHERE age > 18;
QRegularExpressionMatch whereSelectMatch = selectColumnsRegex.match(sql);
if (whereSelectMatch.hasMatch() || whereSelectMatch.isValid()) { // valid new added
    QStringList columns = whereSelectMatch.captured(1).split(',', Qt::SkipEmptyParts);
    for (auto& col : columns) {
        col = col.trimmed();
    } // new added
    QString tableName = whereSelectMatch.captured(2).trimmed();
    QString whereClause = whereSelectMatch.captured(3).trimmed();
    
    // 创建操作对象
    QString dbName = currentDB;
    auto* op = new SelectColumnsOperation(dbName, tableName, columns);
    if (!whereClause.isEmpty()) {
        auto fields = FileUtil::readTableFields(dbName, tableName);
        op->conditions = parseWhereClause(whereClause, fields);
    }
    return op;
}


    // 如果都未匹配，抛出异常
    throw std::invalid_argument("输入指令格式错误");
}

QList<FieldBlock> SqlParser::extractFields(const QString& tableDefinition) {
    QList<FieldBlock> fields;

    //第一阶段：解析普通字段定义
    static QRegularExpression fieldRegex(
        "(\\w+)\\s+(\\w+)(\\((\\d+)\\))?\\s*(PRIMARY KEY|NOT NULL|UNIQUE|DEFAULT\\s+[^,]+|AUTO_INCREMENT|"
        "FOREIGN KEY\\s+REFERENCES\\s+(\\w+)\\s*\\((\\w+)\\)|CHECK\\s*\\(([^,]+)\\))?\\s*,?",
        QRegularExpression::CaseInsensitiveOption
        );
    // 改进后的正则表达式（防止空匹配）
    // static QRegularExpression fieldRegex(
    //     "(\\w+)\\s+(\\w+)(\\((\\d+)\\))?\\s*"
    //     "(?:PRIMARY KEY|NOT NULL|UNIQUE|DEFAULT\\s+[^,]*|AUTO_INCREMENT|"
    //     "FOREIGN KEY\\s+REFERENCES\\s+(\\w+)\\s*\\((\\w+)\\)|CHECK\\s*\\([^)]*\\))?\\s*"
    //     "(?=,|$)",
    //     QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption);

    // 匹配聚合函数
    static QRegularExpression funcRegex(
        "(\\w+)\\(\\s*([\\w\\*]+)\\s*\\)",
        QRegularExpression::CaseInsensitiveOption);


    // 先处理普通字段定义
    QRegularExpressionMatchIterator fieldIter = fieldRegex.globalMatch(tableDefinition);
    while (fieldIter.hasNext()) {
        QRegularExpressionMatch match = fieldIter.next();
        FieldBlock field;
        memset(&field, 0, sizeof(FieldBlock));
        field.isAggregateFunc = false;
        field.order = fields.size() + 1; // 字段顺序从 1 开始
        QString fieldName = match.captured(1).trimmed();
        QString fieldType = match.captured(2).trimmed();
        strncpy(field.name, fieldName.toStdString().c_str(), sizeof(field.name));

        // 类型映射
        if (fieldType == "INTEGER" || fieldType == "int") {
            field.type = DT_INTEGER;
            field.param = 0;
        } else if (fieldType == "BOOL" || fieldType == "bool") {
            field.type = DT_BOOL;
            field.param = 0;
        } else if (fieldType == "DOUBLE" || fieldType == "double") {
            field.type = DT_DOUBLE;
            field.param = 0;
        } else if (fieldType == "VARCHAR" || fieldType == "varchar") {
            field.type = DT_VARCHAR;
            field.param = match.captured(4).toInt(); // VARCHAR 长度
        } else if (fieldType == "DATETIME" || fieldType == "datatime") {
            field.type = DT_DATETIME;
            field.param = 0;
        }

        QString constraint = match.captured(5).trimmed();

        // 解析 PRIMARY KEY
        if (constraint.contains("PRIMARY KEY") || constraint.contains("primary key")) {
            field.integrities |=(1 << CT_PRIMARY_KEY );
        }

        // 解析 NOT NULL
        if (constraint.contains("NOT NULL") || constraint.contains("not null")) {
            field.integrities |=(1 << CT_NOT_NULL);
        }

        // 解析 UNIQUE
        if (constraint.contains("UNIQUE") || constraint.contains("unique")) {
            field.integrities |=(1 << CT_UNIQUE);
        }

        // 解析 DEFAULT
        if (constraint.startsWith("DEFAULT") || constraint.startsWith("default")) {
            field.integrities |=(1 << CT_DEFAULT);
        }

        // 解析 AUTO_INCREMENT
        if (constraint.contains("AUTO_INCREMENT") || constraint.contains("auto_increment")) {
            field.integrities |=(1 << CT_IDENTITY);
        }

        // 解析 FOREIGN KEY
        if (constraint.contains("FOREIGN KEY") || constraint.contains("foreign key")) {
            field.integrities |=(1 << CT_FOREIGN_KEY);
            QString refTable = match.captured(7).trimmed();
            QString refField = match.captured(8).trimmed();
            strncpy(field.ref_table, refTable.toStdString().c_str(), sizeof(field.ref_table));
            strncpy(field.ref_field, refField.toStdString().c_str(), sizeof(field.ref_field));
        }

        // 解析 CHECK
        if (constraint.contains("CHECK") || constraint.contains("check")) {
            field.integrities |=(1 << CT_CHECK);
            QString checkCondition = match.captured(9).trimmed();
            strncpy(field.check_condition, checkCondition.toStdString().c_str(), sizeof(field.check_condition));
        }

        fields.append(field);
    }
    // 单独处理聚合函数（新增逻辑）
    // QRegularExpressionMatchIterator funcIter = funcRegex.globalMatch(tableDefinition);
    // while (funcIter.hasNext()) {
    //     QRegularExpressionMatch match = funcIter.next();
    //     FieldBlock funcField;
    //     memset(&funcField, 0, sizeof(FieldBlock));

    //     // 设置函数信息
    //     QString funcName = match.captured(1).toUpper();
    //     QString fieldName = match.captured(2).trimmed();

    //     // 填充FunctionCall结构
    //     strncpy(funcField.func.funcName, funcName.toUtf8().constData(), 32);
    //     strncpy(funcField.func.fieldName, fieldName.toUtf8().constData(), 128);

    //     // 设置函数类型（与原有枚举值匹配）
    //     if (funcName == "COUNT") funcField.func.funcType = FT_COUNT;
    //     else if (funcName == "MAX") funcField.func.funcType = FT_MAX;
    //     else if (funcName == "MIN") funcField.func.funcType = FT_MIN;
    //     else if (funcName == "AVG") funcField.func.funcType = FT_AVG;
    //     else if (funcName == "SUM") funcField.func.funcType = FT_SUM;
    //     else continue; // 匹配不上就不匹配了

    //     // 设置字段元数据
    //     funcField.order = fields.size() + 1;
    //     snprintf(funcField.name, 128, "%s(%s)", funcName.toUtf8().constData(), fieldName.toUtf8().constData());
    //     funcField.type = (funcField.func.funcType == FT_COUNT) ? DT_INTEGER : DT_DOUBLE;
    //     funcField.isAggregateFunc = true;

    //     fields.append(funcField);
    // }
    QRegularExpressionMatchIterator funcIter = funcRegex.globalMatch(tableDefinition);
    while (funcIter.hasNext()) {
        QRegularExpressionMatch match = funcIter.next();
        FieldBlock funcField;
        memset(&funcField, 0, sizeof(FieldBlock));
        QString funcName = match.captured(1).toUpper();
        QString fieldName = match.captured(2).trimmed();

        // Set FunctionCall info
        strncpy(funcField.func.funcName, funcName.toUtf8().constData(), 31);
        strncpy(funcField.func.fieldName, fieldName.toUtf8().constData(), 127);

        if (funcName == "COUNT") funcField.func.funcType = FT_COUNT;
        else if (funcName == "MAX") funcField.func.funcType = FT_MAX;
        else if (funcName == "MIN") funcField.func.funcType = FT_MIN;
        else if (funcName == "AVG") funcField.func.funcType = FT_AVG;
        else if (funcName == "SUM") funcField.func.funcType = FT_SUM;

        // Set metadata
        funcField.order = fields.size() + 1;
        snprintf(funcField.name, 127, "%s(%s)", funcName.toUtf8().constData(), fieldName.toUtf8().constData());
        funcField.type = (funcField.func.funcType == FT_COUNT) ? DT_INTEGER : DT_DOUBLE;
        funcField.isAggregateFunc = true;

        fields.append(funcField);
    }
        // QRegularExpressionMatchIterator it = funcRegex.globalMatch(tableDefinition);
        // while (it.hasNext()) {
        //     QRegularExpressionMatch match = it.next();
        //     FieldBlock funcField;
        //     memset(&funcField, 0, sizeof(FieldBlock));

        //     // 填充函数信息
        //     QString funcName = match.captured(1).toUpper();
        //     QString fieldName = match.captured(2).trimmed();

        //     // 设置FunctionCall
        //     strncpy(funcField.func.funcName, funcName.toUtf8().constData(), 32);
        //     strncpy(funcField.func.fieldName, fieldName.toUtf8().constData(), 128);

        //     // 设置函数类型
        //     if (funcName == "COUNT") funcField.func.funcType = FT_COUNT;
        //     else if (funcName == "MAX") funcField.func.funcType = FT_MAX;
        //     // ...其他函数类型判断

        //     // 标记为函数字段
        //     funcField.isAggregateFunc = true;

        //     // 设置显示名称和返回类型
        //     snprintf(funcField.name, 128, "%s(%s)", funcField.func.funcName, funcField.func.fieldName);
        //     funcField.type = (funcField.func.funcType == FT_COUNT) ? DT_INTEGER : DT_DOUBLE;

        //     fields.append(funcField);
        // }

    return fields;
}



std::vector<Condition> SqlParser::parseWhereClause(const QString& whereClause, const std::vector<FieldBlock>& fields) {
    std::vector<Condition> conditions;

    // 简单实现：用AND分割多个条件
    QStringList conditionStrs = whereClause.split("AND", Qt::SkipEmptyParts, Qt::CaseInsensitive);

    for (QString condStr : conditionStrs) {
        conditions.push_back(parseSingleCondition(condStr.trimmed(), fields));
    }

    return conditions;
}

// 解析where后的条件
Condition SqlParser::parseSingleCondition(const QString& conditionStr, const std::vector<FieldBlock>& fields) {
    Condition cond;
    memset(&cond, 0, sizeof(Condition));

    // 解析操作符
    static QMap<QString, int> opMap = {
        {"=", 0}, {"<", 1}, {">", 2}, {"<=", 3}, {">=", 4}, {"!=", 5}
    };

    // 查找操作符
    int opPos = -1;
    QString opStr;
    for (auto it = opMap.begin(); it != opMap.end(); ++it) {
        int pos = conditionStr.indexOf(it.key());
        if (pos > 0) {
            opPos = pos;
            opStr = it.key();
            break;
        }
    }

    if (opPos == -1) {
        throw std::invalid_argument("Invalid operator in condition");
    }

    // 提取字段名和值
    QString fieldName = conditionStr.left(opPos).trimmed();
    QString valueStr = conditionStr.mid(opPos + opStr.length()).trimmed();

    // 查找字段定义
    auto fieldIt = std::find_if(fields.begin(), fields.end(),
                                [&fieldName](const FieldBlock& f) {
                                    return QString::fromUtf8(f.name) == fieldName;
                                });

    if (fieldIt == fields.end()) {
        throw std::invalid_argument("Field not found: " + fieldName.toStdString());
    }

    // 填充Condition结构体
    strncpy(cond.fieldName, fieldName.toUtf8().constData(), sizeof(cond.fieldName));
    cond.operatorType = opMap[opStr];
    cond.compareValue = stringToFieldValue(valueStr, static_cast<DataType>(fieldIt->type));

    return cond;
}


// 将字符串转换为FieldValue
FieldValue SqlParser::stringToFieldValue(const QString& str, DataType type) {
    FieldValue val;
    memset(&val, 0, sizeof(FieldValue));
    val.type = type;

    switch (type) {
    case DT_INTEGER:
        val.intVal = str.toInt();
        break;
    case DT_BOOL:
        val.boolVal = str.toLower() == "true" || str == "1";
        break;
    case DT_DOUBLE:
        val.doubleVal = str.toDouble();
        break;
    case DT_VARCHAR: {
        QString cleanStr = str;
        if (cleanStr.startsWith('\'') && cleanStr.endsWith('\'')) {
            cleanStr = cleanStr.mid(1, cleanStr.length() - 2);
        }
        strncpy(val.varcharVal, cleanStr.toUtf8().constData(), sizeof(val.varcharVal));
        break;
    }
    case DT_DATETIME:
        // 简化处理，实际可能需要解析日期时间字符串
        val.intVal = QDateTime::fromString(str, Qt::ISODate).toSecsSinceEpoch();
        break;
    default:
        throw std::invalid_argument("Unsupported data type");
    }

    return val;
}
