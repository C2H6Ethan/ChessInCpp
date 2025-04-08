#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <iostream>

class Move;
// ============= Basic Types =============
using Bitboard = uint64_t;

enum Square : uint8_t {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8,
    NO_SQUARE
};

enum PieceType : uint8_t {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
    PIECE_TYPE_COUNT, NO_PIECE_TYPE
};

enum Color : uint8_t {
    WHITE, BLACK
};

// Separate constant for occupancy tracking
constexpr uint8_t BOTH = 2;

// ============= Utility Structures =============
struct Piece {
    PieceType type : 3;  // 3 bits for piece type (0-7)
    Color color : 1;     // 1 bit for color (WHITE/BLACK)
    uint8_t padding : 4; // Padding to complete 1 byte
};

struct CastlingRights {
    bool white_king_side : 1;
    bool white_queen_side : 1;
    bool black_king_side : 1;
    bool black_queen_side : 1;
};

// ============= Board Class =============
class Board {
private:
    // Bitboards [color][piece_type]
    Bitboard bitboards[BOTH][PIECE_TYPE_COUNT];

    // Occupancy [WHITE, BLACK, BOTH]
    Bitboard occupancy[3];

    // Game state
    Piece mailbox[64];          // 64 bytes
    Color player_to_move;       // 1 byte
    CastlingRights castling_rights; // 1 byte
    Square en_passant_target_square; // 1 byte
    uint8_t half_move_clock;    // 1 byte
    uint16_t full_move_counter; // 2 bytes

    // Precomputed attack tables
    Bitboard pawn_attacks[BOTH][64];          // [color][square]

    // Initialization
    void init_pawn_attacks();

    // Move helpers
    void update_rook_castling_rights(Square from, Color color);
    void handle_pawn_move(Square from, Square to, Color color, Bitboard to_bb, PieceType promotion);
    void handle_king_move(Square from, Square to, Color color);

public:
    // Construction
    Board();

    // Setup
    void setup();
    void setup_with_fen(std::string fen);

    // Game operations
    void make_move(Move move);

    // Accessors
    PieceType get_piece_type_on_square(Square s);
    Color get_piece_color_on_square(Square s);

    // Display
    void print();
};

// ============= Utility Functions =============
inline std::unordered_map<std::string, Square> SquareMap = {
    {"a1", a1}, {"b1", b1}, {"c1", c1}, {"d1", d1}, {"e1", e1}, {"f1", f1}, {"g1", g1}, {"h1", h1},
    // ... rest of square mappings ...
    {"a8", a8}, {"b8", b8}, {"c8", c8}, {"d8", d8}, {"e8", e8}, {"f8", f8}, {"g8", g8}, {"h8", h8}
};

constexpr uint64_t square_diff(Square a, Square b) {
    return (a > b) ? (a - b) : (b - a);
}