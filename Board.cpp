#include "Board.h"
#include "BitboardUtils.h"
#include <sstream>
#include <vector>
#include <iostream>

#include "Move.h"

// Constants
namespace {
    const std::string whitePiecesString = "PNBRQK";
    const std::string blackPiecesString = "pnbrqk";
}

// ============= Initialization Methods =============

void Board::init_pawn_attacks() {
    const Bitboard not_a_file = ~0x0101010101010101ULL;
    const Bitboard not_h_file = ~0x8080808080808080ULL;

    for (int square = 0; square < 64; square++) {
        // White pawn attacks
        Bitboard west_attack = BitboardUtil::square_to_bitboard(static_cast<Square>(square)) << 7;
        Bitboard east_attack = BitboardUtil::square_to_bitboard(static_cast<Square>(square)) << 9;
        pawn_attacks[WHITE][square] = (west_attack & not_h_file) | (east_attack & not_a_file);

        // Black pawn attacks
        east_attack = BitboardUtil::square_to_bitboard(static_cast<Square>(square)) >> 7;
        west_attack = BitboardUtil::square_to_bitboard(static_cast<Square>(square)) >> 9;
        pawn_attacks[BLACK][square] = (west_attack & not_h_file) | (east_attack & not_a_file);
    }
}

void Board::init_pawn_pushes() {
    for (int square = 0; square < 64; square++) {
        // White pawn pushes
        if (square >= 8 && square <= 15) {
            // 2nd rank
            Bitboard double_pawn_push = BitboardUtil::square_to_bitboard(static_cast<Square>(square)) << 8 | BitboardUtil::square_to_bitboard(static_cast<Square>(square)) << 16;
            pawn_pushes[WHITE][square] = double_pawn_push;
        } else if (square >= 16 && square <= 55) {
            // 3rd - 7th rank
            Bitboard pawn_push = BitboardUtil::square_to_bitboard(static_cast<Square>(square)) << 8;
            pawn_pushes[WHITE][square] = pawn_push;
        } else {
            pawn_pushes[WHITE][square] = 0ULL;
        }

        // Black pawn pushes
        if (square >= 48 && square <= 55) {
            // 7th rank
            Bitboard double_pawn_push = BitboardUtil::square_to_bitboard(static_cast<Square>(square)) >> 8 | BitboardUtil::square_to_bitboard(static_cast<Square>(square)) >> 16;
            pawn_pushes[BLACK][square] = double_pawn_push;
        } else if (square <= 47 && square >= 8) {
            // 6th - 2nd rank
            Bitboard pawn_push = BitboardUtil::square_to_bitboard(static_cast<Square>(square)) >> 8;
            pawn_pushes[BLACK][square] = pawn_push;
        } else {
            pawn_pushes[BLACK][square] = 0ULL;
        }
    }
}

Board::Board() {
    // Clear bitboards
    for (auto& color : bitboards) {
        for (auto& pieceBitboard : color) {
            pieceBitboard = 0ULL;
        }
    }

    // Clear occupancies
    for (auto& occ : occupancy) {
        occ = 0ULL;
    }

    // Clear mailbox
    for (auto& piece : mailbox) {
        piece = {NO_PIECE_TYPE, WHITE};
    }

    // Precompute attack tables
    init_pawn_attacks();
    init_pawn_pushes();

    // Set default game state
    player_to_move = WHITE;
    castling_rights = {true, true, true, true};
    half_move_clock = 0;
    full_move_counter = 1;
    en_passant_target_square = NO_SQUARE;
}

// ============= Setup Methods =============

