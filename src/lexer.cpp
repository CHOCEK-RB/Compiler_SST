#include "token.hpp"
#include <cctype>
#include <fcntl.h>
#include <lexer.hpp>
#include <unistd.h>

Lexer::Lexer(const char *path) {
  fd = open(path, O_RDONLY);
  if (fd < 0) {
    perror("open");
    exit(1);
  }
  advance();
}

Lexer::~Lexer() {
  if (fd >= 0)
    close(fd);
}

void Lexer::advance() {
  if (bufferPos >= bufferLen) {
    bufferLen = read(fd, buffer, BUFFER_SIZE);
    bufferPos = 0;
    if (bufferLen <= 0) {
      endOfFile = true;
      currentChar = 0;
      return;
    }
  }
  currentChar = buffer[bufferPos++];
}
Token Lexer::peekToken(size_t n) {
  while (bufferToken.size() < n) {
    bufferToken.push_back(realNextToken());
  }
  return bufferToken[n - 1];
}

Token Lexer::nextToken() {
  if (!bufferToken.empty()) {
    Token tok = bufferToken.front();
    bufferToken.pop_front();
    return tok;
  }
  return realNextToken();
}


Token Lexer::realNextToken() {
  std::string lexeme;
  State state = State::START;

  while (!endOfFile) {
    switch (state) {
    case State::START:
      if (std::isspace(currentChar)) {
        advance();
      } else if (std::isalpha(currentChar) || currentChar == '_') {
        lexeme += currentChar;
        advance();
        state = State::IDENTIFIER;
      } else if (std::isdigit(currentChar)) {
        lexeme += currentChar;
        advance();
        state = State::NUMBER;
      } else if (currentChar == '"') {
        advance();
        state = State::STRING;
      } else {
        TokenType type;
        switch (currentChar) {
        case ':':
          type = TokenType::COLON;
          break;
        case ',':
          type = TokenType::COMMA;
          break;
        case '(':
          type = TokenType::LPAREN;
          break;
        case ')':
          type = TokenType::RPAREN;
          break;
        case '{':
          type = TokenType::LBRACKET;
          break;
        case '}':
          type = TokenType::RBRACKET;
          break;
        default:
          type = TokenType::UNKNOWN;
          break;
        }
        std::string val(1, currentChar);
        advance();
        return {type, val, line};
      }
      break;

    case State::IDENTIFIER:
      while (std::isalnum(currentChar) || currentChar == '_') {
        lexeme += currentChar;
        advance();
      }

      if (lexeme == "background")
        return {TokenType::BACKGROUND, lexeme, line};
      if (lexeme == "define")
        return {TokenType::DEFINE, lexeme, line};
      if (lexeme == "show")
        return {TokenType::SHOW, lexeme, line};
      if (lexeme == "scene")
        return {TokenType::SCENE, lexeme, line};
      return {TokenType::IDENTIFIER, lexeme, line};

    case State::NUMBER: {
      bool hasDot = false;
      while (std::isdigit(currentChar) || (!hasDot && currentChar == '.')) {
        if (currentChar == '.')
          hasDot = true;
        lexeme += currentChar;
        advance();
      }
      return {hasDot ? TokenType::FLOAT : TokenType::INT, lexeme, line};
    }

    case State::STRING:
      while (!endOfFile && currentChar != '"') {
        lexeme += currentChar;
        advance();
      }
      if (currentChar == '"') {
        advance();
        return {TokenType::STRING, lexeme, line};
      } else {
        return {TokenType::UNKNOWN, lexeme, line};
      }

    case State::ERROR:
      while (!std::isspace(currentChar) && !endOfFile) {
        lexeme += currentChar;
        advance();
      }
      return {TokenType::UNKNOWN, lexeme, line};
    }
  }

  return {TokenType::END_OF_FILE, "", line};
}
