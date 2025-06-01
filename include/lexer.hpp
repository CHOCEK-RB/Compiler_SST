#pragma once

#include <token.hpp>

class Lexer {
  static constexpr int BUFFER_SIZE = 4096;

  char buffer[BUFFER_SIZE];
  int fd;
  int bufferPos = 0;
  int bufferLen = 0;
  int line = 1;

  char currentChar = 0;
  bool endOfFile = false;

  void advance();

public:
  explicit Lexer(const char* path);
  ~Lexer();

  Token nextToken();
};
