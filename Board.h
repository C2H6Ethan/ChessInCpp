#pragma once
#include <cstdint>

// Type Aliases
using Bitboard = uint64_t;

// Enums
enum Square : uint64_t{
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

enum PieceType : uint8_t {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_TYPE_COUNT, NO_PIECE_TYPE
};

enum Color : uint8_t {
    WHITE, BLACK
};

struct Piece {
    PieceType type : 3;  // Uses 3 bits (0-7)
    Color color : 1;     // Uses 1 bit (0-1)
};

class Board {
private:
    Bitboard bitboards[2][6];
    Bitboard occupancy[3];
    Color player_to_move;
    Piece mailbox[64];
    PieceType get_piece_type_on_square(Square s);
    Color get_piece_color_on_square(Square s);

public:
    Board() {empty_board();}

    void empty_board();
    void setup();
    void make_move(Square from, Square to);
    void print();
};