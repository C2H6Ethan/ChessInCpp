#include "Board.h"
#include "BitboardUtils.h"

void Board::empty_board() {
    for (auto& color : bitboards) {
        for (auto& pieceBitboard : color) {
            pieceBitboard = 0ULL;
        }
    }

    for (auto& occ: occupancy) {
        occ = 0ULL;
    }

    player_to_move = WHITE;
}

void Board::setup() {
    bitboards[WHITE][PAWN] = 0xFF00ULL;
    bitboards[WHITE][KNIGHT] = BitboardUtil::square_to_bitboard(b1) | BitboardUtil::square_to_bitboard(g1);
    bitboards[WHITE][ROOK] = BitboardUtil::square_to_bitboard(a1) | BitboardUtil::square_to_bitboard(h1);
    bitboards[WHITE][BISHOP] = BitboardUtil::square_to_bitboard(c1) | BitboardUtil::square_to_bitboard(f1);
    bitboards[WHITE][QUEEN] = BitboardUtil::square_to_bitboard(d1);
    bitboards[WHITE][KING] = BitboardUtil::square_to_bitboard(e1);
    bitboards[BLACK][PAWN] = 0x00FF000000000000ULL;
    bitboards[BLACK][KNIGHT] = BitboardUtil::square_to_bitboard(b8) | BitboardUtil::square_to_bitboard(g8);
    bitboards[BLACK][BISHOP] = BitboardUtil::square_to_bitboard(c8) | BitboardUtil::square_to_bitboard(f8);
    bitboards[BLACK][ROOK] = BitboardUtil::square_to_bitboard(a8) | BitboardUtil::square_to_bitboard(h8);
    bitboards[BLACK][QUEEN] = BitboardUtil::square_to_bitboard(d8);
    bitboards[BLACK][KING] = BitboardUtil::square_to_bitboard(e8);

    for (int color = 0; color < BOTH_COLORS; color++) {
        for (int pt = 0; pt < PIECE_TYPE_COUNT; pt++) {
            occupancy[color] |= bitboards[color][pt];
            occupancy[BOTH_COLORS] |= bitboards[color][pt];
        }
    }
}

void Board::make_move(Square from, Square to) {
    PieceType pieceType = get_piece_type_on_square(from, player_to_move);
    if (pieceType == PIECE_TYPE_COUNT) {
        std::cout << "no piece at square";
        return;
    }

    Bitboard from_bb = BitboardUtil::square_to_bitboard(from);
    Bitboard to_bb = BitboardUtil::square_to_bitboard(to);

    // remove from source square
    bitboards[player_to_move][pieceType] ^= from_bb;
    occupancy[player_to_move] ^= from_bb;
    occupancy[BOTH_COLORS] ^= from_bb;

    // handle capture

    // add to destination square
    bitboards[player_to_move][pieceType] ^= to_bb;
    occupancy[player_to_move] ^= to_bb;
    occupancy[BOTH_COLORS] ^= to_bb;
}

void Board::print() {
    const char* whitePieceChars = "PNBRQK";
    const char* blackPieceChars = "pnbrqk";

    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " ";
        for (int file = 0; file < 8; file++) {
            Square s =  static_cast<Square>(rank * 8 + file);
            Color piece_color = get_piece_color_on_square(s);
            if (piece_color == NONE_COLOR) {
                std::cout << "." << " ";
            } else {
                PieceType piece_type = get_piece_type_on_square(s, piece_color);
                if (piece_color == WHITE) {
                    std::cout << whitePieceChars[piece_type] << " ";
                } else {
                    std::cout << blackPieceChars[piece_type] << " ";
                }
            }
        }
        std::cout << '\n';
    }
    std::cout << "  a b c d e f g h\n";

}

// helper methods
Color Board::get_piece_color_on_square(Square s) {
    for (int c = 0; c < BOTH_COLORS; c++) {
        if (occupancy[c] & (1ULL << s)) {
            return static_cast<Color>(c);
        }
    }

    return NONE_COLOR; // no piece at square
}


PieceType Board::get_piece_type_on_square(Square s, Color piece_color) {
    for (int pt = 0; pt < PIECE_TYPE_COUNT; pt++) {
        if (bitboards[piece_color][pt] & (1ULL << s)) {
            return static_cast<PieceType>(pt);
        }
    }

    return NONE_TYPE; // no piece at square
}


