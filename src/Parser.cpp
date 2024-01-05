#include "../include/Parser.h"
#include "../include/Error.h"
#include <cctype>
#include <string.h>

StmtChain *StmtChain::add(Stmt *stmt) {
  this->next = new StmtChain(stmt);
  this->next->header = this->header;
  return this->next;
}

Stmt *CompoundStmt::header() { return this->chain->header->stmt; }

Stmt *CompoundStmt::next() {
  this->chain = this->chain->next;
  return chain->stmt;
}

Stmt *CompoundStmt::cur() { return chain->stmt; }

bool CompoundStmt::isTail() { return this->chain->next == nullptr; }

bool CompoundStmt::isHeader() { return this->chain->header == this->chain; }

bool Shape::operator==(const Shape &other) {
  if (unintialized() || other.unintialized())
    return false;
  if (dims_dim != other.dims_dim)
    return false;
  if (dims_dim == 0 && other.dims_dim == 0) {
    return true;
  }
  for (int i = 0; i < dims_dim; i++) {
    if (dims[i] != other.dims[i])
      return false;
  }
  return true;
}

Shape ScalaValue::shape() {
  ASSERT(!_shape.unintialized(), "");
  return _shape = Shape(0); // the default _shape, with dims_dim = 0
}

// Shape checking, and calculate the shapes by the way
// The shape requriement is similar to Numpy's.
//
// \background: If a tensor is of shape (2, 3, 4), then we say the tensor
// has two sub-tensors, each of which of shape (3, 4), this is a well-formed
// tensor.
//
// But here is an ill-formed tensors that have two sub-tensors of different
// shapes: [[1,2], [1]]. If we use Numpy to wrap such array, then an error
// "The requested array has an inhomogeneous shape" will be thrown.
// Pieck follows Numpy's taste on tensors and fails the shape checking if
// sub-tensors are of different shapes.
bool shape_checking(TensorValue *tv) {
  ASSERT(tv->dim > 0, "Tensor's dim must be larger than 0.");
  auto f_next_dim = [](TensorValue *tv, int i) {
    ASSERT(i < tv->dim, "");
    if (tv->vals[i]->is_scala())
      return 0;
    return dynamic_cast<TensorValue *>(tv->vals[i])->dim;
  };
  // if sub-tensors are not of the same shape,
  // then shape checking fails
  int nextdim = f_next_dim(tv, 0);
  for (int i = 1; i < tv->dim; i++) {
    if (nextdim != f_next_dim(tv, i))
      return false;
  }
  // if all sub-tensors are actually scalars, then no need for
  // further shape checkings on sub-tensors
  if (nextdim == 0) {
    int32_t *dims = new int[1];
    dims[0] = tv->dim;
    tv->set_shape(Shape(1, dims));
    return true;
  }
  // if all sub-tensors are tensors, then check each sub-tensor separately
  // to make sure all of them are of good shape
  bool subchecking = true;
  for (int i = 0; i < tv->dim; i++) {
    subchecking &= shape_checking(dynamic_cast<TensorValue *>(tv->vals[i]));
  }
  if (!subchecking)
    return false;
  // Until now, all sub-shape-checking on sub-tensors succeeds, but remains
  // a single problem: what if sub-shapes are not the same?
  Shape shape_0 = tv->vals[0]->shape();
  for (int i = 1; i < tv->dim; i++) {
    if (shape_0 != tv->vals[i]->shape())
      return false;
  }
  int32_t *sub_dims = shape_0.dims;
  int32_t *dims = (int32_t *)malloc(sizeof(int) * (shape_0.dims_dim + 1));
  memcpy(dims + 1, sub_dims, sizeof(int32_t) * (shape_0.dims_dim));
  dims[0] = tv->dim;
  tv->set_shape(Shape(shape_0.dims_dim + 1, dims));
  return true;
}

Shape TensorValue::shape() {
  if (_shape.unintialized()) {
    shape_checking(this);
  }
  ASSERT(!_shape.unintialized(), "Shape must be initialized here.");
  return _shape;
}

DefFuncStmt *Parser::build_DefFuncStmt(std::string identifier_name) {
  // TODO: impl
  return nullptr;
}

DefVarStmt *Parser::build_DefVarStmt(std::string identifier_name) {
  bool res = lexer.nextToken();
  // TODO: if tk_string is supported, this checking should be expanded
  // TODO: support 'a = func(x, y)'
  if (!res ||
      lexer.get_kind() != tk_number && lexer.get_kind() != tk_identifier) {
    lexer.report("Expected a number or an identifier");
  }
  if (std::isdigit(lexer.get_token()[0])) {
    std::string digit_token_1 = lexer.get_token();
    Value *ve_1 = new ScalaValue(digit_token_1);
    res = lexer.nextToken();
    if (!res) {
      lexer.report("Should not end up here.");
    }

    if (lexer.get_token() == ";") {
      // x = 1;
      Expr *expr = new ValueExpr(ve_1);
      expr->set_type(tyFloat64);
      return new DefVarStmt(scope, std::move(identifier_name), expr);
    }
    // TODO other situations
    return nullptr;
  }
  return nullptr;
}

DefStmt *Parser::build_DefStmt() {
  bool res = lexer.nextToken();
  if (!res || lexer.get_kind() != tk_identifier) {
    lexer.report("Expected an indentifier");
  }
  std::string identifier_name = lexer.get_token();
  res = lexer.nextToken();
  if (!res || lexer.get_kind() != tk_punctuation) {
    lexer.report("Expected a punctuation");
  }
  if (lexer.get_token() == "(") {
    return build_DefFuncStmt(identifier_name);
  } else if (lexer.get_token() == "=") {
    return build_DefVarStmt(identifier_name);
  } else {
    lexer.report("Expected ( or =, but receives a " + lexer.get_token());
  }
}

Stmt *Parser::meet_keyword() {
  if (lexer.get_token() == "def") {
    return build_DefStmt();
  }
  // TODO: add other keywords here
}

Stmt *Parser::parse() {
  bool head_init = false;
  StmtChain *chain = new StmtChain();
  while (lexer.nextToken()) {
    TokenKind tk = lexer.get_kind();
    std::string token = lexer.get_token();
    if (tk == tk_keyword) {
      if (!head_init) {
        head_init = true;
        chain->stmt = meet_keyword();
      } else
        chain->add(meet_keyword());
    }
  }
  return new CompoundStmt(scope, chain);
}