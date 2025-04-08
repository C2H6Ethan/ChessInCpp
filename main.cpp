#include <iostream>

#include "BitboardUtils.h"
#include "Board.h"
#include "Move.h"


int main() {
    Board board;
    board.setup_with_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
    Move move(e2, e4, PROMO_BISHOP);
    board.make_move(move);
    board.print();
    return 0;
}
