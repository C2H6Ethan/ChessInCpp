#pragma once

#include "Board.h"

// Simplified move flags
enum MoveFlags : uint8_t {
    QUIET        = 0x00,  // Normal move
    EN_PASSANT   = 0x01,  // Special capture
    CASTLING     = 0x02,  // King + rook move

    // Promotion types (bits 3-4)
    PROMO_KNIGHT = 0x04,  // 0100
    PROMO_BISHOP = 0x05,  // 0101
    PROMO_ROOK   = 0x06,  // 0110
    PROMO_QUEEN  = 0x07,  // 0111

    // Masks
    PROMO_MASK   = 0x07   // Mask for all flags
};

class Move {
public:
    Move() : m_data(0) {}

    // Optimized constructor
    Move(Square from, Square to, uint8_t flags = QUIET)
        : m_data((flags << 12) | (from << 6) | to) {}

    // Fast accessors
    Square from() const { return static_cast<Square>((m_data >> 6) & 0x3F); }
    Square to() const { return static_cast<Square>(m_data & 0x3F); }
    uint8_t flags() const { return (m_data >> 12) & PROMO_MASK; }

    // Critical checks (kept for special moves)
    bool is_en_passant() const { return flags() == EN_PASSANT; }
    bool is_castling() const { return flags() == CASTLING; }
    bool is_promotion() const { return flags() >= PROMO_KNIGHT; }

    // Promotion type (branchless)
    PieceType promotion_type() const {
        return static_cast<PieceType>((flags() & 0x03) + KNIGHT);
    }

    // Comparison
    bool operator==(const Move& other) const { return m_data == other.m_data; }

    // Compact storage
    uint16_t value() const { return m_data; }

private:
    uint16_t m_data; // 16-bit: [flags:4][from:6][to:6]
};