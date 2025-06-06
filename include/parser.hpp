#pragma once
#include "lexer.hpp"
#include <string>

class Parser {
public:
  Parser(Lexer &lexer);

  void parseProgram();

private:
  Lexer &lexer;
  Token current;

  void advance();
  void expect(TokenType type, const std::string &msg);

  void parseStatement();

  void parseBackground();
  void parseDefine();
  void parseScene();
  void parseShow();
  void parseDialogue();
  void parseParameters();
  void parseParameter();
  void parseModes();
  void parseMode();

  bool isBackground() { return current.type == TokenType::BACKGROUND; }
  bool isDefine() { return current.type == TokenType::DEFINE; }
  bool isScene() { return current.type == TokenType::SCENE; }
  bool isShow() { return current.type == TokenType::SHOW; }
  bool isDialogue() {
    if (current.type == TokenType::STRING)
      return true;
    if (current.type == TokenType::IDENTIFIER) {
      Token next = lexer.peekToken(1);
      return next.type == TokenType::STRING;
    }
    return false;
  }
};
