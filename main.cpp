#include <iostream>

#include "Board.h"


int main() {
    Board board;
    board.setup();

    std::cout << "Initial position:\n";
    board.print();
    
    // Make a move (e2-e4)
    board.make_move(e2, e4);
    
    std::cout << "\nAfter e2-e4:\n";
    board.print();

    return 0;
}
