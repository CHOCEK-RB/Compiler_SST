#pragma once
#include <ast.hpp>
#include <lexer.hpp>
#include <memory>
#include <string>

enum ParameterMode { IMAGE, DIALOGUE };

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

  std::unique_ptr<MusicNode> parseMusic();
  std::unique_ptr<PlayNode> parsePlay();
  std::unique_ptr<StopNode> parseStop();
  std::unique_ptr<BackgroundNode> parseBackground();
  std::unique_ptr<CharacterNode> parseDefine();
  std::unique_ptr<SceneNode> parseScene();
  std::unique_ptr<ShowNode> parseShow();
  std::unique_ptr<HideNode> parseHide();
  std::unique_ptr<DialogueNode> parseDialogue();
  std::unique_ptr<ChoiceNode> parseChoice();
  std::unique_ptr<LabelNode> parseLabel();
  std::unique_ptr<CharacterModeData> parseMode();
  std::unique_ptr<JumpNode> parseJump();
  std::unique_ptr<EndNode> parseEnd();

  void
  parseParameters(ParameterMode mode,
                  std::unordered_map<std::string, ParameterValue> &parametes);
  void
  parseParameter(ParameterMode mode,
                 std::unordered_map<std::string, ParameterValue> &parameters);

  bool isMusic() { return current.type == TokenType::MUSIC; }
  bool isPlay() { return current.type == TokenType::PLAY; }
  bool isStop() { return current.type == TokenType::STOP; }
  bool isBackground() { return current.type == TokenType::BACKGROUND; }
  bool isDefine() { return current.type == TokenType::DEFINE; }
  bool isScene() { return current.type == TokenType::SCENE; }
  bool isShow() { return current.type == TokenType::SHOW; }
  bool isHide() { return current.type == TokenType::HIDE; }
  bool isChoice() { return current.type == TokenType::CHOICE; }
  bool isLabel() { return current.type == TokenType::LABEL; }
  bool isJump() { return current.type == TokenType::JUMP; }
  bool isEnd() { return current.type == TokenType::END; }
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
