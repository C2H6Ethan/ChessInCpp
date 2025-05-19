#include "Board.h"

#include <array>
#include <cassert>

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

// Precompute attack/move tables
consteval auto init_pawn_pushes_table() {
    std::array<std::array<Bitboard, 64>, 2> table = {};
    constexpr Bitboard FileA = 0x0101010101010101ULL;
    constexpr Bitboard FileH = 0x8080808080808080ULL;

    for (int sq = 0; sq < 64; ++sq) {
        const Bitboard bb = 1ULL << sq;

        // White pawn pushes
        table[WHITE][sq] = (bb << 8); // Single push
        if (bb & 0xFF00ULL) { // If on 2nd rank (no need for Rank2 constant)
            table[WHITE][sq] |= (bb << 16); // Add double push
        }

        // Black pawn pushes
        table[BLACK][sq] = (bb >> 8); // Single push
        if (bb & 0xFF000000000000ULL) { // If on 7th rank
            table[BLACK][sq] |= (bb >> 16); // Add double push
        }
    }
    return table;
}

consteval auto init_pawn_attacks() {
    std::array<std::array<Bitboard, 64>, 2> table = {};
    const Bitboard not_a_file = ~0x0101010101010101ULL;
    const Bitboard not_h_file = ~0x8080808080808080ULL;

    for (int square = 0; square < 64; square++) {
        // White pawn attacks
        Bitboard west_attack = BitboardUtil::square_to_bitboard(static_cast<Square>(square)) << 7;
        Bitboard east_attack = BitboardUtil::square_to_bitboard(static_cast<Square>(square)) << 9;
        table[WHITE][square] = (west_attack & not_h_file) | (east_attack & not_a_file);

        // Black pawn attacks
        east_attack = BitboardUtil::square_to_bitboard(static_cast<Square>(square)) >> 7;
        west_attack = BitboardUtil::square_to_bitboard(static_cast<Square>(square)) >> 9;
        table[BLACK][square] = (west_attack & not_h_file) | (east_attack & not_a_file);
    }

    return table;
}

constexpr auto PAWN_PUSHES = init_pawn_pushes_table();
constexpr auto PAWN_ATTACKS = init_pawn_attacks();



// ============= Initialization Methods =============


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

void Board::move(Move m) {
    const Square from = m.from();
    const Square to = m.to();
    const MoveFlags type = m.flags();

    ++game_ply;
    history[game_ply] = UndoInfo(history[game_ply - 1]);
    history[game_ply].entry |= BitboardUtil::square_to_bitboard(to) | BitboardUtil::square_to_bitboard(from);

    switch (type) {
        case QUIET:
            // no piece at destination
            make_quiet_move(from, to);
            break;
        case DOUBLE_PUSH:
            // double pawn push
            make_quiet_move(from, to);
            history[game_ply].epsq = Square(m.from() + relative_dir(NORTH));
            break;
        case OO:
            // king side castle
            if (player_to_move == WHITE) {
                make_quiet_move(e1, g1);
                make_quiet_move(h1, f1);
            } else {
                make_quiet_move(e8, g8);
                make_quiet_move(h8, f8);
            }
            break;
        case OOO:
            // queen side castle
            if (player_to_move == WHITE) {
                make_quiet_move(e1, c1);
                make_quiet_move(a1, d1);
            } else {
                make_quiet_move(e8, c8);
                make_quiet_move(a8, d8);
            }
            break;
        case EN_PASSANT:
            make_quiet_move(from, to);
            remove_piece(static_cast<Square>(to + relative_dir(SOUTH)));
            break;
        case PR_KNIGHT:
            remove_piece(from);
            put_piece(to, Piece(KNIGHT, player_to_move));
            break;
        case PR_BISHOP:
            remove_piece(from);
            put_piece(to, Piece(BISHOP, player_to_move));
            break;
        case PR_ROOK:
            remove_piece(from);
            put_piece(to, Piece(ROOK, player_to_move));
            break;
        case PR_QUEEN:
            remove_piece(from);
            put_piece(to, Piece(QUEEN, player_to_move));
            break;
        case PC_KNIGHT:
            remove_piece(from);
            history[game_ply].captured = mailbox[to];
            remove_piece(to);
            put_piece(to, Piece(KNIGHT, player_to_move));
            break;
        case PC_BISHOP:
            remove_piece(from);
            history[game_ply].captured = mailbox[to];
            remove_piece(to);
            put_piece(to, Piece(BISHOP, player_to_move));
            break;
        case PC_ROOK:
            remove_piece(from);
            history[game_ply].captured = mailbox[to];
            remove_piece(to);
            put_piece(to, Piece(ROOK, player_to_move));
            break;
        case PC_QUEEN:
            remove_piece(from);
            history[game_ply].captured = mailbox[to];
            remove_piece(to);
            put_piece(to, Piece(QUEEN, player_to_move));
            break;
        case CAPTURE:
            history[game_ply].captured = mailbox[to];
            make_move(from, to);
            break;
    }

    player_to_move = static_cast<Color>(!static_cast<bool>(player_to_move));
}

