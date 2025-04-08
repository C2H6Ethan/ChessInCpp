#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>


using Bitboard = uint64_t;

enum Square : uint64_t {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8, NO_SQUARE
};

inline std::unordered_map<std::string, Square> SquareMap = {
    {"a1", a1}, {"b1", b1}, {"c1", c1}, {"d1", d1}, {"e1", e1}, {"f1", f1}, {"g1", g1}, {"h1", h1},
    {"a2", a2}, {"b2", b2}, {"c2", c2}, {"d2", d2}, {"e2", e2}, {"f2", f2}, {"g2", g2}, {"h2", h2},
    {"a3", a3}, {"b3", b3}, {"c3", c3}, {"d3", d3}, {"e3", e3}, {"f3", f3}, {"g3", g3}, {"h3", h3},
    {"a4", a4}, {"b4", b4}, {"c4", c4}, {"d4", d4}, {"e4", e4}, {"f4", f4}, {"g4", g4}, {"h4", h4},
    {"a5", a5}, {"b5", b5}, {"c5", c5}, {"d5", d5}, {"e5", e5}, {"f5", f5}, {"g5", g5}, {"h5", h5},
    {"a6", a6}, {"b6", b6}, {"c6", c6}, {"d6", d6}, {"e6", e6}, {"f6", f6}, {"g6", g6}, {"h6", h6},
    {"a7", a7}, {"b7", b7}, {"c7", c7}, {"d7", d7}, {"e7", e7}, {"f7", f7}, {"g7", g7}, {"h7", h7},
    {"a8", a8}, {"b8", b8}, {"c8", c8}, {"d8", d8}, {"e8", e8}, {"f8", f8}, {"g8", g8}, {"h8", h8}
};

constexpr uint64_t square_diff(Square a, Square b) {
    return (a > b) ? (a - b) : (b - a);
}

enum PieceType : uint8_t {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_TYPE_COUNT, NO_PIECE_TYPE
};

enum Color : uint8_t {
    WHITE, BLACK
};

// structs
struct Piece {
    PieceType type : 3;  // Uses 3 bits (0-7)
    Color color : 1;     // Uses 1 bit (0-1)
};

struct CastlingRights {
    bool white_king_side : 1;
    bool white_queen_side: 1;
    bool black_king_side : 1;
    bool black_queen_side: 1;
};

class Board {
private:
    Bitboard bitboards[2][6];
    Bitboard occupancy[3];
    Color player_to_move;
    Piece mailbox[64];
    int half_move_clock;
    int full_move_counter;
    CastlingRights castling_rights;
    Square en_passant_target_square;

    Bitboard pawn_attacks[2][64];
    Bitboard castling_rook_moves[64][2]; // [square][0=queenside, 1=kingside]

    PieceType get_piece_type_on_square(Square s);
    Color get_piece_color_on_square(Square s);
    void init_pawn_attacks();

public:
    Board() {empty_board();}

    void empty_board();
    void setup();
    void setup_with_fen(std::string fen);
    void make_move(Square from, Square to, PieceType promotion_piece_type = NO_PIECE_TYPE);
    void print();
};