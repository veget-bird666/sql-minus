#ifndef TUPLE_MANAGER_H
#define TUPLE_MANAGER_H

#include "manager.h"
#include "structures.h"
#include <vector>
#include <stdexcept>
#include "tuple_operations.h"


class TupleManager : public Manager {
public:
    static void insert(const InsertOperation* op);

};

#endif // TUPLE_MANAGER_H
