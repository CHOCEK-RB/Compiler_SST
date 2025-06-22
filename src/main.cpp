#include <cstddef>
#include <iostream>
#include <vector>
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

    printf("Personajes Definidos\n");
    for(const auto&[name,ch]:parser.characters){
      std::cout << "Nombre: " << name;
      std::cout << "\nImagine Name: " << ch.Visiblename;
      std::cout << "\nReal Name: " << ch.Internalname;
      std::cout << "\nEstados de Animo:\n";
      for(size_t i=0;i<ch.modes.size();i++){
        for(const auto& par:ch.modes[i]){
          std::cout<<"Feels " << par.first;
          std::cout << "\nImagen " << par.second.imagePath;
          for(const auto&[k,v]:par.second.parameters){
            std::cout << "\nParametro " << k;
            std::cout << "  Valor " << v << '\n';
          }
        } 
      }
    }
    printf("\nEscenas: \n");
    for(size_t i=0;i<parser.scenes.size();i++){
        std::cout << "Nombre: " << parser.scenes[i].BackgroundName;
        for(const auto& [dir, value]:parser.scenes[i].parameters){
          std::cout << "\nParametro: " << dir; 
          std:: cout << "   Valor: " << value;
        }
    }

    printf("\n\nShow: \n");
    for(size_t i=0;i<parser.shows.size();i++){
      std::cout << "Nombre: " << parser.shows[i].character;
      std::cout << " Modo: " << parser.shows[i].mode;
      for(const auto& [par,val]:parser.shows[i].paremeters){
        std::cout << "\nParametro Show: " << par;
        std::cout << "  Valor Show: " << val << '\n';
      }
    }

    printf("\n\n\nDialogos: \n\n");
    for(size_t i=0;i<parser.dialogues.size();i++){
      std::cout << "Nombre: ";
      if(parser.dialogues[i].speaker == "")
        std::cout << "SPEAKER ";
      else
        std::cout << parser.dialogues[i].speaker;
      std::cout << "\nTEXT: " << parser.dialogues[i].text;
      for(const auto& [par,val]:parser.dialogues[i].parameters){
        std::cout << "\nParametro: " << par;
        std::cout << "  Valor: " << val << '\n';
      }
      std::cout << "[Linea " << parser.dialogues[i].linea << "]\n";
    }


    
    std::cout << "¡Análisis sintáctico completado sin errores!\n";
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
