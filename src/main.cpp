#include <iostream>
#include <memory>
#include "board.hpp"
#include "players.hpp"
using namespace std;

unique_ptr<Player> makePlayer(int choice) {
    switch (choice) {
        case 1:  return make_unique<HumanPlayer>();
        case 2:  return make_unique<RandomPlayer>();
        default: return make_unique<HeuristicPlayer>();
    }
}

int askPlayerType(char symbol) {
    int choice = 0;
    while (choice < 1 || choice > 3) {
        cout << "Player " << symbol << ":  1) Human  2) Random  3) Heuristic  > ";
        if (!(cin >> choice)) exit(0);
    }
    return choice;
}

int main() {
    auto playerX = makePlayer(askPlayerType('X'));
    auto playerO = makePlayer(askPlayerType('O'));

    Board board;               // 3x3, 3 in a row
    Cell current = Cell::X;

    while (board.result() == Result::Ongoing) {
        board.print();
        Player& p = (current == Cell::X) ? *playerX : *playerO;
        int idx = p.chooseMove(board, current);
        board.place(idx, current);
        if (p.name() != "Human")
            cout << p.name() << " (" << static_cast<char>(current) << ") plays row "
                 << idx / board.size() + 1 << ", column " << idx % board.size() + 1 << "\n";
        current = opponent(current);
    }

    board.print();
    switch (board.result()) {
        case Result::XWins: cout << "X wins!\n"; break;
        case Result::OWins: cout << "O wins!\n"; break;
        default:            cout << "It's a draw.\n"; break;
    }
}