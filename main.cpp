#include <iostream>
typedef unsigned long long U64;

enum SquareIndex : int {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

constexpr U64 bitboard_from_square(SquareIndex s) {
    // gets bitboard with square set to 1
    return 1ULL << s; //shifts 0000000001 s places so if s is 3 (d1 in Square enum) then 0001 -> 1000
}

void print_board(U64 bitboard) {
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



int main() {
    U64 white_pawns = 0xFF00ULL;
    U64 black_pawns = 0x00FF000000000000ULL;
    U64 white_knights = bitboard_from_square(b1) | bitboard_from_square(g1);
    U64 black_knights = bitboard_from_square(b8) | bitboard_from_square(g8);
    U64 white_rooks = bitboard_from_square(a1) | bitboard_from_square(h1);
    U64 black_rooks = bitboard_from_square(a8) | bitboard_from_square(h8);
    U64 white_bishops = bitboard_from_square(c1) | bitboard_from_square(f1);
    U64 black_bishops = bitboard_from_square(c8) | bitboard_from_square(f8);
    U64 white_queen = bitboard_from_square(d1);
    U64 black_queen = bitboard_from_square(d8);
    U64 white_king = bitboard_from_square(e1);
    U64 black_king = bitboard_from_square(e8);

    U64 all_pieces = white_pawns | black_pawns | white_knights | white_rooks | black_knights | black_rooks | white_bishops | black_bishops | white_queen | black_queen | white_king | black_king;

    print_board(all_pieces);

    return 0;
}