void Board::setup() {
    setup_with_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Board::setup_with_fen(std::string fen) {
    std::istringstream fen_stream(fen);
    std::vector<std::string> tokens;
    std::string token;

    // Split FEN into tokens
    while (std::getline(fen_stream, token, ' ')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 6) return;

    // Parse piece placement
    std::istringstream pieces_stream(tokens[0]);
    std::vector<std::string> ranks;
    while (std::getline(pieces_stream, token, '/')) {
        ranks.insert(ranks.begin(), token);
    }

    int square = 0;
    for (const std::string& rank : ranks) {
        for (char c : rank) {
            if (whitePiecesString.contains(c)) {
                size_t type_idx = whitePiecesString.find(c);
                Bitboard bb = BitboardUtil::square_to_bitboard(static_cast<Square>(square));
                bitboards[WHITE][type_idx] |= bb;
                occupancy[WHITE] |= bb;
                occupancy[BOTH] |= bb;
                mailbox[square] = {static_cast<PieceType>(type_idx), WHITE};
                square++;
            }
            else if (blackPiecesString.contains(c)) {
                size_t type_idx = blackPiecesString.find(c);
                Bitboard bb = BitboardUtil::square_to_bitboard(static_cast<Square>(square));
                bitboards[BLACK][type_idx] |= bb;
                occupancy[BLACK] |= bb;
                occupancy[BOTH] |= bb;
                mailbox[square] = {static_cast<PieceType>(type_idx), BLACK};
                square++;
            }
            else {
                int empty_squares = c - '0';
                for (int i = 0; i < empty_squares; i++) {
                    mailbox[square++] = {NO_PIECE_TYPE, WHITE};
                }
            }
        }
    }

    // Parse active color
    player_to_move = (tokens[1] == "w") ? WHITE : BLACK;

    // Parse castling rights
    castling_rights = {false, false, false, false};
    for (char c : tokens[2]) {
        if (c == 'K') castling_rights.white_king_side = true;
        else if (c == 'Q') castling_rights.white_queen_side = true;
        else if (c == 'k') castling_rights.black_king_side = true;
        else if (c == 'q') castling_rights.black_queen_side = true;
    }

    // Parse en passant
    en_passant_target_square = (tokens[3] == "-") ?
        NO_SQUARE : SquareMap.at(tokens[3]);

    // Parse move counters
    half_move_clock = std::stoi(tokens[4]);
    full_move_counter = std::stoi(tokens[5]);
}

// ============= Move Execution =============

void Board::make_move(Move move) {
    Square from = move.from();
    Square to = move.to();

    PieceType moving_piece = get_piece_type_on_square(from);
    Color moving_color = get_piece_color_on_square(from);

    if (moving_piece == NO_PIECE_TYPE) {
        std::cout << "No piece at square\n";
        return;
    }

    // Update move counters
    half_move_clock++;
    if (player_to_move == BLACK) full_move_counter++;

    // Reset en passant target
    en_passant_target_square = NO_SQUARE;

    Bitboard from_bb = BitboardUtil::square_to_bitboard(from);
    Bitboard to_bb = BitboardUtil::square_to_bitboard(to);

    // Remove piece from source
    bitboards[moving_color][moving_piece] ^= from_bb;
    occupancy[moving_color] ^= from_bb;
    occupancy[BOTH] ^= from_bb;

    // Handle captures
    if (occupancy[!moving_color] & to_bb) {
        half_move_clock = 0;
        PieceType captured = get_piece_type_on_square(to);
        bitboards[!moving_color][captured] ^= to_bb;
        occupancy[!moving_color] ^= to_bb;
        occupancy[BOTH] ^= to_bb;
    }

    // Handle special moves
    switch (moving_piece) {
        case ROOK:
            update_rook_castling_rights(from, moving_color);
            break;

        case PAWN:
            handle_pawn_move(from, to, moving_color, to_bb, move.promotion_type());
            break;

        case KING:
            handle_king_move(from, to, moving_color);
            break;

        default:
            break;
    }

    // Add piece to destination (unless it was a promotion)
    if (moving_piece != PAWN || !move.is_promotion()) {
        bitboards[moving_color][moving_piece] ^= to_bb;
        occupancy[moving_color] ^= to_bb;
        occupancy[BOTH] ^= to_bb;
    }

    // Update mailbox
    mailbox[to] = {move.is_promotion() ? move.promotion_type() : moving_piece,
                  moving_color};
    mailbox[from] = {NO_PIECE_TYPE, WHITE};

    // Switch player
    player_to_move = static_cast<Color>(!static_cast<bool>(player_to_move));
}

// ============= Helper Methods =============

void Board::update_rook_castling_rights(Square from, Color color) {
    if (color == WHITE) {
        if (from == a1) castling_rights.white_queen_side = false;
        else if (from == h1) castling_rights.white_king_side = false;
    }
    else {
        if (from == a8) castling_rights.black_queen_side = false;
        else if (from == h8) castling_rights.black_king_side = false;
    }
}

void Board::handle_pawn_move(Square from, Square to, Color color, Bitboard to_bb, PieceType promotion) {
    half_move_clock = 0;

    // Handle double push
    if (square_diff(from, to) == 16) {
        en_passant_target_square = static_cast<Square>((from + to) / 2);
    }
    // Handle en passant
    else if ((pawn_attacks[color][from] & to_bb) && (get_piece_type_on_square(to) == NO_PIECE_TYPE)) {
        Bitboard captured_bb = (color == WHITE) ? (to_bb >> 8) : (to_bb << 8);
        bitboards[!color][PAWN] ^= captured_bb;
        occupancy[!color] ^= captured_bb;
        occupancy[BOTH] ^= captured_bb;
        mailbox[BitboardUtil::bitboard_to_square(captured_bb)] = {NO_PIECE_TYPE, WHITE};
    }

    // Handle promotion
    if (promotion != NO_PIECE_TYPE) {
        bitboards[color][PAWN] ^= to_bb;
        bitboards[color][promotion] ^= to_bb;
    }
}

void Board::handle_king_move(Square from, Square to, Color color) {
    // Remove castling rights
    if (color == WHITE) {
        castling_rights.white_king_side = false;
        castling_rights.white_queen_side = false;
    }
    else {
        castling_rights.black_king_side = false;
        castling_rights.black_queen_side = false;
    }

    // Handle castling
    if (square_diff(from, to) > 1) {
        int side = (to > from); // 0=queenside, 1=kingside
        Square rook_from = (side == 0) ?
            (color == WHITE ? a1 : a8) :
            (color == WHITE ? h1 : h8);

        Square rook_to = (side == 0) ?
            (color == WHITE ? d1 : d8) :
            (color == WHITE ? f1 : f8);

        Bitboard rook_move = BitboardUtil::square_to_bitboard(rook_from) | BitboardUtil::square_to_bitboard(rook_to);

        // Move rook
        bitboards[color][ROOK] ^= rook_move;
        occupancy[color] ^= rook_move;
        occupancy[BOTH] ^= rook_move;

        mailbox[rook_from] = {NO_PIECE_TYPE, WHITE};
        mailbox[rook_to] = {ROOK, color};
    }
}

// ============= Display Methods =============

void Board::print() {
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " ";
        for (int file = 0; file < 8; file++) {
            Square s = static_cast<Square>(rank * 8 + file);
            PieceType type = get_piece_type_on_square(s);

            if (type == NO_PIECE_TYPE) {
                std::cout << ". ";
            }
            else {
                Color color = get_piece_color_on_square(s);
                std::cout << (color == WHITE ?
                    whitePiecesString[type] :
                    blackPiecesString[type]) << " ";
            }
        }
        std::cout << '\n';
    }
    std::cout << "  a b c d e f g h\n";
}

// ============= Accessors =============

PieceType Board::get_piece_type_on_square(Square s) {
    return mailbox[s].type;
}

Color Board::get_piece_color_on_square(Square s) {
    return mailbox[s].color;
}