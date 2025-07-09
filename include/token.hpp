#pragma once
#include <string>
#include <unordered_map>

enum class State { START, IDENTIFIER, NUMBER, STRING, ERROR };

enum class TokenType {
  BACKGROUND,
  COMMENT,
  DEFINE,
  SHOW,
  HIDE,
  SCENE,
  MUSIC,
  PLAY,
  STOP,
  IDENTIFIER,
  STRING,
  FLOAT,
  INT,
  COLON,
  COMMA,
  LPAREN,
  RPAREN,
  LBRACKET,
  RBRACKET,
  END_OF_FILE,
  UNKNOWN
};

struct Token {
  TokenType type;
  std::string value;
  int line;

  Token(TokenType t, const std::string &v, int l)
      : type(t), value(v), line(l) {}
};

namespace std {
template <> struct hash<TokenType> {
  std::size_t operator()(const TokenType &t) const {
    return std::hash<int>()(static_cast<int>(t));
  }
};
} // namespace std

const std::unordered_map<TokenType, std::string> tokenStr = {
    {TokenType::BACKGROUND, "BACKGROUND"},
    {TokenType::COMMENT, "COMMENT"},
    {TokenType::DEFINE, "DEFINE"},
    {TokenType::SHOW, "SHOW"},
    {TokenType::SCENE, "SCENE"},
    {TokenType::MUSIC, "MUSIC"},
    {TokenType::PLAY, "PLAY"},
    {TokenType::STOP, "STOP"},
    {TokenType::IDENTIFIER, "IDENTIFIER"},
    {TokenType::STRING, "STRING"},
    {TokenType::INT, "INT"},
    {TokenType::FLOAT, "FLOAT"},
    {TokenType::COLON, "COLON"},
    {TokenType::COMMA, "COMMA"},
    {TokenType::LPAREN, "LPAREN"},
    {TokenType::RPAREN, "RPAREN"},
    {TokenType::LBRACKET, "LBRACKET"},
    {TokenType::RBRACKET, "RBRACKET"},
    {TokenType::UNKNOWN, "UNKNOWN"},
    {TokenType::END_OF_FILE, "EOF"}};
