#ifndef OPERATION_H
#define OPERATION_H

class Operation {
public:
    virtual ~Operation() = default;
    virtual void execute() = 0; // 纯虚函数，子类必须实现
};
#endif // OPERATION_H
