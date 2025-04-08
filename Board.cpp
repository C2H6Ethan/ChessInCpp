#include "Board.h"

#include <sstream>
#include <vector>

#include "BitboardUtils.h"

#pragma region move bitboards precalculation

void Board::init_pawn_attacks() {
    Bitboard not_a_file = ~0x0101010101010101ULL;
    Bitboard not_h_file = ~0x8080808080808080ULL;
    for (int square = 0; square < 64; square++) {
        // white
        Bitboard west_attacks = BitboardUtil::square_to_bitboard(static_cast<Square>(square)) << 7;
        Bitboard east_attacks = BitboardUtil::square_to_bitboard(static_cast<Square>(square)) << 9;
        // remove moves on A and H file preventing warp issues
        west_attacks = west_attacks & not_h_file;
        east_attacks = east_attacks & not_a_file;

        pawn_attacks[WHITE][square] = west_attacks | east_attacks;

        // black
        east_attacks = BitboardUtil::square_to_bitboard(static_cast<Square>(square)) >> 7;
        west_attacks = BitboardUtil::square_to_bitboard(static_cast<Square>(square)) >> 9;
        // remove moves on A and H file preventing warp issues
        west_attacks = west_attacks & not_h_file;
        east_attacks = east_attacks & not_a_file;

        pawn_attacks[BLACK][square] = west_attacks | east_attacks;
    }
}


#pragma endregion

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

    init_pawn_attacks();

    player_to_move = WHITE;
    castling_rights = {true, true, true, true};
    half_move_clock = 0;
    full_move_counter = 1;
    en_passant_target_square = NO_SQUARE;
}

