#include "Parser.h"

StmtChain* StmtChain::add(Stmt *stmt) {
  this->next = new StmtChain(stmt);
  this->next->header = this->header;
  return this->next;
}

Stmt *CompoundStmt::header() {
  return this->chain->header->stmt;
}

Stmt *CompoundStmt::next() {
  this->chain = this->chain->next;
  return chain->stmt;
}

bool CompoundStmt::isTail() {
  return this->chain->next == nullptr;
}

bool CompoundStmt::isHeader() {
  return this->chain->header == this->chain;
}