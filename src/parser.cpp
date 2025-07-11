#include <ast.hpp>
#include <parser.hpp>
#include <stdexcept>
#include <token.hpp>

void checkParameterImage(const std::string &name, const std::string &value,
                         ParameterValue &buffValue) {
  if (name == "x" || name == "y" || name == "scale") {
    try {
      buffValue = std::stod(value);
    } catch (const std::invalid_argument &e) {
      throw std::runtime_error(
          "[Error: Se ingreso un valor incorrecto para el parametro: " + name +
          "]");
    } catch (const std::out_of_range &e) {
      throw std::runtime_error(
          "[Error: El valor es muy grande para el parametro : " + name + "]");
    }
    return;
  }
  throw std::runtime_error("[Error: No existe el parametro : " + name + "]");
}

void checkParameterDialogue(const std::string &name, const std::string &value,
                            ParameterValue &buffValue) {
  if (name == "size" || name == "speed") {
    try {
      buffValue = std::stod(value);
    } catch (const std::invalid_argument &e) {
      throw std::runtime_error(
          "[Error: Se ingreso un valor incorrecto para el parametro: " + name +
          "]");
    } catch (const std::out_of_range &e) {
      throw std::runtime_error(
          "[Error: El valor es muy grande para el parametro : " + name + "]");
    }
    return;
  }
  throw std::runtime_error("[Error: No existe el parametro : " + name + "]");
  return;
}

std::string tokenToString(TokenType type) {
  auto it = tokenStr.find(type);
  if (it != tokenStr.end())
    return it->second;
  return "UNDEFINED";
}

Parser::Parser(Lexer &lexer)
    : lexer(lexer), current(Token{TokenType::UNKNOWN, "", 0}) {
  advance();
}

void Parser::advance() {
  current = lexer.nextToken();
  while (current.type == TokenType::COMMENT) {
    current = lexer.nextToken();
  }
}

void Parser::expect(TokenType type, const std::string &msg) {
  if (current.type != type) {
    throw std::runtime_error("[Línea " + std::to_string(current.line) +
                             "] Error: " + msg);
  }
  advance();
}

std::unique_ptr<ProgramNode> Parser::parseProgram() {
  auto program = std::make_unique<ProgramNode>();

  while (current.type != TokenType::END_OF_FILE) {
    if (isBackground()) {
      program->statements.push_back(parseBackground());
    } else if (isDefine()) {
      program->statements.push_back(parseDefine());
    } else if (isScene()) {
      program->statements.push_back(parseScene());
    } else if (isShow()) {
      program->statements.push_back(parseShow());
    } else if (isHide()) {
      program->statements.push_back(parseHide());
    } else if (isDialogue()) {
      program->statements.push_back(parseDialogue());
    } else if (isMusic()) {
      program->statements.push_back(parseMusic());
    } else if (isPlay()) {
      program->statements.push_back(parsePlay());
    } else if (isStop()) {
      program->statements.push_back(parseStop());
    } else if (isChoice()) {
      program->statements.push_back(parseChoice());
    } else if (isLabel()) {
      program->statements.push_back(parseLabel());
    } else if (isJump()) {
      program->statements.push_back(parseJump());
    } else if (isEnd()) {
      program->statements.push_back(parseEnd());
    } else {
      throw std::runtime_error("[Línea " + std::to_string(current.line) +
                               "] Sentencia inválida");
    }
  }
  return program;
}

std::unique_ptr<BackgroundNode> Parser::parseBackground() {
  auto node = std::make_unique<BackgroundNode>();

  advance();
  node->name = current.value;
  expect(TokenType::IDENTIFIER, "Se esperaba nombre de fondo");

  expect(TokenType::LPAREN, "Se esperaba '('");
  node->imagePath = current.value;
  expect(TokenType::STRING, "Se esperaba ruta de imagen");

  if (current.type == TokenType::COMMA) {
    advance();
    parseParameters(IMAGE, node->parameters);
  }

  expect(TokenType::RPAREN, "Se esperaba ')'");
  return node;
}

