#pragma once

#include "Lexer.h"


/*

*/
class INode {
  
};

class IVisitor {
public:
  virtual bool visit() = 0;
};