void Board::undo_move(Move m) {
    // move must be last move made
    const Square from = m.from();
    const Square to = m.to();
    const MoveFlags type = m.flags();

    player_to_move = static_cast<Color>(!static_cast<bool>(player_to_move)); // switch player to move so it's the one who made the move

    switch (type) {
        case QUIET:
        case DOUBLE_PUSH:
            make_quiet_move(to, from);
            break;
        case OO:
            if (player_to_move == WHITE) {
                make_quiet_move(g1, e1);
                make_quiet_move(f1, h1);
            } else {
                make_quiet_move(g8, e8);
                make_quiet_move(f8, h8);
            }
            break;
        case OOO:
            if (player_to_move == WHITE) {
                make_quiet_move(c1, e1);
                make_quiet_move(d1, a1);
            } else {
                make_quiet_move(c8, e8);
                make_quiet_move(d8, a8);
            }
            break;
        case EN_PASSANT:
            make_quiet_move(to, from);
            put_piece(static_cast<Square>(to + relative_dir(SOUTH)), Piece(PAWN, player_to_move == WHITE ? BLACK : WHITE));
            break;
        case PR_KNIGHT:
        case PR_BISHOP:
        case PR_ROOK:
        case PR_QUEEN:
            remove_piece(to);
            put_piece(from, Piece(PAWN, player_to_move));
            break;
        case PC_KNIGHT:
        case PC_BISHOP:
        case PC_ROOK:
        case PC_QUEEN:
            remove_piece(to);
            put_piece(to, history[game_ply].captured);
            put_piece(from, Piece(PAWN, player_to_move));
            break;
        case CAPTURE:
            make_quiet_move(to, from);
            put_piece(to, history[game_ply].captured);
            break;
    }

    game_ply--;
}


// ============= Helper Methods =============

void Board::make_move(Square from, Square to) {
    // a piece may be at target square
    Piece moving_piece = mailbox[from];

    Piece destination_piece = mailbox[to];
    if (destination_piece.type != NO_PIECE_TYPE) {
        remove_piece(to);
    }

    remove_piece(from);
    put_piece(to, moving_piece);
}

void Board::make_quiet_move(Square from, Square to) {
    // no piece at destination square
    Piece moving_piece = mailbox[from];
    remove_piece(from);
    put_piece(to, moving_piece);
}


void Board::remove_piece(Square s) {
    // there must be a piece on this square
    assert(mailbox[s].type != NO_PIECE_TYPE && "Trying to remove a piece from an empty square.");
    Bitboard square_bb = BitboardUtil::square_to_bitboard(s);
    Color owner = get_piece_color_on_square(s);
    PieceType type = get_piece_type_on_square(s);

    bitboards[owner][type] ^= square_bb;
    occupancy[owner] ^= square_bb;
    occupancy[BOTH] ^= square_bb;

    mailbox[s].type = NO_PIECE_TYPE;
}

void Board::put_piece(Square s, Piece p) {
    // square must be empty
    assert(mailbox[s].type == NO_PIECE_TYPE && "Trying to ad a piece to a non empty square.");
    Bitboard square_bb = BitboardUtil::square_to_bitboard(s);
    Color owner = p.color;
    PieceType type = p.type;

    bitboards[owner][type] ^= square_bb;
    occupancy[owner] ^= square_bb;
    occupancy[BOTH] ^= square_bb;

    mailbox[s] = p;
}

int Board::relative_dir(Direction dir) {
    return player_to_move == WHITE ? dir : -dir;
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