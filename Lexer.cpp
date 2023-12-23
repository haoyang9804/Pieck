#include "Lexer.h"
#include <cctype>
#include <cstdio>
#include <stdlib.h> 
#include <string>
#include "Error.h"


void Lexer::eat_chars(int number) {
  while (number --) {
    if (file_loc.col == code_line_length) {
      nextLine();
    }
    file_loc.col++;
  }
  if (file_loc.col == code_line_length) nextLine();
}

std::string_view Lexer::get_chars_in_this_line(int number) {
  ASSERT(number + file_loc.col <= code_line_length, "There are only " + std::to_string(code_line_length-file_loc.col) + " unhandled characters in this line (" + code_line + "), but the lexer requires " + std::to_string(number));
  return std::string_view(code_line.data() + file_loc.col, number);
}

void Lexer::nextLine() {
  std::getline(file, code_line);
  code_line_length = code_line.size();
  file_loc.line ++;
  file_loc.col = 0;
}

int Lexer::num_of_unhandled_chars_this_line() {
  return code_line_length - file_loc.col;
}

void Lexer::report() {
  std::string msg = "File: " + file_loc.file_name + ", Line: " + std::to_string(file_loc.line) + "\n" + code_line + "\n";
  msg += std::string(file_loc.col, ' ') + "^ Unexpected character";
}

bool Lexer::is_end_of_line(int num) {
  return file_loc.col + num == code_line_length;
}

bool Lexer::is_space(int number) {
  if (is_end_of_line(number)) return false;
  ASSERT(number + file_loc.col < code_line_length, 
  "The value of the argument number is at most " + std::to_string(code_line_length-file_loc.col - 1)
  + ". But you offer a " + std::to_string(number));
  return std::isspace(code_line[file_loc.col + number]);

}

void TokenHandler::pass() {
  this->next->handle(lexer->code_line);
}

void TokenHandler_def::handle(std::string &code_line) {
  TokenHandler::handle(code_line);
  if (lexer->num_of_unhandled_chars_this_line() < 3) {
    pass();
  }
  else if (lexer->get_chars_in_this_line(3) == "def" && (lexer->is_end_of_line(3) || lexer->is_space(3))) {
    this->lexer->kind = tk_keyword;
    this->lexer->token_text = "def";
    this->lexer->eat_chars(3);
  }
  else {
    pass();
  }
}

void TokenHandler_print::handle(std::string &code_line) {
  TokenHandler::handle(code_line);
  if (lexer->num_of_unhandled_chars_this_line() < 5) {
    pass();
  }
  else if (lexer->get_chars_in_this_line(5) == "print" && (lexer->is_end_of_line(5) || lexer->is_space(5))) {
    this->lexer->kind = tk_keyword;
    this->lexer->token_text = "print";
    this->lexer->eat_chars(5);
  }
  else {
    pass();
  }
}

void TokenHandler_for::handle(std::string &code_line) {
  TokenHandler::handle(code_line);
  if (lexer->num_of_unhandled_chars_this_line() < 3) {
    pass();
  }
  else if (lexer->get_chars_in_this_line(3) == "for" && (lexer->is_end_of_line(3) || lexer->is_space(3))) {
    this->lexer->kind = tk_keyword;
    this->lexer->token_text = "for";
    this->lexer->eat_chars(3);
  }
  else {
    pass();
  }
}

void TokenHandler_in::handle(std::string &code_line) {
  TokenHandler::handle(code_line);
  if (lexer->num_of_unhandled_chars_this_line() < 2) {
    pass();
  }
  else if (lexer->get_chars_in_this_line(2) == "in" && (lexer->is_end_of_line(2) || lexer->is_space(2))) {
    this->lexer->kind = tk_keyword;
    this->lexer->token_text = "in";
    this->lexer->eat_chars(2);
  }
  else {
    pass();
  }
}

void TokenHandler_unknown::handle(std::string &code_line) {
  this->lexer->report();
}

#define CHAIN(handler, ...) \


TokenHandler *Default_TokenHandler_Factory::create(Lexer *lexer) {
  return 
  new TokenHandler_def(
  lexer,
  new TokenHandler_print(
  lexer,
  new TokenHandler_for(
  lexer,
  new TokenHandler_in(
  lexer,
  new TokenHandler_unknown(
  lexer))))
  );
}

static Default_TokenHandler_Factory* _token_handler_factory = new Default_TokenHandler_Factory();

void Lexer::nextToken() {
  _token_handler_factory->create(this)->handle(this->code_line);
}