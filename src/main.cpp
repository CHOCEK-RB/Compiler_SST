#include <ast.hpp>
#include <fstream>
#include <iostream>
#include <lexer.hpp>
#include <parser.hpp>

int main(int argc, char *argv[]) {
  try {
    if (argc < 2) {
      throw std::runtime_error(
          "Ingrese el archivo que desea compilar [./compiler <file.sst>]");
    }

    const char *file = argv[1];
    Lexer lexer(file);
    Parser parser(lexer);
    auto ast = parser.parseProgram();

    std::ofstream output("story.json");
    if (!output.is_open()) {
      throw std::runtime_error("No se pudo abrir story.json");
    }

    ast->generateCode(output, 0);
    output.close();

    system("g++ -std=c++17 -o juego juego_generado.cpp -Iinclude "
           "-lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio");

    std::cout << "Compilación exitosa. Ejecute: ./juego\n";
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
