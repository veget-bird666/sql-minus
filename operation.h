#ifndef OPERATION_H
#define OPERATION_H
#include <QList>
#include "structures.h"

class Operation {
public:
    virtual ~Operation() = default;
    virtual void execute() = 0; // 纯虚函数，子类必须实现

    QString sql;          // 原始 SQL 语句
    QString statementType;// 语句类型
    LogRecord logRecord;  // 日志记录

};
#endif // OPERATION_H
