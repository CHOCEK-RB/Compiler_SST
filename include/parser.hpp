#pragma once
#include <lexer.hpp>
#include <string>
#include <memory>
#include <ast.hpp>

enum ParameterMode {IMAGE, DIALOGUE};

class Parser {
public:
  Parser(Lexer &lexer);

  std::unique_ptr<ProgramNode> parseProgram();

private:
  Lexer &lexer;
  Token current;

  void advance();
  void expect(TokenType type, const std::string &msg);

  void parseStatement();

  std::unique_ptr<BackgroundNode> parseBackground();
  std::unique_ptr<CharacterNode> parseDefine();
  std::unique_ptr<SceneNode> parseScene();
  std::unique_ptr<ShowNode> parseShow();
  std::unique_ptr<HideNode> parseHide();
  std::unique_ptr<DialogueNode> parseDialogue();
  void parseParameters(ParameterMode mode, std::unordered_map<std::string,ParameterValue> &parametes);
  void parseParameter(ParameterMode mode, std::unordered_map<std::string,ParameterValue> &parameters);
 std::unique_ptr<CharacterModeData> parseMode(std::string parentCharacterId);

  bool isBackground() { return current.type == TokenType::BACKGROUND; }
  bool isDefine() { return current.type == TokenType::DEFINE; }
  bool isScene() { return current.type == TokenType::SCENE; }
  bool isShow() { return current.type == TokenType::SHOW; }
  bool isHide() { return current.type == TokenType::HIDE; }
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
