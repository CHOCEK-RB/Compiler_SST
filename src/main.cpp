#include <lexer.hpp>
#include <token.hpp>

std::string tokenToString(TokenType type) {
  auto it = tokenStr.find(type);
  if (it != tokenStr.end())
    return it->second;
  return "UNDEFINED";
}

int main() {
  Lexer lexer("main.sst");

  while (true) {
    Token token = lexer.nextToken();
    if (token.type == TokenType::END_OF_FILE)
      break;

    printf("Token: %-15s Value: %-20s Line: %d\n",
           tokenToString(token.type).c_str(),
           token.value.c_str(),
           token.line);
  }

  return 0;
}
