#include "token.hpp"
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
  std::string name = current.value;

  expect(TokenType::IDENTIFIER, "Se esperaba el nombre del fondo");
  expect(TokenType::LPAREN, "Se esperaba '('");

  std::string imagePath = current.value;

  expect(TokenType::STRING, "Se esperaba un string con la ruta de la imagen");

  std::unordered_map<std::string, std::string> parameters;
  if (current.type == TokenType::COMMA) {
    advance();
    do{
      std::string key = current.value;
      expect(TokenType::IDENTIFIER, "se esperaba nombre del parametro");
      expect(TokenType::COLON, "Se esperaba \':\'");
      std::string value = current.value;
      advance();
      parameters[key]=value;
      if(current.type == TokenType::COMMA)
        advance();
    }while(current.type !=TokenType::RPAREN);
    //parseParameters();
  }
  expect(TokenType::RPAREN, "Se esperaba ')'");

  BackgroundInfo bg;
  bg.name = name;
  bg.imagePath = imagePath;
  bg.parameters = parameters;
  backgrounds[name] = bg;
}

void Parser::parseDefine() {
  advance();
  expect(TokenType::IDENTIFIER, "Se esperaba nombre del personaje");
  expect(TokenType::STRING,
         "Se esperaba string con el nombre visible del personaje");
  expect(TokenType::LBRACKET, "Se esperaba '{'");
  parseModes();
  expect(TokenType::RBRACKET, "Se esperaba '}'");
}

void Parser::parseScene() {
  advance(); // consume 'scene'
  expect(TokenType::IDENTIFIER, "Se esperaba nombre de escena o fondo");
  if (current.type == TokenType::LPAREN) {
    advance();
    parseParameters();
    expect(TokenType::RPAREN, "Se esperaba ')'");
  }
}

void Parser::parseShow() {
  advance();
  expect(TokenType::IDENTIFIER, "Se esperaba nombre del personaje o imagen");
  if (current.type == TokenType::IDENTIFIER) {
    advance();
  }
  if (current.type == TokenType::LPAREN) {
    advance();
    parseParameters();
    expect(TokenType::RPAREN, "Se esperaba ')'");
  }
}

void Parser::parseDialogue() {
  if (current.type == TokenType::STRING) {
    advance();
    if (current.type == TokenType::LPAREN) {
      advance();
      parseParameters();
      expect(TokenType::RPAREN, "Se esperaba ')'");
    }
  } else if (current.type == TokenType::IDENTIFIER) {
    advance();
    expect(TokenType::STRING, "Se esperaba diálogo entre comillas");
    if (current.type == TokenType::LPAREN) {
      advance();
      parseParameters();
      expect(TokenType::RPAREN, "Se esperaba ')'");
    }
  } else {
    throw std::runtime_error("Diálogo inválido");
  }
}

void Parser::parseParameters() {
  parseParameter();
  while (current.type == TokenType::COMMA) {
    advance();
    parseParameter();
  }
}

void Parser::parseParameter() {
  expect(TokenType::IDENTIFIER, "Se esperaba nombre de parámetro");
  expect(TokenType::COLON, "Se esperaba ':'");
  if (current.type == TokenType::INT || current.type == TokenType::FLOAT ||
      current.type == TokenType::IDENTIFIER ||
      current.type == TokenType::STRING) {
    advance();
  } else {
    throw std::runtime_error("Valor de parámetro inválido");
  }
}

void Parser::parseModes() {
  parseMode();
  while (current.type == TokenType::COMMA) {
    advance();
    parseMode();
  }
}

void Parser::parseMode() {
  expect(TokenType::IDENTIFIER, "Se esperaba un identificador para el modo");
  expect(TokenType::COLON, "Se esperaba ':'");
  expect(TokenType::LPAREN, "Se esperaba '('");
  expect(TokenType::STRING, "Se esperaba la imagen del modo");
  if (current.type == TokenType::COMMA) {
    advance();
    parseParameters();
  }
  expect(TokenType::RPAREN, "Se esperaba ')'");
}
