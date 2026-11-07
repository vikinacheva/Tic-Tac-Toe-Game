#include <iostream>
#include "board.hpp"

using namespace std;

int main() {
    Board board;
    Cell current = Cell::X;

    while (board.result() == Result::Ongoing) {
        board.print();
        cout << static_cast<char>(current) << ", enter row and column (e.g. 2 3): ";
        int r, c;
        if (!(cin >> r >> c)) return 0;
        int idx = (r - 1) * board.size() + (c - 1);
        if (r < 1 || c < 1 || r > board.size() || c > board.size() || !board.place(idx, current)) {
            cout << "Invalid move, try again.\n";
            continue;
        }
        current = opponent(current);
    }

    board.print();
    switch (board.result()) {
        case Result::XWins: cout << "X wins!\n"; break;
        case Result::OWins: cout << "O wins!\n"; break;
        default:            cout << "It's a draw.\n"; break;
    }
}