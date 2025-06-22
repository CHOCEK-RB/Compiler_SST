#include "token.hpp"
#include <exception>
#include <iostream>
#include <parser.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>

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
  std::cout << tokenToString(current.type) << "\n";
}

void Parser::expect(TokenType type, const std::string &msg) {
  if (current.type != type) {
    throw std::runtime_error("[Línea " + std::to_string(current.line) +
                             "] Error: " + msg);
  }
  advance();
}

void Parser::parseProgram() {
  while (current.type != TokenType::END_OF_FILE) {
    parseStatement();
  }
}

void Parser::parseStatement() {
  if (isBackground())
    parseBackground();
  else if (isDefine())
    parseDefine();
  else if (isScene())
    parseScene();
  else if (isShow())
    parseShow();
  else if (isDialogue())
    parseDialogue();
  else
    throw std::runtime_error("[Línea " + std::to_string(current.line) +
                             "] Sentencia inválida");
}

void Parser::parseBackground() {
  advance();
  std::string nombre_fondo = current.value;
  expect(TokenType::IDENTIFIER, "Se esperaba el nombre del fondo");
  expect(TokenType::LPAREN, "Se esperaba '('");
  std::string imagen_path = current.value;
  expect(TokenType::STRING, "Se esperaba un string con la ruta de la imagen");
  std::unordered_map<std::string, std::string> parameters;
  if (current.type == TokenType::COMMA) {
    advance();
    parseParameters(parameters);
  }
  expect(TokenType::RPAREN, "Se esperaba ')'");

  BackgroundInfo bg;
  bg.name = nombre_fondo;
  bg.imagePath = imagen_path;
  bg.parameters = parameters;
  backgrounds[nombre_fondo] = bg;
}

void Parser::parseDefine() {
  advance();
  std::string name_personaje = current.value;
  expect(TokenType::IDENTIFIER, "Se esperaba nombre del personaje");
  std::string name_visible = current.value;
  expect(TokenType::STRING,
         "Se esperaba string con el nombre visible del personaje");
  expect(TokenType::LBRACKET, "Se esperaba '{'");
  Mode m;
  std::vector<std::unordered_map<std::string,Mode>> modos;
  parseModes(modos);
  expect(TokenType::RBRACKET, "Se esperaba '}'");
  CharacterInfo ch;
  ch.Internalname = name_personaje;
  ch.Visiblename = name_visible;
  ch.modes = modos;
  characters[name_personaje] = ch;
}

void Parser::parseScene() {
  advance(); // consume 'scene'
  std::string nombre_escena = current.value;
  expect(TokenType::IDENTIFIER, "Se esperaba nombre de escena o fondo");
  std::unordered_map<std::string, std::string> parameters;
  if (current.type == TokenType::LPAREN) {
    advance();
    parseParameters(parameters);
    expect(TokenType::RPAREN, "Se esperaba ')'");
  }
  SceneInfo sc;
  sc.BackgroundName = nombre_escena;
  sc.parameters = parameters;
  scenes.push_back(sc);
}

void Parser::parseShow() {
  advance();
  std::string name_personaje = current.value;
  std::string state_personaje;
  expect(TokenType::IDENTIFIER, "Se esperaba nombre del personaje o imagen");
  std::unordered_map<std::string, std::string> parameters;
  if (current.type == TokenType::IDENTIFIER) {
    state_personaje = current.value;
    advance();

  }
  if (current.type == TokenType::LPAREN) {
    advance();
    parseParameters(parameters);
    expect(TokenType::RPAREN, "Se esperaba ')'");
  }
  showInfo sh;
  sh.character = name_personaje;
  sh.mode = state_personaje;
  sh.paremeters = parameters;
  shows.push_back(sh);
}

void Parser::parseDialogue() {
  DialogoLinea dl;
  dl.linea = current.line;

  if (current.type == TokenType::STRING) {
    dl.text = current.value;
    advance();
    if (current.type == TokenType::LPAREN) {
      advance();
      parseParameters(dl.parameters);
      expect(TokenType::RPAREN, "Se esperaba ')'");
    }
  } else if (current.type == TokenType::IDENTIFIER) {
    dl.speaker = current.value;
    advance();
    dl.text = current.value;
    expect(TokenType::STRING, "Se esperaba diálogo entre comillas");
    if (current.type == TokenType::LPAREN) {
      advance();
      parseParameters(dl.parameters);
      expect(TokenType::RPAREN, "Se esperaba ')'");
    }
  } else {
    throw std::runtime_error("Diálogo inválido");
  }
  dialogues.push_back(dl);

}

void Parser::parseParameters(std::unordered_map<std::string,std::string>&parameters) {
  parseParameter(parameters);
  while (current.type == TokenType::COMMA) {
    advance();
    parseParameter(parameters);
  }
}

void Parser::parseParameter(std::unordered_map<std::string, std::string> &map) {
  std::string nombre_parametro = current.value;
  std::string valor_parametro;
  expect(TokenType::IDENTIFIER, "Se esperaba nombre de parámetro");
  expect(TokenType::COLON, "Se esperaba ':'");
  if (current.type == TokenType::INT || current.type == TokenType::FLOAT ||
      current.type == TokenType::IDENTIFIER ||
      current.type == TokenType::STRING) {
      valor_parametro = current.value;
    advance();
    map[nombre_parametro] = valor_parametro;
  } else {
    throw std::runtime_error("Valor de parámetro inválido");
  }
}

void Parser::parseModes(std::vector<std::unordered_map<std::string,Mode>> &mode){
  parseMode(mode);
  while (current.type == TokenType::COMMA) {
    advance();
    parseMode(mode);
  }
}

void Parser::parseMode(std::vector<std::unordered_map<std::string,Mode>> &mode){
  Mode m;
  std::unordered_map<std::string,Mode> map;
  std::string identificador_modo = current.value;
  expect(TokenType::IDENTIFIER, "Se esperaba un identificador para el modo");
  expect(TokenType::COLON, "Se esperaba ':'");
  expect(TokenType::LPAREN, "Se esperaba '('");
  std::string imagen_modo = current.value;
  expect(TokenType::STRING, "Se esperaba la imagen del modo");
  std::unordered_map<std::string, std::string> parameters;
  if (current.type == TokenType::COMMA) {
    advance();
    parseParameters(parameters);
  }
  expect(TokenType::RPAREN, "Se esperaba ')'");
  m.imagePath = imagen_modo;
  m.parameters = parameters;
  map[identificador_modo] = m;
  mode.push_back(map);

}
