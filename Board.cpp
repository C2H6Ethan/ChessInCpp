#include "Board.h"

#include <sstream>
#include <vector>

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
    PieceType piece_type = get_piece_type_on_square(from);
    Color piece_color = get_piece_color_on_square(from);
    if (piece_type == NO_PIECE_TYPE) {
        std::cout << "no piece at square";
        return;
    }

    Bitboard from_bb = BitboardUtil::square_to_bitboard(from);
    Bitboard to_bb = BitboardUtil::square_to_bitboard(to);

    // remove from source square
    bitboards[player_to_move][piece_type] ^= from_bb;
    occupancy[player_to_move] ^= from_bb;
    occupancy[2] ^= from_bb; // both colors

    // capture
    if (occupancy[!player_to_move] & to_bb) {
        PieceType captured_piece_type = get_piece_type_on_square(to);

        // remove captured piece
        bitboards[!player_to_move][captured_piece_type] ^= to_bb;
        occupancy[!player_to_move] ^= to_bb;
        occupancy[2] ^= to_bb;
    }

    // todo: handle castling
    // todo: handle en passant


    if (promotion_piece_type != NO_PIECE_TYPE) {
        // important: promotion will only work if promotion_piece_type is passed to this method, otherwise it will stay a pawn, it's up to move generation to handle this
        piece_type = promotion_piece_type;
    }

    // add to destination square
    bitboards[player_to_move][piece_type] ^= to_bb;
    occupancy[player_to_move] ^= to_bb;
    occupancy[2] ^= to_bb; // both colors

    // update mailbox
    mailbox[to] = {piece_type, player_to_move};
    mailbox[from] = {NO_PIECE_TYPE, WHITE};

    // update counters
    half_move_clock++;
    if (piece_color == BLACK) full_move_counter++;

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


