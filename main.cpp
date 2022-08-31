#include "Chip8.h"
#include <iostream>
#include <fstream>
#include <vector>
#include "cxxopts.hpp"


int main(int argc, char **argv) {
    cxxopts::Options options("chip9", "half-assed chip-8 interpreter");
    options.add_options()
            ("d,debug", "Enable debugging")
            ("cosmicVIP", "Set behaviour to match the original CosmicVIP", cxxopts::value<bool>()->default_value("false"))
            ("file", "File name of the ROM file", cxxopts::value<std::string>())
            ;
    options.parse_positional({"file"});
    auto result = options.parse(argc, argv);
    Chip8 interpreter = Chip8();

    if (!result.count("file")) {

        std::cout << "Please specify a path to a ROM file.\n";
        return 1;
    }

    std::ifstream file(result["file"].as<std::string>(), std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return 1;
    }

    std::vector<uint8_t> buffer;

    std::for_each(std::istreambuf_iterator<char>(file),
                  std::istreambuf_iterator<char>(),
                  [&buffer](const char c){
                      buffer.push_back(c);
                  });

    file.close();

    if (buffer.size() > 3596) {
        std::cout << "ROM is too large!" << std::endl;
        return 1;
    }

    interpreter.run(buffer, !result["cosmicVIP"].as<bool>());

    return 0;
}