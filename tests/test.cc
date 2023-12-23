#include "../Lexer.h"

#include <iostream>
using namespace std;

int main() {
  Lexer lexer(FileLocation("code.pieck", 0, 0));
  for (int i = 0; i < 4; i++) {
    lexer.nextToken();
    cout << lexer.get_kind() << " " << lexer.get_token() << endl;  
    cout << "> " << lexer.file_loc.line << " " << lexer.file_loc.col << endl;
  }
  
}