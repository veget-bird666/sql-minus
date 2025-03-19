#include "sqlparser.h"
#include "database_operations.h"
#include <QRegularExpression>


SqlParser::SqlParser() {}

// 解析SQL语句
Operation* SqlParser::parse(const QString& sql) {

    // 正则匹配CREATE DATABASE
    static QRegularExpression createDbRegex(
        "^CREATE\\s+DATABASE\\s+(\\w+)\\s*;$",
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption
        );

    // 正则匹配DROP DATABASE
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

    // 都未匹配，则抛出异常
    throw std::invalid_argument("Invalid SQL syntax");
}

