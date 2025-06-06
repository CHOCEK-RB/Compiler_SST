#include <iostream>
#include <lexer.hpp>
#include <parser.hpp>
#include <token.hpp>

int main() {
  try {
    Lexer lexer("main.sst");
    Parser parser(lexer);
    parser.parseProgram();
    std::cout << "¡Análisis sintáctico completado sin errores!\n";
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
