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

consteval auto init_knight_attacks() {
    std::array<Bitboard, 64> table = {};

    const Bitboard not_a_file = 0xfefefefefefefefeULL;
    const Bitboard not_ab_file = 0xfcfcfcfcfcfcfcfcULL;
    const Bitboard not_h_file = 0x7f7f7f7f7f7f7f7fULL;
    const Bitboard not_gh_file = 0x3f3f3f3f3f3f3f3fULL;

    for (int square = 0; square < 64; square++) {
        Bitboard bb = BitboardUtil::square_to_bitboard(static_cast<Square>(square));
        Bitboard attacks = 0ULL;

        // 8 possible knight jumps
        attacks |= (bb << 17) & not_a_file;
        attacks |= (bb << 15) & not_h_file;
        attacks |= (bb << 10) & not_ab_file;
        attacks |= (bb << 6)  & not_gh_file;
        attacks |= (bb >> 17) & not_h_file;
        attacks |= (bb >> 15) & not_a_file;
        attacks |= (bb >> 10) & not_gh_file;
        attacks |= (bb >> 6)  & not_ab_file;

        table[square] = attacks;
    }

    return table;
}

consteval auto init_bishop_masks() {
    std::array<Bitboard, 64> masks = {};
    for (int sq = 0; sq < 64; ++sq) {
        Bitboard mask = 0ULL;
        int rank = sq / 8;
        int file = sq % 8;

        // Diagonal directions: SE, SW, NE, NW
        for (int r = rank + 1, f = file + 1; r < 8 && f < 8; ++r, ++f) {
            mask |= BitboardUtil::square_to_bitboard(static_cast<Square>(r * 8 + f));
        }
        for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; ++r, --f) {
            mask |= BitboardUtil::square_to_bitboard(static_cast<Square>(r * 8 + f));
        }
        for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; --r, ++f) {
            mask |= BitboardUtil::square_to_bitboard(static_cast<Square>(r * 8 + f));
        }
        for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; --r, --f) {
            mask |= BitboardUtil::square_to_bitboard(static_cast<Square>(r * 8 + f));
        }
        masks[sq] = mask;
    }
    return masks;
}

constexpr auto BISHOP_MASKS = init_bishop_masks();

inline std::array<std::array<Bitboard, 512>, 64> init_bishop_attacks() {
    std::array<std::array<Bitboard, 512>, 64> table = {};
    for (int sq = 0; sq < 64; ++sq) {
        Bitboard mask = BISHOP_MASKS[sq];
        int num_bits = __builtin_popcountll(mask);
        int num_occupancies = 1 << num_bits;

        for (int occ_idx = 0; occ_idx < num_occupancies; ++occ_idx) {
            Bitboard occ = 0ULL;
            Bitboard attacks = 0ULL;

            // Set occupancy bits
            Bitboard temp_mask = mask;
            for (int i = 0; i < num_bits; ++i) {
                int lsb = __builtin_ctzll(temp_mask);
                if (occ_idx & (1 << i)) {
                    occ |= BitboardUtil::square_to_bitboard(static_cast<Square>(lsb));
                }
                temp_mask &= temp_mask - 1;  // Clear LSB
            }

            // Simulate bishop attacks with occupancy
            int rank = sq / 8;
            int file = sq % 8;
            for (int r = rank + 1, f = file + 1; r < 8 && f < 8; ++r, ++f) {
                attacks |= BitboardUtil::square_to_bitboard(static_cast<Square>(r * 8 + f));
                if (occ & BitboardUtil::square_to_bitboard(static_cast<Square>(r * 8 + f))) break;
            }
            for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; ++r, --f) {
                attacks |= BitboardUtil::square_to_bitboard(static_cast<Square>(r * 8 + f));
                if (occ & BitboardUtil::square_to_bitboard(static_cast<Square>(r * 8 + f))) break;
            }
            for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; --r, ++f) {
                attacks |= BitboardUtil::square_to_bitboard(static_cast<Square>(r * 8 + f));
                if (occ & BitboardUtil::square_to_bitboard(static_cast<Square>(r * 8 + f))) break;
            }
            for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; --r, --f) {
                attacks |= BitboardUtil::square_to_bitboard(static_cast<Square>(r * 8 + f));
                if (occ & BitboardUtil::square_to_bitboard(static_cast<Square>(r * 8 + f))) break;
            }
            table[sq][occ_idx] = attacks;
        }
    }
    return table;
}


