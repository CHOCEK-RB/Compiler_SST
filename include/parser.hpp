#pragma once
#include "lexer.hpp"
#include <string>
#include <unordered_map>
#include <vector>

struct BackgroundInfo{
  std::string name;
  std::string imagePath;
  std::unordered_map<std::string, std::string> parameters;
};

struct Mode{
  std::string imagePath;
  std::unordered_map<std::string, std::string> parameters;
};

struct CharacterInfo{
  std::string Internalname;
  std::string Visiblename;
  std::unordered_map<std::string, Mode> modes;
};

struct SceneInfo{
  std::string BackgroundName;
  std::unordered_map<std::string, std::string> parameters;
};

struct showInfo{
  std::string character;
  std::string mode;
  std::unordered_map<std::string, std::string> paremeters;
};

class Parser {
public:
  Parser(Lexer &lexer);
  std::unordered_map<std::string, BackgroundInfo> backgrounds;
  std::unordered_map<std::string, CharacterInfo> characters;
  std::vector<SceneInfo> scenes;
  std::vector<showInfo> shows;
  void parseProgram();

private:
  Lexer &lexer;
  Token current;
  //Tabla de Simbolos



  //////////////////////////////////////////

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
