#pragma once
#include <iostream>
#include <vector>
using namespace std;

enum class Cell : char { Empty = ' ', X = 'X', O = 'O' };
enum class Result { Ongoing, XWins, OWins, Draw };

inline Cell opponent(Cell c) { return c == Cell::X ? Cell::O : Cell::X; }

class Board {
public:
    Board(int size = 3, int winLength = 3)
        : n_(size), k_(winLength), cells_(size * size, Cell::Empty) {}

    int size() const { return n_; }
    int winLength() const { return k_; }
    Cell at(int row, int col) const { return cells_[row * n_ + col]; }
    Cell at(int idx) const { return cells_[idx]; }

    bool isValidMove(int idx) const {
        return idx >= 0 && idx < n_ * n_ && cells_[idx] == Cell::Empty;
    }

    bool place(int idx, Cell player) {
        if (!isValidMove(idx)) return false;
        cells_[idx] = player;
        return true;
    }

    void undo(int idx) { cells_[idx] = Cell::Empty; }

    vector<int> legalMoves() const {
        vector<int> moves;
        for (int i = 0; i < n_ * n_; ++i)
            if (cells_[i] == Cell::Empty) moves.push_back(i);
        return moves;
    }

    Result result() const {
        const int dr[] = {0, 1, 1, 1};   
        const int dc[] = {1, 0, 1, -1};
        for (int r = 0; r < n_; ++r) {
            for (int c = 0; c < n_; ++c) {
                Cell p = at(r, c);
                if (p == Cell::Empty) continue;
                for (int d = 0; d < 4; ++d) {
                    int endR = r + dr[d] * (k_ - 1);
                    int endC = c + dc[d] * (k_ - 1);
                    if (endR < 0 || endR >= n_ || endC < 0 || endC >= n_) continue;
                    bool all = true;
                    for (int i = 1; i < k_; ++i) {
                        if (at(r + dr[d] * i, c + dc[d] * i) != p) { all = false; break; }
                    }
                    if (all) return p == Cell::X ? Result::XWins : Result::OWins;
                }
            }
        }
        return legalMoves().empty() ? Result::Draw : Result::Ongoing;
    }

    void print() const {
        cout << "\n   ";
        for (int c = 0; c < n_; ++c) cout << ' ' << c + 1 << "  ";
        cout << '\n';
        for (int r = 0; r < n_; ++r) {
            cout << ' ' << r + 1 << ' ';
            for (int c = 0; c < n_; ++c) {
                cout << ' ' << static_cast<char>(at(r, c));
                if (c < n_ - 1) cout << " |";
            }
            cout << '\n';
            if (r < n_ - 1) {
                cout << "   ";
                for (int c = 0; c < n_; ++c) cout << "---" << (c < n_ - 1 ? "+" : "");
                cout << '\n';
            }
        }
        cout << '\n';
    }

private:
    int n_, k_;
    vector<Cell> cells_;
};