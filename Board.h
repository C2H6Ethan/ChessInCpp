#pragma once
#include <cstdint>

// Type Aliases
using Bitboard = uint64_t;

// Enums
enum Square {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

enum PieceType {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_TYPE_COUNT, NONE_TYPE = -1
};

enum Color {
    WHITE, BLACK, BOTH_COLORS, NONE_COLOR = -1
};

class Board {
private:
    Bitboard bitboards[BOTH_COLORS][PIECE_TYPE_COUNT];
    Bitboard occupancy[3];
    Color player_to_move;
    PieceType get_piece_type_on_square(Square s, Color piece_color);
    Color get_piece_color_on_square(Square s);

public:
    Board() {empty_board();}

    void empty_board();
    void setup();
    void make_move(Square from, Square to);
    void print();
};