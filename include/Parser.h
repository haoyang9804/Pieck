#pragma once

#include "Error.h"
#include "Lexer.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#ifdef DEBUG
#include <iostream>
#endif

class Expr;
class Value;
class Stmt;
class Parser;

enum Type {
  tyUnknown = 0,
  tyFloat64 = -1,
  tyNone = -2,
  tyTuple = -3,
  tyList = -4
};

class Scope {
public:
  std::vector<std::string> scope_names;
  void push(std::string scope_name) { scope_names.push_back(scope_name); }
  void pop() { scope_names.pop_back(); }
  bool operator<=(const Scope &other) {
    if (scope_names.size() < other.scope_names.size())
      return false;
    for (int i = 0; i < other.scope_names.size(); i++) {
      if (scope_names[i] != other.scope_names[i])
        return false;
    }
    return true;
  }
};

class Stmt {
protected:
  Stmt(Scope scope) : scope(scope) {}
  Stmt(){};
  virtual ~Stmt() {}

public:
  Scope scope;
};

class StmtChain {
public:
  StmtChain() {}
  StmtChain(Stmt *stmt) : stmt(stmt) {}
  Stmt *stmt = nullptr;
  StmtChain *header = this;
  StmtChain *next = nullptr;
  StmtChain *add(Stmt *stmt);
};

// a sequence of stmts in the same scope
class CompoundStmt : public Stmt {
private:
  StmtChain *chain;

public:
  CompoundStmt(Scope scope, StmtChain *chain) : Stmt(scope), chain(chain) {}
  Stmt *header();
  Stmt *next();
  Stmt *cur();
  bool isTail();
  bool isHeader();
};

class ReturnStmt : public Stmt {
public:
  Expr *expr;
  ReturnStmt(Scope scope, Expr *expr) : Stmt(scope), expr(expr){};
};

class LoopStmt : public Stmt {
public:
  StmtChain *body;
  LoopStmt(Scope scope, StmtChain *body) : Stmt(scope), body(body){};
};

class PrintStmt : public Stmt {
public:
  Expr *expr;
  PrintStmt(Scope scope, Expr *expr) : Stmt(scope), expr(expr){};
};

class DefStmt : public Stmt {
protected:
  DefStmt(Scope scope, const std::string &name)
      : Stmt(scope), identifier_name(name) {}

public:
  std::string identifier_name;
};

class DefVarStmt : public DefStmt {
public:
  DefVarStmt(Scope scope, const std::string &name, Expr *rhs)
      : DefStmt(scope, name), rhs(rhs) {}
  Expr *rhs;
};

class DefFuncStmt : public DefStmt {
public:
  DefFuncStmt(Scope scope, const std::string &name, CompoundStmt *rhs)
      : DefStmt(scope, name), rhs(rhs) {}
  CompoundStmt *rhs;
};

// TODO: support parsing Let
//  class LetStmt : public Stmt {
//  };
class Expr {
protected:
  Type ty = tyUnknown;

public:
  Expr(Type ty) : ty(ty) {}
  Expr() {}
  virtual ~Expr() {}
  Type type() { return this->ty; }
  void set_type(Type ty) { this->ty = ty; }
};

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
#ifdef DEBUG
  void print() {
    ASSERT(!unintialized(), "Shape must be initialized before being printed.");
    std::cout << "{";
    for (int i = 0; i < dims_dim; i++) {
      std::cout << dims[i];
      if (i < dims_dim - 1)
        std::cout << ", ";
    }
    std::cout << "}";
  }
#endif
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

class Parser {
private:
  Lexer lexer;
  Scope scope;
  DefStmt *build_DefStmt();
  DefVarStmt *build_DefVarStmt(std::string);
  DefFuncStmt *build_DefFuncStmt(std::string);
  Stmt *meet_keyword();

public:
  Parser(std::string file_name) : lexer(FileLocation(file_name, 0, 0)) {}

  Stmt *parse();
};