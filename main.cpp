#include <iostream>
#include <string>
#include <vector>
#include "Board.h"
#include "Move.h"

int main(int argc, char* argv[]) {
    // Tier 1: system error — wrong number of args
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <fen> <uci_move>\n";
        return 1;
    }

    // Tier 1: system error — catastrophically malformed FEN
    Board board;
    try {
        board.setup_with_fen(argv[1]);
    } catch (...) {
        std::cerr << "Error: failed to parse FEN\n";
        return 1;
    }

    // Tier 2: domain logic — always exit 0 from here
    Move m = board.parse_uci_move(argv[2]);

    if (m == Move()) {
        std::cout << "{\"status\": \"INVALID\"}" << std::endl;
        return 0;
    }

    board.move(m);

    std::vector<Move> legal_moves = board.get_legal_moves();
    bool in_check = board.is_in_check(board.get_player_to_move());

    std::string game_state;
    if (legal_moves.empty() && in_check) {
        game_state = "CHECKMATE";
    } else if (legal_moves.empty() && !in_check) {
        game_state = "STALEMATE";
    } else if (board.get_halfmove_clock() >= 100) {
        game_state = "DRAW_50_MOVE";
    } else if (board.is_insufficient_material()) {
        game_state = "DRAW_INSUFFICIENT";
    } else {
        game_state = "ACTIVE";
    }

    std::cout << "{"
              << "\"status\": \"VALID\", "
              << "\"game_state\": \"" << game_state << "\", "
              << "\"new_fen\": \"" << board.to_fen() << "\""
              << "}" << std::endl;

    return 0;
}
