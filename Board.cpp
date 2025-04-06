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

    for (auto & i : mailbox) {
        i = {NO_PIECE_TYPE, WHITE};
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

    for (int color = WHITE; color <= BLACK; color++) {
        for (int pt = PAWN; pt <= KING; pt++) {
            occupancy[color] |= bitboards[color][pt];
            occupancy[2] |= bitboards[color][pt];
        }
    }

    mailbox[0] = {ROOK, WHITE};
    mailbox[1] = {KNIGHT, WHITE};
    mailbox[2] = {BISHOP, WHITE};
    mailbox[3] = {QUEEN, WHITE};
    mailbox[4] = {KING, WHITE};
    mailbox[5] = {BISHOP, WHITE};
    mailbox[6] = {KNIGHT, WHITE};
    mailbox[7] = {ROOK, WHITE};
    mailbox[8] = {PAWN, WHITE};
    mailbox[9] = {PAWN, WHITE};
    mailbox[10] = {PAWN, WHITE};
    mailbox[11] = {PAWN, WHITE};
    mailbox[12] = {PAWN, WHITE};
    mailbox[13] = {PAWN, WHITE};
    mailbox[14] = {PAWN, WHITE};
    mailbox[15] = {PAWN, WHITE};


    mailbox[48] = {PAWN, WHITE};
    mailbox[49] = {PAWN, WHITE};
    mailbox[50] = {PAWN, WHITE};
    mailbox[51] = {PAWN, WHITE};
    mailbox[52] = {PAWN, WHITE};
    mailbox[53] = {PAWN, WHITE};
    mailbox[54] = {PAWN, WHITE};
    mailbox[55] = {PAWN, WHITE};
    mailbox[56] = {ROOK, BLACK};
    mailbox[57] = {KNIGHT, BLACK};
    mailbox[58] = {BISHOP, BLACK};
    mailbox[59] = {QUEEN, BLACK};
    mailbox[60] = {KING, BLACK};
    mailbox[61] = {BISHOP, BLACK};
    mailbox[62] = {KNIGHT, BLACK};
    mailbox[63] = {ROOK, BLACK};

}

void Board::make_move(Square from, Square to) {
    PieceType pieceType = get_piece_type_on_square(from);
    if (pieceType == NO_PIECE_TYPE) {
        std::cout << "no piece at square";
        return;
    }

    Bitboard from_bb = BitboardUtil::square_to_bitboard(from);
    Bitboard to_bb = BitboardUtil::square_to_bitboard(to);

    // remove from source square
    bitboards[player_to_move][pieceType] ^= from_bb;
    occupancy[player_to_move] ^= from_bb;
    occupancy[2] ^= from_bb;

    // handle capture

    // add to destination square
    bitboards[player_to_move][pieceType] ^= to_bb;
    occupancy[player_to_move] ^= to_bb;
    occupancy[2] ^= to_bb;

    // update mailbox
    mailbox[to] = mailbox[from];
    mailbox[from] = {NO_PIECE_TYPE, WHITE};
}

void Board::print() {
    const char* whitePieceChars = "PNBRQK";
    const char* blackPieceChars = "pnbrqk";

    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " ";
        for (int file = 0; file < 8; file++) {
            Square s =  static_cast<Square>(rank * 8 + file);
            PieceType piece_type = get_piece_type_on_square(s);
            if (piece_type == NO_PIECE_TYPE) {
                std::cout << "." << " ";
            } else {
                Color piece_color = get_piece_color_on_square(s);
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
    return mailbox[s].color;
}


PieceType Board::get_piece_type_on_square(Square s) {
    return mailbox[s].type;
}


