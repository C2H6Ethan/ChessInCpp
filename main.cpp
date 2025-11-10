#include <iostream>
#include <chrono>
#include <string>
#include "Board.h"
#include "Move.h"

// Helper to get algebraic square names from enums
static const std::string square_to_string[64] = {
    "a1","b1","c1","d1","e1","f1","g1","h1",
    "a2","b2","c2","d2","e2","f2","g2","h2",
    "a3","b3","c3","d3","e3","f3","g3","h3",
    "a4","b4","c4","d4","e4","f4","g4","h4",
    "a5","b5","c5","d5","e5","f5","g5","h5",
    "a6","b6","c6","d6","e6","f6","g6","h6",
    "a7","b7","c7","d7","e7","f7","g7","h7",
    "a8","b8","c8","d8","e8","f8","g8","h8"
};

// Standard recursive perft
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

// Root perft that prints Stockfish-like breakdown
void perft_divide(Board &board, int depth) {
    Move moves[256];
    Move *end = board.generate_legal_moves(moves);

    uint64_t total_nodes = 0;

    for (Move *m = moves; m < end; ++m) {
        board.move(*m);
        uint64_t nodes = perft(board, depth - 1);
        board.undo_move(*m);

        std::string from = square_to_string[m->from()];
        std::string to   = square_to_string[m->to()];

        std::cout << from << to << ": " << nodes << "\n";
        total_nodes += nodes;
    }

    std::cout << "Total nodes: " << total_nodes << "\n";
}

void start_perft(int depth, Board &board) {
    std::cout << "Running perft(" << depth << ")...\n";

    auto start = std::chrono::high_resolution_clock::now();
    perft_divide(board, depth);
    auto end = std::chrono::high_resolution_clock::now();

    double elapsed = std::chrono::duration<double>(end - start).count();
    std::cout << "Time: " << elapsed << "s\n";
}

int main() {
    Board board;
    board.setup_with_fen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 0 1");
    start_perft(5, board);
    return 0;
}
