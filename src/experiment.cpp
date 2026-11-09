#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "board.hpp"
#include "players.hpp"
using namespace std;

const vector<string> NAMES = {"Random", "Heuristic", "Minimax", "AlphaBeta"};

unique_ptr<Player> makePlayer(int i) {
    switch (i) {
        case 0:  return make_unique<RandomPlayer>();
        case 1:  return make_unique<HeuristicPlayer>();
        case 2:  return make_unique<MinimaxPlayer>();
        default: return make_unique<AlphaBetaPlayer>();
    }
}

Result playGame(Player& px, Player& po) {
    Board board;
    Cell current = Cell::X;
    while (board.result() == Result::Ongoing) {
        Player& p = (current == Cell::X) ? px : po;
        board.place(p.chooseMove(board, current), current);
        current = opponent(current);
    }
    return board.result();
}

int main() {
    const int GAMES = 1000;
    const int REPEATS = 10;   // repetitions for timing

    // ---------- Experiment 1: every pair of players, 1000 games each ----------
    ofstream pairs("results/pairs.csv");
    if (!pairs) { cout << "Cannot write results/pairs.csv - does the results folder exist?\n"; return 1; }
    pairs << "X,O,XWins,Draws,OWins\n";

    cout << "=== Experiment 1: " << GAMES << " games per pair ===\n";
    cout << left << setw(12) << "X player" << setw(12) << "O player"
         << right << setw(9) << "X wins" << setw(9) << "Draws" << setw(9) << "O wins" << "\n";

    for (int x = 0; x < (int)NAMES.size(); ++x) {
        for (int o = 0; o < (int)NAMES.size(); ++o) {
            auto px = makePlayer(x);
            auto po = makePlayer(o);
            int xw = 0, draws = 0, ow = 0;
            for (int g = 0; g < GAMES; ++g) {
                Result r = playGame(*px, *po);
                if (r == Result::XWins) ++xw;
                else if (r == Result::OWins) ++ow;
                else ++draws;
            }
            cout << left << setw(12) << NAMES[x] << setw(12) << NAMES[o]
                 << right << setw(9) << xw << setw(9) << draws << setw(9) << ow << "\n";
            pairs << NAMES[x] << "," << NAMES[o] << "," << xw << "," << draws << "," << ow << "\n";
        }
    }

    // ---------- Experiment 2: nodes and time, minimax vs alpha-beta ----------
    ofstream nodesCsv("results/nodes.csv");
    nodesCsv << "Position,Algorithm,Nodes,TimeMs\n";

    cout << "\n=== Experiment 2: one move, minimax vs alpha-beta ===\n";
    cout << left << setw(26) << "Position" << setw(12) << "Algorithm"
         << right << setw(12) << "Nodes" << setw(14) << "Time (ms)" << "\n";

    MinimaxPlayer mm;
    AlphaBetaPlayer ab;
    vector<Player*> algos = {&mm, &ab};

    for (int pos = 0; pos < 2; ++pos) {
        Board board;
        Cell turn = Cell::X;
        string label = "empty board, X to move";
        if (pos == 1) {
            board.place(0, Cell::X);            // X took a corner
            turn = Cell::O;
            label = "X in corner, O to move";
        }
        for (Player* p : algos) {
            double totalMs = 0;
            for (int i = 0; i < REPEATS; ++i) {
                auto start = chrono::steady_clock::now();
                p->chooseMove(board, turn);
                auto end = chrono::steady_clock::now();
                totalMs += chrono::duration<double, milli>(end - start).count();
            }
            double avgMs = totalMs / REPEATS;
            cout << left << setw(26) << label << setw(12) << p->name()
                 << right << setw(12) << p->nodes() << setw(14) << fixed << setprecision(3) << avgMs << "\n";
            nodesCsv << label << "," << p->name() << "," << p->nodes() << "," << avgMs << "\n";
        }
    }

    cout << "\nSaved: results/pairs.csv and results/nodes.csv\n";
    return 0;
}