#include <iostream>

#include "BitboardUtils.h"
#include "Board.h"


int main() {
    Board board;
    board.setup_with_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
    board.make_move(e1, c1);
    board.print();
    return 0;
}
