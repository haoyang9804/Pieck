#include "Lexer.h"
#include <cctype>
#include <cstdio>
#include <stdlib.h> 
#include <string>
#include "Error.h"


void Lexer::eat_chars_in_the_current_line(int number) {
  while (number --) {
    file_loc.col++;
  }
  ASSERT(file_loc.col <= code_line_length, "file_loc.col (" + std::to_string(file_loc.col) + ") should not be larger than code_line_length (" + std::to_string(code_line_length) + ").");
}

std::string_view Lexer::get_chars_in_this_line(int number) {
  ASSERT(number > 0, "get_chars_in_this_line: The number of characeters you want to obtain should be larger than 0.");
  ASSERT(number + file_loc.col <= code_line_length, "There are only " + std::to_string(code_line_length-file_loc.col) + " unhandled characters in this line (" + code_line + "), but the lexer requires " + std::to_string(number));
  return std::string_view(code_line.data() + file_loc.col, number);
}

char Lexer::get_char_in_this_line(int dis) {
  ASSERT(dis >= 0, "dis should not be less than zero.");
  ASSERT(dis + file_loc.col < code_line_length, "get_char_in_this_line: There are only " + std::to_string(code_line_length-file_loc.col) + " unhandled characters in this line (" + code_line + "), but the lexer requires the " + std::to_string(dis+1) + (dis == 1 ? "st" : (dis == 2 ? "nd" : "th"))); 
  return code_line[file_loc.col + dis];
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
  ERROR(msg);
}

bool Lexer::is_end_of_line(int num) {
  return file_loc.col + num >= code_line_length;
}

bool Lexer::is_space(int number) {
  if (is_end_of_line(number)) return false;
  ASSERT(number + file_loc.col < code_line_length, 
  "The value of the argument number is at most " + std::to_string(code_line_length-file_loc.col - 1)
  + ". But you offer a " + std::to_string(number));
  return std::isspace(code_line[file_loc.col + number]);

}

void TokenHandler::pass() {
  return this->next->handle(lexer->code_line);
}

#define REGISTER_KEYWORD_TOKEN_HANDLER(LEN, NAME) \
  if (lexer->num_of_unhandled_chars_this_line() < LEN) { \
    pass();\
  }\
  else if (lexer->get_chars_in_this_line(LEN) == #NAME && (lexer->is_end_of_line(LEN) || lexer->is_space(LEN))) { \
    this->lexer->kind = tk_keyword; \
    this->lexer->token_text = #NAME; \
    this->lexer->eat_chars_in_the_current_line(LEN); \
  } \
  else pass();

void TokenHandler_def::handle(std::string &code_line) {
  TokenHandler::handle(code_line);
  REGISTER_KEYWORD_TOKEN_HANDLER(3, def);
}

void TokenHandler_print::handle(std::string &code_line) {
  TokenHandler::handle(code_line);
  REGISTER_KEYWORD_TOKEN_HANDLER(5, print); 
}

void TokenHandler_for::handle(std::string &code_line) {
  TokenHandler::handle(code_line);
  REGISTER_KEYWORD_TOKEN_HANDLER(3, for);
}

void TokenHandler_in::handle(std::string &code_line) {
  TokenHandler::handle(code_line);
  REGISTER_KEYWORD_TOKEN_HANDLER(2, in);
}

void TokenHandler_return::handle(std::string &code_line) {
  TokenHandler::handle(code_line);
  REGISTER_KEYWORD_TOKEN_HANDLER(6, return);
}

void TokenHandler_let::handle(std::string &code_line) {
  TokenHandler::handle(code_line);
  REGISTER_KEYWORD_TOKEN_HANDLER(3, let);
}


void TokenHandler_identifier::handle(std::string &code_line) {
  TokenHandler::handle(code_line);
  if (!std::isalpha(lexer->get_char_in_this_line(0))) {
    pass();
    return;
  }
  int32_t len = 1;
  while(! (lexer->is_end_of_line(len) || lexer->is_space(len)) && std::isalnum(lexer->get_char_in_this_line(len))) {
    len++;
  }
  this->lexer->kind = tk_identifier;
  this->lexer->token_text = code_line.substr(lexer->file_loc.col, len);
  this->lexer->eat_chars_in_the_current_line(len);
}

