#include <ast.hpp>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <lexer.hpp>
#include <parser.hpp>
#include <string>
#include <unistd.h>

std::string getExecutablePath() {
  char result[1024];
  ssize_t count = readlink("/proc/self/exe", result, 1024);
  if (count != -1) {
    return std::filesystem::path(result).parent_path();
  }
  return ".";
}

void printHelp() {
  std::cout << "Uso: compiler <archivo_entrada.sst> [opciones]\n"
            << "\n"
            << "Opciones:\n"
            << "  -o <archivo_salida>   Especifica el nombre del ejecutable de "
               "salida (por defecto: 'juego').\n"
            << "  -h, --help              Muestra este mensaje de ayuda.\n";
}

int main(int argc, char *argv[]) {
  std::string inputFile;
  std::string outputFile = "juego";

  if (argc < 2) {
    printHelp();
    return 1;
  }

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      printHelp();
      return 0;

    } else if (arg == "-o") {
      if (i + 1 < argc) {
        outputFile = argv[++i];

      } else {
        std::cerr << "Error: La opción '-o' requiere un argumento."
                  << std::endl;
        return 1;
      }
    } else if (inputFile.empty()) {
      inputFile = arg;
    } else {
      std::cerr << "Error: Se especificó un archivo de entrada más de una vez."
                << std::endl;
      return 1;
    }
  }

  if (inputFile.empty()) {
    std::cerr << "Error: No se especificó ningún archivo de entrada."
              << std::endl;
    return 1;
  }

  try {
    std::string compilerPath = getExecutablePath();
    std::string tmpPath = compilerPath + "/.tmp";
    std::string resPath = compilerPath + "/../res";

    system(("mkdir -p " + tmpPath).c_str());

    system(("cp " + resPath + "/nlohmann_json.hpp " + tmpPath + "/").c_str());

    Lexer lexer(inputFile.c_str());
    Parser parser(lexer, compilerPath);
    auto ast = parser.parseProgram();

    std::string jsonPath = tmpPath + "/story.json";
    std::ofstream output(jsonPath);
    if (!output.is_open()) {
      throw std::runtime_error("No se pudo abrir " + jsonPath);
    }

    ast->generateCode(output, 0);
    output.close();

    std::string compileCommand = "g++ -std=c++17 -o " + outputFile + " " +
                                 tmpPath + "/juego_generado.cpp -I" + tmpPath +
                                 " -lsfml-graphics -lsfml-window "
                                 "-lsfml-system -lsfml-audio";

    int compileResult = system(compileCommand.c_str());
    if (compileResult != 0) {
      throw std::runtime_error("Falló la compilación del juego.");
    }

    std::cout << "Compilación exitosa. Ejecute: ./" << outputFile << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
