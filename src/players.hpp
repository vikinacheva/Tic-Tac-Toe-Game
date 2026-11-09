#pragma once
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "board.hpp"
using namespace std;

class Player {
public:
    virtual ~Player() = default;
    virtual int chooseMove(Board& board, Cell me) = 0;
    virtual string name() const = 0;
    virtual long long nodes() const { return -1; }  
};

inline Result winFor(Cell c) { return c == Cell::X ? Result::XWins : Result::OWins; }

inline bool isWinningMove(Board& board, int idx, Cell player) {
    board.place(idx, player);
    bool win = board.result() == winFor(player);
    board.undo(idx);
    return win;
}

class HumanPlayer : public Player {
public:
    int chooseMove(Board& board, Cell me) override {
        const int n = board.size();
        while (true) {
            cout << static_cast<char>(me) << ", enter row and column (e.g. 2 3): ";
            int r, c;
            if (!(cin >> r >> c)) exit(0);
            int idx = (r - 1) * n + (c - 1);
            if (r >= 1 && r <= n && c >= 1 && c <= n && board.isValidMove(idx)) return idx;
            cout << "Invalid move, try again.\n";
        }
    }
    string name() const override { return "Human"; }
};

class RandomPlayer : public Player {
public:
    explicit RandomPlayer(unsigned seed = random_device{}()) : rng_(seed) {}
    int chooseMove(Board& board, Cell) override {
        auto moves = board.legalMoves();
        uniform_int_distribution<size_t> pick(0, moves.size() - 1);
        return moves[pick(rng_)];
    }
    string name() const override { return "Random"; }
private:
    mt19937 rng_;
};

class HeuristicPlayer : public Player {
public:
    explicit HeuristicPlayer(unsigned seed = random_device{}()) : rng_(seed) {}

    int chooseMove(Board& board, Cell me) override {
        auto moves = board.legalMoves();

        for (int m : moves)                                   
            if (isWinningMove(board, m, me)) return m;

        Cell opp = opponent(me);
        for (int m : moves)                                   
            if (isWinningMove(board, m, opp)) return m;

        const int n = board.size();
        int center = (n / 2) * n + n / 2;
        if (board.isValidMove(center)) return center;         

        vector<int> corners;                                  
        for (int c : {0, n - 1, n * (n - 1), n * n - 1})
            if (board.isValidMove(c)) corners.push_back(c);
        if (!corners.empty()) return pickRandom(corners);

        return pickRandom(moves);                             
    }
    string name() const override { return "Heuristic"; }

private:
    int pickRandom(const vector<int>& v) {
        uniform_int_distribution<size_t> pick(0, v.size() - 1);
        return v[pick(rng_)];
    }
    mt19937 rng_;
};

class MinimaxPlayer : public Player {
public:
    explicit MinimaxPlayer(unsigned seed = random_device{}()) : rng_(seed) {}

    int chooseMove(Board& board, Cell me) override {
        nodes_ = 0;
        int bestScore = -INF;
        vector<int> bestMoves;

        for (int m : board.legalMoves()) {
            board.place(m, me);
            int score = minimax(board, opponent(me), me, 1);
            board.undo(m);

            if (score > bestScore) {
                bestScore = score;
                bestMoves = {m};
            } else if (score == bestScore) {
                bestMoves.push_back(m);
            }
        }

        uniform_int_distribution<size_t> pick(0, bestMoves.size() - 1);
        return bestMoves[pick(rng_)];
    }

    string name() const override { return "Minimax"; }
    long long nodes() const override { return nodes_; }   

private:
    static constexpr int INF = 1000000;
    static constexpr int WIN = 1000;

    int minimax(Board& board, Cell turn, Cell me, int depth) {
        ++nodes_;

        Result r = board.result();
        if (r == winFor(me)) return WIN - depth;
        if (r == winFor(opponent(me))) return depth - WIN;
        if (r == Result::Draw) return 0;

        bool maximizing = (turn == me);
        int best = maximizing ? -INF : INF;

        for (int m : board.legalMoves()) {
            board.place(m, turn);
            int score = minimax(board, opponent(turn), me, depth + 1);
            board.undo(m);
            best = maximizing ? max(best, score) : min(best, score);
        }
        return best;
    }

    mt19937 rng_;
    long long nodes_ = 0;
};

class AlphaBetaPlayer : public Player {
public:
    explicit AlphaBetaPlayer(unsigned seed = random_device{}()) : rng_(seed) {}

    int chooseMove(Board& board, Cell me) override {
        nodes_ = 0;
        int bestScore = -INF;
        vector<int> bestMoves;

        for (int m : board.legalMoves()) {
            board.place(m, me);
            int score = alphabeta(board, opponent(me), me, 1, bestScore - 1, INF);
            board.undo(m);

            if (score > bestScore) {
                bestScore = score;
                bestMoves = {m};
            } else if (score == bestScore) {
                bestMoves.push_back(m);
            }
        }

        uniform_int_distribution<size_t> pick(0, bestMoves.size() - 1);
        return bestMoves[pick(rng_)];
    }

    string name() const override { return "AlphaBeta"; }
    long long nodes() const override { return nodes_; }

private:
    static constexpr int INF = 1000000;
    static constexpr int WIN = 1000;

    int alphabeta(Board& board, Cell turn, Cell me, int depth, int alpha, int beta) {
        ++nodes_;

        Result r = board.result();
        if (r == winFor(me)) return WIN - depth;
        if (r == winFor(opponent(me))) return depth - WIN;
        if (r == Result::Draw) return 0;

        if (turn == me) {                          
            int best = -INF;
            for (int m : board.legalMoves()) {
                board.place(m, turn);
                int score = alphabeta(board, opponent(turn), me, depth + 1, alpha, beta);
                board.undo(m);
                best = max(best, score);
                alpha = max(alpha, best);
                if (alpha >= beta) break;         
            }
            return best;
        } else {                                   
            int best = INF;
            for (int m : board.legalMoves()) {
                board.place(m, turn);
                int score = alphabeta(board, opponent(turn), me, depth + 1, alpha, beta);
                board.undo(m);
                best = min(best, score);
                beta = min(beta, best);
                if (alpha >= beta) break;          
            }
            return best;
        }
    }

    mt19937 rng_;
    long long nodes_ = 0;
};