#include <iostream>

#include "BitboardUtils.h"
#include "Board.h"
#include "Move.h"


int main() {
    Board board;
    board.setup();

    board.move(Move(Square(a2), Square(a4)));
    board.move(Move(Square(h7), Square(h6)));
    board.move(Move(Square(a4), Square(a5)));
    board.move(Move(Square(b7), Square(b5)));
    board.print();

    Move en_passant_move = Move(Square(a5), Square(b6), EN_PASSANT);
    board.move(en_passant_move);
    board.print();

    board.undo_move(en_passant_move);
    board.print();

    return 0;
}
