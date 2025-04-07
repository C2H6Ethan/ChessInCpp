#include <iostream>

#include "Board.h"


int main() {
    Board board;
    board.setup();

    board.make_move(e2, e8, KNIGHT);
    board.print();

    return 0;
}
