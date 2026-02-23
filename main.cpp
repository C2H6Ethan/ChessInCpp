#include <iostream>
#include <string>
#include "Validator.h"

int main(int argc, char* argv[]) {
    // Tier 1: system error — wrong number of args
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <fen> <uci_move>\n";
        return 1;
    }

    // Call the decoupled validation engine
    std::string result = process_move(argv[1], argv[2]);

    // Tier 1: system error — catastrophically malformed FEN
    if (result == "SYSTEM_ERROR") {
        std::cerr << "Error: failed to parse FEN\n";
        return 1;
    }

    // Tier 2: domain logic — print JSON and exit 0
    std::cout << result << std::endl;
    return 0;
}