#pragma once

#include <deque>
#include <token.hpp>

class Lexer {
  static constexpr int BUFFER_SIZE = 4096;
  std::deque<Token> bufferToken;

  char buffer[BUFFER_SIZE];
  int fd;
  int bufferPos = 0;
  int bufferLen = 0;
  int line = 1;

  char currentChar = 0;
  bool endOfFile = false;

  void advance();
  Token realNextToken();

public:
  explicit Lexer(const char* path);
  ~Lexer();

  Token nextToken();
  Token peekToken(size_t n = 1);
};