void TokenHandler_punctuation::handle(std::string &code_line) {
  TokenHandler::handle(code_line);
  if (!std::ispunct(lexer->get_char_in_this_line(0))) {
    pass();
    return;
  }
  int32_t len = 1;
  switch (lexer->get_char_in_this_line(0)) {
    // basic calculation-related symbols can be followed by a single '='
    case '+':
    case '-':
    case '*':
    case '/':
    case '@':
      if (!lexer->is_end_of_line(1) && lexer->get_char_in_this_line(1) == '=') {
        len++;
      }
      break;
    // .T means transpose
    case '.':
      if (!lexer->is_end_of_line(1) && lexer->get_char_in_this_line(1) == 'T') {
        len++;
      }
      break;
    // ; -> statement delimiter ;; -> function delimiter
    case ';':
      if (!lexer->is_end_of_line(1) && lexer->get_char_in_this_line(1) == ';') {
        len++;
      }
      break;
    case '&': // reference
    case '~': // range
    case ':': // field
    case '#': // comment
    case ',': // element delimiter
    case '[': case ']':
    case '{': case '}':
    case '(': case ')':
    case '=':
      break;
    default:
      this->lexer->report();
      break;
  }
  this->lexer->kind = tk_punctuation;
  this->lexer->token_text = code_line.substr(lexer->file_loc.col, len);
  this->lexer->eat_chars_in_the_current_line(len);
}

void TokenHandler_number::handle(std::string &code_line) {
  TokenHandler::handle(code_line);
  if (!(std::isdigit(lexer->get_char_in_this_line(0)) 
    || lexer->get_char_in_this_line(0) == '.' && !lexer->is_end_of_line(1) && std::isdigit(lexer->get_char_in_this_line(1)))) {
    pass();
    return;
  }
  int32_t len = 1;
  bool period_shown_up = (lexer->get_char_in_this_line(0) == '.');
  while(! (lexer->is_end_of_line(len) || lexer->is_space(len)) && 
  (std::isdigit(lexer->get_char_in_this_line(len) || (!period_shown_up && lexer->get_char_in_this_line(len) == '.')))) {
    if (!period_shown_up) period_shown_up = lexer->get_char_in_this_line(len) == '.';
    len++;
  }
  this->lexer->kind = tk_number;
  this->lexer->token_text = code_line.substr(lexer->file_loc.col, len);
  this->lexer->eat_chars_in_the_current_line(len);
}

void TokenHandler_string::handle(std::string &code_line) {
  TokenHandler::handle(code_line);
  if (lexer->get_char_in_this_line(0) != '\"' && lexer->get_char_in_this_line(0) != '\'') {
    pass();
    return;
  }
  int32_t len = 1;
  bool meet_another_delimiter = false;
  char delimeter = lexer->get_char_in_this_line(0);
  while (!(lexer->is_end_of_line(len))) {
    if (lexer->get_char_in_this_line(len) == delimeter) {
      len++;
      meet_another_delimiter = true;
      break;
    }
    len++;
  }
  if (meet_another_delimiter) {
    this->lexer->kind = tk_string;
    this->lexer->token_text = code_line.substr(lexer->file_loc.col, len);
    this->lexer->eat_chars_in_the_current_line(len);
  }
  else {
    this->lexer->report();
  }
}

void TokenHandler_unknown::handle(std::string &code_line) {
  TokenHandler::handle(code_line);
  this->lexer->report();
}

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
  new TokenHandler_return(
  lexer,
  new TokenHandler_let(
  lexer,
  // TokenHandler_identifier must be placed after all keyword handlers !!
  new TokenHandler_identifier(
  lexer,
  // TokenHandler_number must be placed before TokenHandler_punctuation since '.' is also considered in the number handler
  new TokenHandler_number(
  lexer,
  // TokenHandler_string must be placed before TokenHandler_punctuation since strings are wrapped by " or ', which are also punctuation characters
  new TokenHandler_string(
  lexer,
  new TokenHandler_punctuation(
  lexer,
  new TokenHandler_unknown(
  lexer)))))))))))
  
  
}

static Default_TokenHandler_Factory* _token_handler_factory = new Default_TokenHandler_Factory();

bool Lexer::nextToken() {
  _token_handler_factory->create(this)->handle(this->code_line);
  if (is_end_of_line(0) && file.eof()) {
    return false;
  }
  return true;
}