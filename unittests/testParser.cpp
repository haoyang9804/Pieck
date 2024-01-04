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
#ifdef DEBUG
  std::cout << "tv->shape():\n";
  tv4->shape().print();
  std::cout << "shape1's shape:\n";
  shape1.print();
#endif
  EXPECT_TRUE(tv4->shape() == shape1);
}