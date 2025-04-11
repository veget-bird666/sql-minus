#include "sqlparser.h"
#include "database_operations.h"
#include "table_operations.h"
#include <QRegularExpression>
#include "structures.h"


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

    // 如果都未匹配，抛出异常
    throw std::invalid_argument("输入指令格式错误");
}

// 解析字段定义部分
QList<FieldBlock> SqlParser::extractFields(const QString& tableDefinition) {
    QList<FieldBlock> fields;
    static QRegularExpression fieldRegex(
        "(\\w+)\\s+(\\w+)(\\((\\d+)\\))?\\s*(PRIMARY KEY|NOT NULL|UNIQUE|DEFAULT\\s+[^,]+|AUTO_INCREMENT|FOREIGN KEY\\s+REFERENCES\\s+(\\w+)\\s*\\((\\w+)\\)|CHECK\\s*\\(([^,]+)\\))?\\s*",
        QRegularExpression::CaseInsensitiveOption
        );
    QRegularExpressionMatchIterator matchIterator = fieldRegex.globalMatch(tableDefinition);
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        FieldBlock field;
        field.order = fields.size() + 1; // 字段顺序从 1 开始
        QString fieldName = match.captured(1).trimmed();
        QString fieldType = match.captured(2).trimmed();
        strncpy(field.name, fieldName.toStdString().c_str(), sizeof(field.name));

        // 类型映射
        if (fieldType == "INTEGER") {
            field.type = DT_INTEGER;
        } else if (fieldType == "BOOL") {
            field.type = DT_BOOL;
        } else if (fieldType == "DOUBLE") {
            field.type = DT_DOUBLE;
        } else if (fieldType == "VARCHAR") {
            field.type = DT_VARCHAR;
            field.param = match.captured(4).toInt(); // VARCHAR 长度
        } else if (fieldType == "DATETIME") {
            field.type = DT_DATETIME;
        }

        QString constraint = match.captured(5).trimmed();

        // 解析 PRIMARY KEY
        if (constraint.contains("PRIMARY KEY")) {
            field.integrities |= CT_PRIMARY_KEY;
        }

        // 解析 NOT NULL
        if (constraint.contains("NOT NULL")) {
            field.integrities |= CT_NOT_NULL;
        }

        // 解析 UNIQUE
        if (constraint.contains("UNIQUE")) {
            field.integrities |= CT_UNIQUE;
        }

        // 解析 DEFAULT
        if (constraint.startsWith("DEFAULT")) {
            field.integrities |= CT_DEFAULT;
        }

        // 解析 AUTO_INCREMENT
        if (constraint.contains("AUTO_INCREMENT")) {
            field.integrities |= CT_IDENTITY;
        }

        // 解析 FOREIGN KEY
        if (constraint.contains("FOREIGN KEY")) {
            field.integrities |= CT_FOREIGN_KEY;
            QString refTable = match.captured(7).trimmed();
            QString refField = match.captured(8).trimmed();
            strncpy(field.ref_table, refTable.toStdString().c_str(), sizeof(field.ref_table));
            strncpy(field.ref_field, refField.toStdString().c_str(), sizeof(field.ref_field));
        }

        // 解析 CHECK
        if (constraint.contains("CHECK")) {
            field.integrities |= CT_CHECK;
            QString checkCondition = match.captured(9).trimmed();
            strncpy(field.check_condition, checkCondition.toStdString().c_str(), sizeof(field.check_condition));
        }

        fields.append(field);
    }
    return fields;
}

