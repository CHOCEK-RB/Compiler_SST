#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

using ParameterValue = std::variant<int, double, std::string>;
using Parameters = std::unordered_map<std::string, ParameterValue>;

class ASTNode {
public:
  virtual ~ASTNode() = default;
  virtual void generateCode(std::ostream &out, int indent = 0) const = 0;
};

class ProgramNode : public ASTNode {
public:
  std::vector<std::unique_ptr<ASTNode>> statements;

  void generateCode(std::ostream &out, int indent) const override;
};

class MusicNode : public ASTNode {
public:
  std::string id;
  std::string filePath;
  void generateCode(std::ostream &out, int indent) const override;
};

class PlayNode : public ASTNode {
public:
  std::string musicId;
  void generateCode(std::ostream &out, int indent) const override;
};

class StopNode : public ASTNode {
public:
  std::string musicId;
  void generateCode(std::ostream &out, int indent) const override;
};

class BackgroundNode : public ASTNode {
public:
  std::string name;
  std::string imagePath;
  Parameters parameters;

  void generateCode(std::ostream &out, int indent) const override;
};

struct CharacterModeData {
  std::string name;
  std::string imagePath;
  Parameters parameters;
};

class CharacterNode : public ASTNode {
public:
  std::string id;
  std::string displayName;
  std::vector<std::unique_ptr<CharacterModeData>> modes;

  void generateCode(std::ostream &out, int indent) const override;
};

class ShowNode : public ASTNode {
public:
  std::string characterId;
  std::string mode;
  Parameters parameters;

  void generateCode(std::ostream &out, int indent) const override;
};

class HideNode : public ASTNode {
public:
  std::string characterId;
  Parameters parameters;

  void generateCode(std::ostream &out, int indent) const override;
};

class DialogueNode : public ASTNode {
public:
  std::string speaker;
  std::string text;
  Parameters parameters;

  void generateCode(std::ostream &out, int indent) const override;
};

class SceneNode : public ASTNode {
public:
  std::string name;
  Parameters parameters;

  void generateCode(std::ostream &out, int indent) const override;
};

class OptionNode : public ASTNode {
public:
  std::string text;
  std::string gotoLabel;

  void generateCode(std::ostream &out, int indent) const override;
};

class ChoiceNode : public ASTNode {
public:
  std::string prompt;
  std::vector<std::unique_ptr<OptionNode>> options;

  void generateCode(std::ostream &out, int indent) const override;
};

class LabelNode : public ASTNode {
public:
  std::string name;
  std::vector<std::unique_ptr<ASTNode>> statements;

  void generateCode(std::ostream &out, int indent) const override;
};

class JumpNode : public ASTNode {
public:
  std::string target;
  void generateCode(std::ostream &out, int indent) const override;
};

class EndNode : public ASTNode {
public:
  void generateCode(std::ostream &out, int indent) const override;
};
