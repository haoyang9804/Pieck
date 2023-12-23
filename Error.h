#pragma once

#include <stdexcept>

#define ASSERT(STATEMENT, STR)                                                 \
  if (!(STATEMENT))                                                            \
  throw std::logic_error(STR)

#define ERROR(STR) throw std::logic_error(STR)