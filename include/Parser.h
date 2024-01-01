#pragma once

#include "Error.h"
#include "Lexer.h"
#include <cstddef>
#include <string>
#include <unordered_map>

class Scope;
class Expr;
class Value;
class Stmt;
class Sema;

enum Type {
  tyUnknown = 0,
  tyFloat64 = -1,
  tyNone = -2,
  tyTuple = -3,
  tyList = -4
};

class Stmt {
protected:
  Stmt(Scope *scope) : scope(scope) {}
  Stmt(){};
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
  StmtChain *add(Stmt *stmt);
};

class Expr : public Stmt {
protected:
  Type ty = tyUnknown;

public:
  Expr(Type ty) : ty(ty) {}
  Expr() {}
  Type type() { return this->ty; }
  void set_type(Type ty) { this->ty = ty; }
};

// a sequence of stmts in the same scope
class CompoundStmt : public Stmt {
private:
  StmtChain *chain;

public:
  CompoundStmt(Scope *scope, StmtChain *chain) : Stmt(scope), chain(chain) {}
  Stmt *header();
  Stmt *next();
  bool isTail();
  bool isHeader();
};

class ReturnStmt : public Stmt {
public:
  Expr *expr;
  ReturnStmt(Scope *scope, Expr *expr) : Stmt(scope), expr(expr){};
};

class LoopStmt : public Stmt {
public:
  StmtChain *body;
  LoopStmt(Scope *scope, StmtChain *body) : Stmt(scope), body(body){};
};

class PrintStmt : public Stmt {
public:
  Expr *expr;
  PrintStmt(Scope *scope, Expr *expr) : Stmt(scope), expr(expr){};
};

class DefStmt : public Stmt {
protected:
  DefStmt(Scope *scope, const std::string &name, Stmt *rhs)
      : Stmt(scope), identifier_name(name), RHS(rhs) {}

public:
  std::string identifier_name;
  Stmt *RHS;
};

class DefVarStmt : public DefStmt {
public:
  DefVarStmt(Scope *scope, const std::string &name, Expr *rhs)
      : DefStmt(scope, name, rhs) {}
  Expr *rhs() { return dynamic_cast<Expr *>(this->RHS); }
};

class DefFuncStmt : public DefStmt {
public:
  DefFuncStmt(Scope *scope, const std::string &name, CompoundStmt *rhs)
      : DefStmt(scope, name, rhs) {}
  CompoundStmt *rhs() { return dynamic_cast<CompoundStmt *>(this->RHS); }
};

// TODO: support parsing Let
//  class LetStmt : public Stmt {
//  };

class CallExpr : public Expr {
public:
  CallExpr(std::string func_name) : func_name(func_name) {}
  std::string func_name;
};

class VarExpr : public Expr {
public:
  VarExpr(std::string var_name) : var_name(var_name) {}
  std::string var_name;
};

class BinaryOpExpr : public Expr {
protected:
  enum OP { add, sub, mul, div, matmul };

public:
  BinaryOpExpr(Expr *lhs, OP op, Expr *rhs) : lhs(lhs), op(op), rhs(rhs) {}
  OP op;
  Expr *lhs, *rhs;
};

// TODO: support UnaryOP
//  class UnaryOpExpr : public Expr {
//  };

// Describe the shape of a tensor (a1,a2,...,an) or a scale ()
struct Shape {
  Shape() {}
  // the length of the dims vector
  int32_t dims_dim = -1;
  // an int array representing the dims
  // such as [1, 3, 2, 4]
  int32_t *dims = nullptr;
  Shape(int32_t dims_dim) : dims_dim(dims_dim) {}
  Shape(int32_t dims_dim, int32_t dims[]) : dims_dim(dims_dim), dims(dims) {}
  // The comparison here is very strict.
  // Shape A == Shape B holds only when A's dims is exactly the same as B's
  // Does not support broadcast rules to enable "interspecies communication"
  // such as Scala + Vector
  bool operator==(const Shape &other);
  bool operator!=(const Shape &other) { return !operator==(other); }
  bool unintialized() const { return dims_dim == -1; }
};

class Value {
public:
  Value() {}
  ~Value() {}
  virtual bool is_scala() { return false; }
  virtual bool is_tensor() { return false; }
  virtual Shape shape() = 0;

protected:
  Shape _shape;
};

class ScalaValue : public Value {
public:
  ScalaValue(std::string val_str) {
    try {
      this->val = std::stod(val_str);
    } catch (...) {
      ERROR(val_str + " cannot be converted into a double value.");
    }
    _shape.dims_dim = 0;
  }
  double val;
  bool is_scala() override { return true; }
  Shape shape() override;
};

class TensorValue : public Value {
public:
  TensorValue(int32_t dim, Value *vals[]) : dim(dim), vals(vals) {}
  // dim must be > 0
  int32_t dim;
  Value **vals;
  bool is_tensor() override { return true; }
  // We delay the shape inference of tensors to when they are used.
  // Therefore, the unused incorrect shapes will not trigger an error
  // and should be removed in codegen.
  Shape shape() override;
  void set_shape(Shape _shape) { this->_shape = _shape; }
};

class ValueExpr : public Expr {
protected:
public:
  Value *val;
  ValueExpr(Value *val) : val(val) {}
};

class Sema {};