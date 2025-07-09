#include <ast.hpp>
#include <fstream>
#include <iostream>
#include <lexer.hpp>
#include <parser.hpp>
#include <token.hpp>

int main() {
  try {
    Lexer lexer("main.sst");
    Parser parser(lexer);
    auto ast = parser.parseProgram();
    std::ofstream output("juego_generado.cpp");
    if (!output.is_open()) {
      throw std::runtime_error("No se pudo abrir archivo de salida");
    }

    ast->generateCode(output, 0);
    output.close();

    // 3. Compilación (ejemplo para Linux)
    system("g++ -o juego juego_generado.cpp -lsfml-graphics -lsfml-window "
           "-lsfml-system -lsfml-audio");

    std::cout << "Compilación exitosa. Ejecute: ./juego\n";
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