void Board::setup() {
    setup_with_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Board::setup_with_fen(std::string fen) {
    std::istringstream fen_stream(fen);
    std::vector<std::string> tokens;
    std::string token;

    while (std::getline(fen_stream, token, ' ')) {  // Extracts words separated by whitespace
        tokens.push_back(token);
    }

    std::string pieces_string = tokens[0];
    std::string player_to_move_string = tokens[1];
    std::string castling_rights_string = tokens[2];
    std::string en_passant_target_square_string = tokens[3];
    std::string half_move_clock_string = tokens[4];
    std::string full_move_counter_string = tokens[5];


    // pieces
    std::istringstream pieces_stream(pieces_string);
    std::vector<std::string> ranks;

    while (std::getline(pieces_stream, token, '/')) {
        ranks.insert(ranks.begin(), token);
    }

    std::string whitePieces = "PNBRQK";
    std::string blackPieces = "pnbrqk";
    int square_counter = 0;
    for (std::string rank : ranks) {
        for (char c : rank) {
            if (whitePieces.contains(c)) {
                size_t piece_type_index = whitePieces.find(c);
                PieceType type = PieceType(piece_type_index);
                Bitboard piece_bb = BitboardUtil::square_to_bitboard(static_cast<Square>(square_counter));
                bitboards[WHITE][type] |= piece_bb;
                occupancy[WHITE] |= piece_bb;
                occupancy[2] |= piece_bb; // both
                mailbox[square_counter] = {type, WHITE};
                square_counter++;
            } else if (blackPieces.contains(c)) {
                size_t piece_type_index = blackPieces.find(c);
                PieceType type = PieceType(piece_type_index);
                Bitboard piece_bb = BitboardUtil::square_to_bitboard(static_cast<Square>(square_counter));
                bitboards[BLACK][type] |= piece_bb;
                occupancy[BLACK] |= piece_bb;
                occupancy[2] |= piece_bb; // both
                mailbox[square_counter] = {type, BLACK};
                square_counter++;
            } else {
                int amount_of_empty_squares = c - '0';
                for (int i = 0; i < amount_of_empty_squares; i++) {
                    mailbox[square_counter] = {NO_PIECE_TYPE, WHITE};
                    square_counter++;
                }
            }
        }
    }

    // player to move
    if (player_to_move_string == "w") {
        player_to_move = WHITE;
    } else {
        player_to_move = BLACK;
    }

    // castling rights
    castling_rights = {false, false, false, false};
    for (char c : castling_rights_string) {
        if (c == 'K') {
            castling_rights.white_king_side = true;
        } else if (c == 'Q') {
            castling_rights.white_queen_side = true;
        } else if (c == 'k') {
            castling_rights.black_king_side = true;
        } else if (c == 'q') {
            castling_rights.black_queen_side = true;
        }
    }

    // en passant target square
    if (en_passant_target_square_string != "-") {
        auto s = SquareMap.find(en_passant_target_square_string);
        en_passant_target_square = s->second;
    }

    // counters
    half_move_clock = std::stoi(half_move_clock_string);
    full_move_counter = std::stoi(full_move_counter_string);
}

void Board::make_move(Square from, Square to, PieceType promotion_piece_type) {
    PieceType moving_piece_type = get_piece_type_on_square(from);
    Color moving_color = get_piece_color_on_square(from);
    if (moving_piece_type == NO_PIECE_TYPE) {
        std::cout << "no piece at square";
        return;
    }

    // update counters
    half_move_clock++;
    if (player_to_move == BLACK) full_move_counter++;

    // reset en passant target square
    en_passant_target_square = NO_SQUARE;

    Bitboard from_bb = BitboardUtil::square_to_bitboard(from);
    Bitboard to_bb = BitboardUtil::square_to_bitboard(to);

    // remove from source square
    bitboards[player_to_move][moving_piece_type] ^= from_bb;
    occupancy[player_to_move] ^= from_bb;
    occupancy[2] ^= from_bb; // both colors

    // capture
    if (occupancy[!moving_color] & to_bb) {
        half_move_clock = 0; // reset half move clock on capture

        PieceType captured_piece_type = get_piece_type_on_square(to);

        // remove captured piece
        bitboards[!moving_color][captured_piece_type] ^= to_bb;
        occupancy[!moving_color] ^= to_bb;
        occupancy[2] ^= to_bb;
    }

    if (moving_piece_type == ROOK) {
        if (moving_color == WHITE) {
            if (from == 0) {
                castling_rights.white_queen_side = false;
            } else if (from == 7) {
                castling_rights.white_king_side = false;
            }
        } else {
            if (from == 56) {
                castling_rights.black_queen_side = false;
            } else if (from == 63) {
                castling_rights.black_king_side = false;
            }
        }
    } else if (moving_piece_type == PAWN) {
        half_move_clock = 0; // reset half move clock on pawn move

        if (square_diff(from, to) == 16) {
            // checks if pawn double pushed
            en_passant_target_square = Square((from + to) / 2); // midpoint
        } else if (pawn_attacks[moving_color][from] & to_bb && get_piece_type_on_square(to) == NO_PIECE_TYPE) {
            // checks if pawn move was an attack move, then checks if there is no piece at the destination square
            // en passant capture move
            Bitboard pawn_to_remove_bb;
            if (moving_color == WHITE) {
                pawn_to_remove_bb = to_bb >> 8;
            } else {
                pawn_to_remove_bb = to_bb << 8;
            }

            // remove captured en passant pawn
            bitboards[!moving_color][PAWN] ^= pawn_to_remove_bb;
            occupancy[!moving_color] ^= pawn_to_remove_bb;
            occupancy[2] ^= pawn_to_remove_bb;
            mailbox[BitboardUtil::bitboard_to_square(pawn_to_remove_bb)] = {NO_PIECE_TYPE, WHITE};
        }

        if (promotion_piece_type != NO_PIECE_TYPE) {
            // important: promotion will only work if promotion_piece_type is passed to this method, otherwise it will stay a pawn, it's up to move generation to handle this
            moving_piece_type = promotion_piece_type;
        }
    } else if (moving_piece_type == KING) {
        if (moving_color == WHITE) {
            castling_rights.white_king_side = false;
            castling_rights.white_queen_side = false;
        } else {
            castling_rights.black_king_side = false;
            castling_rights.black_queen_side = false;
        }

        if (square_diff(from, to)) {
            // castle move
            int side = (to > from); // 0=queenside, 1=kingside
            Bitboard rook_move = castling_rook_moves[from][side];
            Square rook_from = (side == 0) ? (moving_color == WHITE ? a1 : a8)
                               : (moving_color == WHITE ? h1 : h8);
            Square rook_to   = (side == 0) ? (moving_color == WHITE ? d1 : d8)
                                           : (moving_color == WHITE ? f1 : f8);

            // replace rooks
            bitboards[moving_color][ROOK] ^= rook_move;
            occupancy[moving_color] ^= rook_move;
            occupancy[2] ^= rook_move;

            mailbox[rook_from] = {NO_PIECE_TYPE, WHITE};
            mailbox[rook_to] = {ROOK, moving_color};
        }
    }



    // add to destination square
    bitboards[moving_color][moving_piece_type] ^= to_bb;
    occupancy[moving_color] ^= to_bb;
    occupancy[2] ^= to_bb; // both colors

    // update mailbox
    mailbox[to] = {moving_piece_type, moving_color};
    mailbox[from] = {NO_PIECE_TYPE, WHITE};

    // update player to move
    player_to_move = player_to_move ? WHITE : BLACK;
}

void Board::print() {
    const char* whitePieceChars = "PNBRQK";
    const char* blackPieceChars = "pnbrqk";

    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " ";
        for (int file = 0; file < 8; file++) {
            Square s = static_cast<Square>(rank * 8 + file);
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


