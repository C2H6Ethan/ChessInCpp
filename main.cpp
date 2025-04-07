#include <iostream>

#include "Board.h"


int main() {
    Board board;
    board.setup();

    board.make_move(e2, f3);
    board.print();

    return 0;
}
