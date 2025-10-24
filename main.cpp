#include "Board.h"
#include "Move.h"
#include "BitboardUtils.h"
#include <iostream>
#include <chrono>   // <-- for timing

int main() {
    Board board;
    board.setup_with_fen("rnbqkb1r/pppppppp/5n2/8/8/8/PPP1PPPP/RNBQKBNR w KQkq - 0 1");

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



    if (board.is_square_under_attack(d4, BLACK)) std::cout << "under attack";


    return 0;
}