std::unique_ptr<CharacterNode> Parser::parseDefine() {
  auto node = std::make_unique<CharacterNode>();

  advance();
  node->id = current.value;
  expect(TokenType::IDENTIFIER, "Se esperaba ID de personaje");

  node->displayName = current.value;
  expect(TokenType::STRING, "Se esperaba nombre visible");

  expect(TokenType::LBRACKET, "Se esperaba '{'");
  while (current.type != TokenType::RBRACKET &&
         current.type != TokenType::END_OF_FILE) {
    node->modes.push_back(parseMode());

    if (current.type == TokenType::COMMA) {
      advance();
    }
  }
  expect(TokenType::RBRACKET, "Se esperaba '}'");

  return node;
}

std::unique_ptr<SceneNode> Parser::parseScene() {
  auto node = std::make_unique<SceneNode>();

  advance();
  node->name = current.value;
  expect(TokenType::IDENTIFIER, "Se esperaba nombre de escena o fondo");

  std::unordered_map<std::string, ParameterValue> parametros;
  if (current.type == TokenType::LPAREN) {
    advance();
    parseParameters(IMAGE, parametros);
    expect(TokenType::RPAREN, "Se esperaba ')'");
  }

  node->parameters = parametros;
  return node;
}

std::unique_ptr<ShowNode> Parser::parseShow() {
  auto node = std::make_unique<ShowNode>();

  advance();
  node->characterId = current.value;
  expect(TokenType::IDENTIFIER, "Se esperaba nombre del personaje");

  node->mode = current.value;
  expect(TokenType::IDENTIFIER, "Se esperaba el modo del personaje");

  if (current.type == TokenType::LPAREN) {
    advance();
    parseParameters(IMAGE, node->parameters);
    expect(TokenType::RPAREN, "Se esperaba ')'");
  }

  return node;
}

std::unique_ptr<HideNode> Parser::parseHide() {
  auto node = std::make_unique<HideNode>();
  advance();
  node->characterId = current.value;
  expect(TokenType::IDENTIFIER, "Se esperaba nombre del personaje o imagen");

  if (current.type == TokenType::LPAREN) {
    advance();
    parseParameters(IMAGE, node->parameters);
    expect(TokenType::RPAREN, "Se esperaba ')'");
  }
  return node;
}

std::unique_ptr<DialogueNode> Parser::parseDialogue() {
  auto node = std::make_unique<DialogueNode>();

  std::unordered_map<std::string, ParameterValue> parametro;
  if (current.type == TokenType::STRING) {
    node->speaker = "You";
    node->text = current.value;
    advance();

    if (current.type == TokenType::LPAREN) {
      advance();
      parseParameters(DIALOGUE, parametro);
      expect(TokenType::RPAREN, "Se esperaba ')'");
    }

    node->parameters = parametro;
  } else if (current.type == TokenType::IDENTIFIER) {
    node->speaker = current.value;
    advance();

    node->text = current.value;
    expect(TokenType::STRING, "Se esperaba diálogo entre comillas");

    if (current.type == TokenType::LPAREN) {
      advance();
      parseParameters(DIALOGUE, parametro);
      expect(TokenType::RPAREN, "Se esperaba ')'");
    }

    node->parameters = parametro;
  } else {
    throw std::runtime_error("Diálogo inválido");
  }
  return node;
}

void Parser::parseParameters(
    ParameterMode mode,
    std::unordered_map<std::string, ParameterValue> &parameters) {
  parseParameter(mode, parameters);
  while (current.type == TokenType::COMMA) {
    advance();
    parseParameter(mode, parameters);
  }
}

void Parser::parseParameter(
    ParameterMode mode,
    std::unordered_map<std::string, ParameterValue> &parameters) {
  std::string name = current.value;
  expect(TokenType::IDENTIFIER, "Se esperaba nombre de parámetro");
  expect(TokenType::COLON, "Se esperaba ':'");
  if (current.type == TokenType::INT || current.type == TokenType::FLOAT ||
      current.type == TokenType::IDENTIFIER ||
      current.type == TokenType::STRING) {
    ParameterValue realValue;

    if (mode == IMAGE)
      checkParameterImage(name, current.value, realValue);
    else if (mode == DIALOGUE)
      checkParameterDialogue(name, current.value, realValue);

    parameters[name] = realValue;
    advance();
  } else {
    throw std::runtime_error("Valor de parámetro inválido");
  }
}

