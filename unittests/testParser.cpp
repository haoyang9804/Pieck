#include "../include/Parser.h"
#include <gtest/gtest.h>

TEST(TestParser, ShapeChecking) {
  ScalaValue *sv1 = new ScalaValue("1.5");
  Value *vals[2];
  vals[0] = sv1;
  vals[1] = sv1;
  TensorValue *tv1 = new TensorValue(2, vals);
  Value *vals2[3];
  vals2[0] = sv1;
  vals2[1] = sv1;
  vals2[2] = sv1;
  TensorValue *tv2 = new TensorValue(3, vals2);
  Value *vals3[2];
  vals3[0] = tv1;
  vals3[1] = tv2;
  TensorValue *tv3 = new TensorValue(2, vals3);
  EXPECT_THROW(tv3->shape(), std::logic_error);
  Value *vals4[2];
  vals4[0] = tv1;
  vals4[1] = tv1;
  TensorValue *tv4 = new TensorValue(2, vals4); // 2x2
  int dims[2] = {2, 2};
  Shape shape1(2, dims);
  EXPECT_TRUE(tv4->shape() == shape1);
}

TEST(TestParser, build_DefVarStmt_1) {
  Parser parser("./codes/code_2.pieck");
  Stmt *stmt = parser.parse();
  CompoundStmt *c_stmt = dynamic_cast<CompoundStmt *>(stmt);
  EXPECT_TRUE(c_stmt->isTail());
  EXPECT_EQ(dynamic_cast<DefVarStmt *>(c_stmt->cur())->identifier_name, "x");
  ValueExpr *ve =
      dynamic_cast<ValueExpr *>(dynamic_cast<DefVarStmt *>(c_stmt->cur())->rhs);
  EXPECT_EQ(dynamic_cast<ScalaValue *>(ve->val)->val, 1.0);
  EXPECT_EQ(ve->type(), tyFloat64);
}