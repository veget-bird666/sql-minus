#include "tuple_operations.h"
#include "tuple_manager.h"

void InsertOperation::execute() {
    TupleManager::insert(this);
}


void SelectAllOperation::execute() {
    TupleManager::selectAll(this);
}

void DeleteOperation::execute() {
    TupleManager::deleteRows(this);
}

void SelectColumnsOperation::execute() {
    TupleManager::selectColumns(this);
}

void UpdateOperation::execute() {
    TupleManager::update(this);
}