std::unique_ptr<CharacterModeData> Parser::parseMode() {
  auto node = std::make_unique<CharacterModeData>();
  node->name = current.value;
  expect(TokenType::IDENTIFIER, "Se esperaba nombre de modo");

  expect(TokenType::COLON, "Se esperaba ':'");
  expect(TokenType::LPAREN, "Se esperaba '('");

  node->imagePath = current.value;
  expect(TokenType::STRING, "Se esperaba ruta de imagen");

  if (current.type == TokenType::COMMA) {
    advance();
    parseParameters(IMAGE, node->parameters);
  }

  expect(TokenType::RPAREN, "Se esperaba ')'");

  return node;
}

std::unique_ptr<MusicNode> Parser::parseMusic() {
  auto node = std::make_unique<MusicNode>();

  advance();
  node->id = current.value;
  expect(TokenType::IDENTIFIER, "Se esperaba un ID para la música");

  node->filePath = current.value;
  expect(TokenType::STRING, "Se esperaba la ruta del archivo de música");
  return node;
}

std::unique_ptr<PlayNode> Parser::parsePlay() {
  auto node = std::make_unique<PlayNode>();

  advance();
  node->musicId = current.value;
  expect(TokenType::IDENTIFIER, "Se esperaba el ID de la música a reproducir");
  return node;
}

std::unique_ptr<StopNode> Parser::parseStop() {
  auto node = std::make_unique<StopNode>();

  advance();
  node->musicId = current.value;
  expect(TokenType::IDENTIFIER, "Se esperaba el ID de la música a detener");
  return node;
}

std::unique_ptr<ChoiceNode> Parser::parseChoice() {
  auto node = std::make_unique<ChoiceNode>();

  advance();
  node->prompt = current.value;
  expect(TokenType::STRING,
         "Se esperaba un string para el prompt de la elección");

  while (current.type == TokenType::OPTION) {
    auto optionNode = std::make_unique<OptionNode>();
    advance();
    optionNode->text = current.value;
    expect(TokenType::STRING,
           "Se esperaba un string para el texto de la opción");
    expect(TokenType::ARROW, "Se esperaba '->' después del texto de la opción");
    optionNode->gotoLabel = current.value;
    expect(TokenType::IDENTIFIER,
           "Se esperaba un identificador para la etiqueta de salto");
    node->options.push_back(std::move(optionNode));
  }

  return node;
}

std::unique_ptr<LabelNode> Parser::parseLabel() {
  auto node = std::make_unique<LabelNode>();

  advance();
  node->name = current.value;
  expect(TokenType::IDENTIFIER,
         "Se esperaba un identificador para la etiqueta");
  expect(TokenType::COLON, "Se esperaba ':' después del nombre de la etiqueta");

  while (current.type != TokenType::LABEL &&
         current.type != TokenType::END_OF_FILE) {
    if (isBackground()) {
      node->statements.push_back(parseBackground());
    } else if (isDefine()) {
      node->statements.push_back(parseDefine());
    } else if (isScene()) {
      node->statements.push_back(parseScene());
    } else if (isShow()) {
      node->statements.push_back(parseShow());
    } else if (isHide()) {
      node->statements.push_back(parseHide());
    } else if (isDialogue()) {
      node->statements.push_back(parseDialogue());
    } else if (isMusic()) {
      node->statements.push_back(parseMusic());
    } else if (isPlay()) {
      node->statements.push_back(parsePlay());
    } else if (isStop()) {
      node->statements.push_back(parseStop());
    } else if (isChoice()) {
      node->statements.push_back(parseChoice());
    } else if (isJump()) {
      node->statements.push_back(parseJump());
    } else if (isEnd()) {
      node->statements.push_back(parseEnd());
    } else {
      throw std::runtime_error("[Línea " + std::to_string(current.line) +
                               "] Sentencia inválida dentro de la etiqueta");
    }
  }

  return node;
}

std::unique_ptr<JumpNode> Parser::parseJump() {
  auto node = std::make_unique<JumpNode>();

  advance();
  node->target = current.value;
  expect(TokenType::IDENTIFIER, "Se esperaba el ID de la etiqueta a ir");
  return node;
}

std::unique_ptr<EndNode> Parser::parseEnd() {
  advance();
  auto node = std::make_unique<EndNode>();
  return node;
}
