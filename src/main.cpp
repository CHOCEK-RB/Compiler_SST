#include <iostream>
#include <lexer.hpp>
#include <parser.hpp>
#include <token.hpp>

int main() {
  try {
    Lexer lexer("main.sst");
    Parser parser(lexer);
    parser.parseProgram();
    //Tabla de simbolos fondo
    printf("Fondos Definidos\n");
    for(const auto&[name,bg]:parser.backgrounds){
      std::cout << "nombre: " << name;
      std::cout << " Imagen: " << bg.imagePath << '\n';
      for(const auto&[k,v]:bg.parameters)
        std::cout << " " << k << " : " << v << '\n';
    }
    std::cout << "¡Análisis sintáctico completado sin errores!\n";
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
