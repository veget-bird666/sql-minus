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


};

#endif // SQLPARSER_H
