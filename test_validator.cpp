#include "catch_amalgamated.hpp"
#include "Validator.h"
#include <string>
#include <vector>

struct TestCase {
    std::string name;
    std::string initial_fen;
    std::string uci_move;
    std::string expected_json_substring;
};

TEST_CASE("Stateless Move Validator Engine", "[validator]") {
    std::vector<TestCase> tests = {
        // --- 1. Basic Moves & State Updates ---
        {
            "Standard Pawn Push (Resets Halfmove)",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            "e2e4",
            "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
        },

        // --- 2. Castling Dynamics ---
        {
            "Valid Kingside Castle (Revokes Rights)",
            "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
            "e1g1",
            "r3k2r/8/8/8/8/8/8/R4RK1 b kq - 1 1"
        },
        {
            "Invalid Castle (King in Check)",
            "r3k2r/8/8/8/8/8/8/R3K2r w KQkq - 0 1",
            "e1g1",
            "\"status\": \"INVALID\""
        },
        {
            "Invalid Castle (Through Check)",
            "r3k2r/8/8/8/8/8/8/R3K1qR w KQkq - 0 1",
            "e1g1",
            "\"status\": \"INVALID\""
        },

        // --- 3. En Passant Mechanics ---
        {
            "Valid En Passant Capture",
            "rnbqkbnr/pppp1ppp/8/3Pp3/8/8/PPP1PPPP/RNBQKBNR w KQkq e6 0 3",
            "d5e6",
            "rnbqkbnr/pppp1ppp/4P3/8/8/8/PPP1PPPP/RNBQKBNR b KQkq - 0 3"
        },

        // --- 4. Pawn Promotions ---
        {
            "Valid Queen Promotion",
            "8/3P4/8/8/8/8/8/k6K w - - 0 1",
            "d7d8q",
            "3Q4/8/8/8/8/8/8/k6K b - - 0 1"
        },

        // --- 5. Absolute Pins ---
        {
            "Moving a Pinned Piece (Invalid)",
            "k6b/8/8/8/8/8/1R6/K7 w - - 0 1",
            "b2b3",
            "\"status\": \"INVALID\""
        },

        // --- 6. Game Terminations ---
        {
            "Checkmate State",
            "rnbqkbnr/pppp1ppp/8/4p3/6P1/5P2/PPPPP2P/RNBQKBNR b KQkq - 0 1",
            "d8h4",
            "\"game_state\": \"CHECKMATE\""
        },
        {
            "Stalemate State",
            "k7/8/8/8/8/8/5Q2/K7 w - - 0 1",
            "f2b6",
            "\"game_state\": \"STALEMATE\""
        },
        {
            "50-Move Rule Triggered",
            "k2q4/8/8/8/8/8/8/K2Q4 w - - 99 50",
            "a1a2",
            "\"game_state\": \"DRAW_50_MOVE\""
        },
        {
            "Insufficient Material Triggered",
            "k7/8/8/8/8/2q5/8/K2N4 w - - 0 1",
            "d1c3",
            "\"game_state\": \"DRAW_INSUFFICIENT\""
        }
    };

    for (const auto& tc : tests) {
        SECTION(tc.name) {
            // Call the actual function from Validator.cpp
            std::string actual_result = process_move(tc.initial_fen, tc.uci_move);

            // Verify the JSON output contains the expected data
            bool contains_expected = actual_result.find(tc.expected_json_substring) != std::string::npos;

            REQUIRE(contains_expected);
        }
    }
}