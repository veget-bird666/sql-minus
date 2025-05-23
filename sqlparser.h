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

    // 新增方法：处理可能包含换行和多条语句的SQL字符串
    static void executeMulti(const QString& sql, std::function<void(const QString&)> executeCallback);

    // 新增方法：从文件读取并执行SQL脚本
    static void executeFromFile(const QString& filePath, std::function<void(const QString&)> executeCallback);

private:
    // 解析单个条件表达式
    static Condition parseSingleCondition(const QString& conditionStr, const std::vector<FieldBlock>& fields);

    // 将字符串转换为FieldValue
    static FieldValue stringToFieldValue(const QString& str, DataType type);

    // 判断是否符合日期输入格式
    bool isDateValue(const QString &input);
    bool isDateValue2(const QString &input);
};

#endif // SQLPARSER_H
