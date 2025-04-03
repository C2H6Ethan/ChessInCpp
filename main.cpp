#include <cstdint>
#include <iostream>

// Type Aliases
typedef uint64_t Bitboard;

// Enums
enum Square : int {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

// Board Utilities
constexpr Bitboard square_to_bitboard(Square s) {
    // gets bitboard with square set to 1
    return 1ULL << s; //shifts 0000000001 s places so if s is 3 (d1 in Square enum) then 0001 -> 1000
}

void print_bitboard(Bitboard bitboard) {
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " ";
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            bool isSet = (bitboard & (1ULL << square));
            std::cout << (isSet ? '1' : '.') << "  ";
        }
        std::cout << std::endl;
    }
    std::cout << "  a  b  c  d  e  f  g  h" << std::endl;

}

// Piece Bitboards
Bitboard white_pawns   = 0xFF00ULL;
Bitboard black_pawns   = 0x00FF000000000000ULL;
Bitboard white_knights = square_to_bitboard(b1) | square_to_bitboard(g1);
Bitboard black_knights = square_to_bitboard(b8) | square_to_bitboard(g8);
Bitboard white_rooks   = square_to_bitboard(a1) | square_to_bitboard(h1);
Bitboard black_rooks   = square_to_bitboard(a8) | square_to_bitboard(h8);
Bitboard white_bishops = square_to_bitboard(c1) | square_to_bitboard(f1);
Bitboard black_bishops = square_to_bitboard(c8) | square_to_bitboard(f8);
Bitboard white_queen   = square_to_bitboard(d1);
Bitboard black_queen   = square_to_bitboard(d8);
Bitboard white_king    = square_to_bitboard(e1);
Bitboard black_king    = square_to_bitboard(e8);

// --- Move Logic ---
void make_move(Bitboard& piece_bb, Square from, Square to) {
    piece_bb &= ~square_to_bitboard(from); // Clear 'from'
    piece_bb |= square_to_bitboard(to);    // Set 'to'
}

int main() {
    Bitboard all_pieces = white_pawns | black_pawns | white_knights | white_rooks | black_knights | black_rooks | white_bishops | black_bishops | white_queen | black_queen | white_king | black_king;
    print_bitboard(all_pieces);

    // Example: Move white pawn from A2 to A4
    make_move(white_pawns, a2, a4);
    all_pieces = white_pawns | black_pawns | white_knights | white_rooks | black_knights | black_rooks | white_bishops | black_bishops | white_queen | black_queen | white_king | black_king;
    std::cout << "\nAfter moving pawn A2->A4:\n";
    print_bitboard(all_pieces);

    return 0;
}