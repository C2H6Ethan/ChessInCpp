#include <iostream>
#include <chrono>
#include "Board.h"
#include "Move.h"

uint64_t perft(Board& board, int depth) {
    if (depth == 0) return 1ULL;

    Move moves[256];
    Move *end = board.generate_legal_moves(moves);
    uint64_t nodes = 0;

    for (Move *m = moves; m < end; ++m) {
        board.move(*m);
        nodes += perft(board, depth - 1);
        board.undo_move(*m);
    }

    return nodes;
}


void start_perft(int depth, Board &board) {
    std::cout << "Running perft(" << depth << ")...\n";

    auto start = std::chrono::high_resolution_clock::now();
    uint64_t nodes = perft(board, depth);
    auto end = std::chrono::high_resolution_clock::now();

    double elapsed = std::chrono::duration<double>(end - start).count();
    std::cout << "Nodes: " << nodes << "\n";
    std::cout << "Time: " << elapsed << "s\n";
    std::cout << "NPS: " << (uint64_t)(nodes / elapsed) << " nodes/s\n";
}

int main() {
    Board board;
    board.setup_with_fen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    start_perft(4, board);


    return 0;
}
