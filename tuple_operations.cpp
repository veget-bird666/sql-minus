#include "tuple_operations.h"
#include "tuple_manager.h"

void InsertOperation::execute() {
    TupleManager::insert(this);
}


void SelectAllOperation::execute() {
    TupleManager::selectAll(this);
}
