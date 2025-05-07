#ifndef SQLPARSER_H
#define SQLPARSER_H
#include <QString>
#include "database_operations.h"
#include "structures.h"

// 手写实现SQL语句解析器
class SqlParser
{
public:
    SqlParser();

    // 解析SQL语句
    static Operation* parse(const QString& sql);
    static QList<FieldBlock>  extractFields(const QString& tableDefinition);

    // 解析WHERE条件
    static std::vector<Condition> parseWhereClause(const QString& whereClause, const std::vector<FieldBlock>& fields);

private:
    // 解析单个条件表达式
    static Condition parseSingleCondition(const QString& conditionStr, const std::vector<FieldBlock>& fields);

    // 将字符串转换为FieldValue
    static FieldValue stringToFieldValue(const QString& str, DataType type);

};

#endif // SQLPARSER_H
