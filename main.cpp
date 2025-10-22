#include "Board.h"
#include "Move.h"
#include <iostream>

int main() {
    Board board;
    board.setup();
    std::cout << "Initial board:\n";
    board.print();

    // Define a couple of moves
    Move move1(a2, a4, DOUBLE_PUSH);
    Move move2(h7, h6, QUIET);

    // Before any move
    std::cout << "\n--- BEFORE ANY MOVE ---\n";
    std::cout << "game_ply: 0\n";

    // First move
    std::cout << "\n--- MOVE 1 ---\n";
    std::cout << "From: " << int(move1.from()) << ", To: " << int(move1.to())
              << ", Type: " << int(move1.flags()) << "\n";
    board.move(move1);
    std::cout << "After move1:\n";
    board.print();
    std::cout << "game_ply: 1\n";

    // Inspect history
    std::cout << "History entry bitboard after move1: " << board.history[1].entry << "\n";
    std::cout << "History epsq after move1: " << int(board.history[1].epsq) << "\n";
    std::cout << "History captured after move1: " << int(board.history[1].captured.type) << "\n";

    // Second move
    std::cout << "\n--- MOVE 2 ---\n";
    std::cout << "From: " << int(move2.from()) << ", To: " << int(move2.to())
              << ", Type: " << int(move2.flags()) << "\n";
    board.move(move2);
    std::cout << "After move2:\n";
    board.print();
    std::cout << "game_ply: 2\n";

    // Inspect history
    std::cout << "History entry bitboard after move2: " << board.history[2].entry << "\n";
    std::cout << "History epsq after move2: " << int(board.history[2].epsq) << "\n";
    std::cout << "History captured after move2: " << int(board.history[2].captured.type) << "\n";

    // Try undo
    std::cout << "\n--- UNDO MOVE 2 ---\n";
    board.undo_move(move2);
    board.print();
    std::cout << "game_ply: " << int(board.history[1].entry != 0 ? 1 : 0) << "\n";

    std::cout << "\n--- UNDO MOVE 1 ---\n";
    board.undo_move(move1);
    board.print();
    std::cout << "game_ply: 0\n";

    return 0;
}
