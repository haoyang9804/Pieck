#include "../Lexer.h"

#include <iostream>
using namespace std;

int main() {
  Lexer lexer(FileLocation("code.pieck", 0, 0));
  int cnt = 0;
  while (true) {
    bool res = lexer.nextToken();
    cnt ++;
    cout << lexer.get_kind() << " " << lexer.get_token() << endl;  
    // cout << "> " << lexer.file_loc.line << " " << lexer.file_loc.col << endl;
    if (!res) break;
  }
  cout << "cnt is " << cnt << endl;
}