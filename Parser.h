#pragma once

#include "Lexer.h"
#include "Error.h"
#include <string>

class Scope;
class Decl;
class Expr;

class Stmt {
protected:
  Stmt(Scope *scope) : scope(scope) {}
  Stmt() {};
  virtual ~Stmt() {}
public:
  Scope *scope = nullptr;
};

class StmtChain {
public:
  // Use Constructor only when creating the head of the chain
  StmtChain(Stmt *stmt) : stmt(stmt) {} 
  Stmt *stmt = nullptr;
  StmtChain *header = this;
  StmtChain *next = nullptr;
  StmtChain* add(Stmt *stmt);
};
class Expr : public Stmt {
private:
  enum Type {
    tyFloat64,
    tyNone,
    tyTuple,
    tyList
  };
  Type ty;
public:
  Expr (Type ty) : ty(ty) {}
};

// a sequence of stmts in the same scope
class CompoundStmt : public Stmt {
private:
  StmtChain* chain;
public:
  CompoundStmt(Scope* scope, StmtChain* chain) : Stmt(scope), chain(chain) {}
  Stmt *header();
  Stmt *next();
  bool isTail();
  bool isHeader();
};

class ReturnStmt : public Stmt {
public:
  Expr *expr;
  ReturnStmt(Scope *scope, Expr *expr) : Stmt(scope), expr(expr) {};
};

class LoopStmt : public Stmt {
public:
  StmtChain *body;
  LoopStmt(Scope *scope, StmtChain *body) : Stmt(scope), body(body) {};
};

class PrintStmt : public Stmt {
public:
  Expr *expr;
  PrintStmt(Scope *scope, Expr *expr) : Stmt(scope), expr(expr) {};
};

class DefStmt : public Stmt {
protected:
  DefStmt(Scope* scope, const std::string& name, Stmt* rhs) : Stmt(scope), identifier_name(name), rhs(rhs) {}
public:
  std::string identifier_name;
  Stmt *rhs;
};

class DefVarStmt : public DefStmt {
public:
  DefVarStmt(Scope* scope, const std::string& name, Expr* rhs) : DefStmt(scope, name, rhs) {}
  Expr *rhs_() { return dynamic_cast<Expr*>(this->rhs); }
};

class DefFuncStmt : public DefStmt {
public:
  DefFuncStmt(Scope* scope, const std::string& name, CompoundStmt* rhs) : DefStmt(scope, name, rhs) {}
  CompoundStmt *rhs_() { return dynamic_cast<CompoundStmt*>(this->rhs); }
};

//TODO: support parsing Let
// class LetStmt : public Stmt {
// };

class CallExpr : public Expr{

};

class VarExpr : public Expr {

};

class BinaryOpExpr : public Expr {

};

class ValueExpr : public Expr {

};

class ScalarExpr : public ValueExpr {

};

class TensorExpr : public ValueExpr {

};

class SliceExpr : public TensorExpr {

};

class RangeExpr : public ValueExpr {

};



class IVisitor {
public:
  virtual bool visit() = 0;
};