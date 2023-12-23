#pragma once

#include <cctype>
#include <cstddef>
#include <string>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string_view>

/*

This Lexer is for a Python-like programming language for matrix calculation:

def f(x, y):
  return x@y;
;;

def main():
  x = [[1,2,3], [2,3,4]];
  y = [[1,2], [2,3], [3,4]]

  print f(x, y) # output is [[14, 20], [20, 29]]
  
  sum = 0
  for i in x[0][:2]:
    sum += i;
  for &i in x[1]:
    i ++; 
  
  print x # output is [[1,2,3], [3,4,5]]

  print x, y, sum
;;

*/

enum TokenKind : int {
  tk_punctuation,
  tk_identifier,
  tk_keyword,
  tk_literal,
  tk_eof,
  // tk_comment,
};

struct FileLocation {
  std::string file_name;
  int32_t line, col;
  FileLocation(std::string file_name, int32_t line, int32_t col): file_name(file_name), line(line), col(col) {}
};

struct TokenHandler;

class Lexer {
public:
  Lexer(FileLocation file_loc) : file_loc(file_loc) {
    file.open(file_loc.file_name);
    nextLine();
  }
  FileLocation file_loc;
  ~Lexer() { file.close(); }
  
  void nextToken(); // get the next token

  // for debuggin purposes
  TokenKind get_kind() { return kind; }
  std::string get_token() { return token_text; }

  friend class TokenHandler;
  friend class TokenHandler_def;
  friend class TokenHandler_print;
  friend class TokenHandler_for;
  friend class TokenHandler_in;
  friend class TokenHandler_unknown;

private:
  // eat a number of characters
  void eat_chars(int);
  // get a number of unhandeled continuous characters
  std::string_view get_chars_in_this_line(int);
  // a queue to get 
  // read the next line from the source file
  void nextLine(); 
  // the number of remaining characters
  int num_of_unhandled_chars_this_line();
  // report the error
  void report();
  // Look n characters backward and check if the nth character is '\n'
  bool is_end_of_line(int num);
  // Look n characters backward and check if the nth character is ' '
  bool is_space(int num);

private:
  TokenKind kind;  
  // the real text for this token
  std::string token_text;
  // FileLocation file_loc;
  // a buffer the temporarily store a line from the source file
  std::string code_line;
  int32_t code_line_length;
  std::ifstream file;
};


struct TokenHandler {
  TokenHandler(Lexer * lexer) {
    this->lexer = lexer;
  }
  TokenHandler(Lexer * lexer, TokenHandler *next) {
    this->lexer = lexer;
    this->next = next;
  }
  virtual void handle(std::string &code_line) {
    if (std::isspace(code_line[lexer->file_loc.col])) {
      lexer->eat_chars(1);
    }
  }
  // pass the responsibility to the next handler
  void pass();
  TokenHandler *next = nullptr;
  Lexer *lexer;
};

struct TokenHandler_def : TokenHandler {
  TokenHandler_def(Lexer *lexer, TokenHandler *next) : TokenHandler(lexer, next){}
  void handle(std::string &code_line) final;
};

struct TokenHandler_print : TokenHandler {
  TokenHandler_print(Lexer *lexer, TokenHandler *next) : TokenHandler(lexer, next){}
  void handle(std::string &code_line);
};

struct TokenHandler_for : TokenHandler {
  TokenHandler_for(Lexer *lexer, TokenHandler *next) : TokenHandler(lexer, next){}
  void handle(std::string &code_line);
};


struct TokenHandler_in : TokenHandler {
  TokenHandler_in(Lexer *lexer, TokenHandler *next) : TokenHandler(lexer, next){}
  void handle(std::string &code_line);
};

struct TokenHandler_unknown : TokenHandler {
  TokenHandler_unknown(Lexer *lexer) : TokenHandler(lexer){}
  void handle(std::string &code_line);
};


struct ITokenHandler_Factory {
  virtual TokenHandler *create(Lexer* lexer) = 0;
};

struct Default_TokenHandler_Factory : ITokenHandler_Factory {
  TokenHandler *create(Lexer *lexer) override;
};
