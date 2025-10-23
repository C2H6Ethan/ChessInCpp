#include "Board.h"
#include "Move.h"
#include "BitboardUtils.h"
#include <iostream>
#include <chrono>   // <-- for timing

int main() {
    Board board;
    board.setup_with_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq - 0 1");
    board.move(Move(h1, g1, QUIET));
    board.move(Move(h7, h6, QUIET));
    board.move(Move(g1, h1, QUIET));
    board.undo_move(Move(g1, h1, QUIET));
    board.undo_move(Move(h7, h6, QUIET));
    board.undo_move(Move(h1, g1, QUIET));
    board.print();
    Move moveList[256];  // preallocated move buffer

    // --- Timing starts here ---
    auto start = std::chrono::high_resolution_clock::now();

    Move* end = board.generate_pseudo_legal_moves(moveList);


    auto stop = std::chrono::high_resolution_clock::now();
    // --- Timing ends here ---

    // Duration in microseconds
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    int moveCount = static_cast<int>(end - moveList);

    std::cout << "Generated " << moveCount << " pseudo-legal moves in "
              << duration.count() << " microseconds.\n";

    return 0;
}
