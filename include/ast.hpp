#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

// No forward declaration needed for Character anymore in ast.hpp itself for CharacterModeNode.
// The Character class is part of the generated code, not a type directly linked in ASTNode definitions.

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

class BackgroundNode : public ASTNode {
public:
  std::string name;
  std::string imagePath;
  Parameters parameters;

  void generateCode(std::ostream &out, int indent) const override;
};

struct CharacterModeData { // Renamed from CharacterModeNode to reflect it's now a data struct
  std::string name;
  std::string imagePath;
  Parameters parameters;
  // No need for parentCharacterId here, as CharacterNode will use this data directly.
};

class CharacterNode : public ASTNode {
public:
  std::string id;
  std::string displayName;
  // This will store the CharacterModeNodes, which now contain their parent ID.
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