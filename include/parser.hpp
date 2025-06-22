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
  std::vector<std::unordered_map<std::string, Mode>> modes;
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

struct DialogoLinea{
  std::string speaker;
  std::string text;
  std::unordered_map<std::string, std::string>parameters;
  int linea;
};

class Parser {
public:
  Parser(Lexer &lexer);
  std::unordered_map<std::string, BackgroundInfo> backgrounds;
  std::unordered_map<std::string, CharacterInfo> characters;
  std::vector<SceneInfo> scenes;
  std::vector<showInfo> shows;
  std::vector<DialogoLinea> dialogues;
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
  void parseParameters(std::unordered_map<std::string,std::string>&);
  void parseParameter(std::unordered_map<std::string,std::string>&);
  void parseModes(std::vector<std::unordered_map<std::string,Mode>>&);  
  void parseMode(std::vector<std::unordered_map<std::string,Mode>>&);

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