constexpr auto PAWN_PUSHES = init_pawn_pushes_table();
constexpr auto PAWN_ATTACKS = init_pawn_attacks();
constexpr auto KNIGHT_ATTACKS = init_knight_attacks();
std::array<std::array<Bitboard, 512>, 64> BISHOP_ATTACKS = init_bishop_attacks();


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
    game_ply = 0;
    full_move_counter = 1;
    en_passant_target_square = NO_SQUARE;
}

// ============= Setup Methods =============



void Board::setup() {
    setup_with_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Example: Bishop on e4 (square 28) with a blocking piece on f5 (square 37)
    Bitboard occupancy = BitboardUtil::square_to_bitboard(static_cast<Square>(37)); // f5
    Bitboard attacks = get_attacks(static_cast<Square>(28), occupancy, BISHOP_MASKS, BISHOP_ATTACKS);

    std::cout << "Bishop attacks for e4 with occupancy on f5:\n";
    BitboardUtil::print_bitboard(attacks);

    return 0;
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
    game_ply = std::stoi(tokens[4]);
    full_move_counter = std::stoi(tokens[5]);
}

// ============= Move Execution =============

void Board::move(Move m) {
    const Square from = m.from();
    const Square to = m.to();
    const MoveFlags type = m.flags();

    game_ply++;
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


void Board::put_piece(Square s, Piece p) {
    assert(mailbox[s].type == NO_PIECE_TYPE && "Square not empty");
    Bitboard bb = BitboardUtil::square_to_bitboard(s);
    bitboards[p.color][p.type] |= bb;
    occupancy[p.color] |= bb;
    occupancy[BOTH] |= bb;
    mailbox[s] = p;
}

void Board::remove_piece(Square s) {
    assert(mailbox[s].type != NO_PIECE_TYPE && "No piece to remove");
    Piece p = mailbox[s];
    Bitboard bb = BitboardUtil::square_to_bitboard(s);
    bitboards[p.color][p.type] &= ~bb;
    occupancy[p.color] &= ~bb;
    occupancy[BOTH] &= ~bb;
    mailbox[s].type = NO_PIECE_TYPE;
}


int Board::relative_dir(Direction dir) {
    return player_to_move == WHITE ? dir : -dir;
}

Bitboard Board::get_attacks_bb_for_sliding_piece(Square sq, const std::array<Bitboard, 64>& masks, const std::array<std::array<Bitboard, 512>, 64>& attacks) {
    // todo: add method in header
    // todo: just start with pseudo legal move generation and make this method useful
    // todo: use result in pseudo legal move generation, go through each set bit and create Move, add to Move vector and return
    // Calculate the occupancy index
    Bitboard mask = masks[sq];
    Bitboard relevant_blocking_squares = occupancy[BOTH] & mask;
    int num_bits = __builtin_popcountll(mask);
    int occ_idx = 0;

    // Calculate the occupancy index
    Bitboard temp_mask = mask;
    for (int i = 0; i < num_bits; ++i) {
        int lsb = __builtin_ctzll(temp_mask);
        if (relevant_blocking_squares & (1ULL << lsb)) {
            occ_idx |= (1 << i);
        }
        temp_mask &= temp_mask - 1;  // Clear LSB
    }

    // Return the attack bitboard, don't allow friendlies to be attacked
    return attacks[sq][occ_idx] & ~occupancy[player_to_move];
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