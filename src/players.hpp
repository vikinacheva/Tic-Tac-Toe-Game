#pragma once
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "board.hpp"
using namespace std;

// Every player (human or AI) implements this interface,
// so the game loop doesn't care who is playing.
class Player {
public:
    virtual ~Player() = default;
    virtual int chooseMove(Board& board, Cell me) = 0;
    virtual string name() const = 0;
    virtual long long nodes() const { return -1; }   // -1 = this player doesn't count nodes
};

inline Result winFor(Cell c) { return c == Cell::X ? Result::XWins : Result::OWins; }

// Try a move, check if it wins, then take it back.
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

// Rules in order: win -> block -> center -> corner -> random.
class HeuristicPlayer : public Player {
public:
    explicit HeuristicPlayer(unsigned seed = random_device{}()) : rng_(seed) {}

    int chooseMove(Board& board, Cell me) override {
        auto moves = board.legalMoves();

        for (int m : moves)                                   // 1. win if possible
            if (isWinningMove(board, m, me)) return m;

        Cell opp = opponent(me);
        for (int m : moves)                                   // 2. block the opponent's win
            if (isWinningMove(board, m, opp)) return m;

        const int n = board.size();
        int center = (n / 2) * n + n / 2;
        if (board.isValidMove(center)) return center;         // 3. take the center

        vector<int> corners;                                  // 4. a random free corner
        for (int c : {0, n - 1, n * (n - 1), n * n - 1})
            if (board.isValidMove(c)) corners.push_back(c);
        if (!corners.empty()) return pickRandom(corners);

        return pickRandom(moves);                             // 5. anything else
    }
    string name() const override { return "Heuristic"; }

private:
    int pickRandom(const vector<int>& v) {
        uniform_int_distribution<size_t> pick(0, v.size() - 1);
        return v[pick(rng_)];
    }
    mt19937 rng_;
};

// Scores an unfinished position from the point of view of "me".
// Every line of K cells that holds only one player's marks counts 10^(number of marks).
inline int evaluate(const Board& board, Cell me) {
    const int n = board.size(), k = board.winLength();
    const int dr[] = {0, 1, 1, 1};
    const int dc[] = {1, 0, 1, -1};
    Cell opp = opponent(me);
    int score = 0;

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < n; ++c) {
            for (int d = 0; d < 4; ++d) {
                int endR = r + dr[d] * (k - 1);
                int endC = c + dc[d] * (k - 1);
                if (endR < 0 || endR >= n || endC < 0 || endC >= n) continue;

                int mine = 0, theirs = 0;
                for (int i = 0; i < k; ++i) {
                    Cell p = board.at(r + dr[d] * i, c + dc[d] * i);
                    if (p == me) ++mine;
                    else if (p == opp) ++theirs;
                }
                if (mine > 0 && theirs == 0) {
                    int v = 1;
                    for (int i = 0; i < mine; ++i) v *= 10;
                    score += v;
                } else if (theirs > 0 && mine == 0) {
                    int v = 1;
                    for (int i = 0; i < theirs; ++i) v *= 10;
                    score -= v;
                }
            }
        }
    }
    return score;
}

// Minimax: tries every possible game continuation, down to maxDepth moves.
class MinimaxPlayer : public Player {
public:
    explicit MinimaxPlayer(int maxDepth = 1000, unsigned seed = random_device{}())
        : maxDepth_(maxDepth), rng_(seed) {}

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
    static constexpr int INF = 1000000000;
    static constexpr int WIN = 1000000;

    int minimax(Board& board, Cell turn, Cell me, int depth) {
        ++nodes_;

        Result r = board.result();
        if (r == winFor(me)) return WIN - depth;
        if (r == winFor(opponent(me))) return depth - WIN;
        if (r == Result::Draw) return 0;
        if (depth >= maxDepth_) return evaluate(board, me);

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

    int maxDepth_;
    mt19937 rng_;
    long long nodes_ = 0;
};

// Minimax with alpha-beta pruning, down to maxDepth moves.
class AlphaBetaPlayer : public Player {
public:
    explicit AlphaBetaPlayer(int maxDepth = 1000, unsigned seed = random_device{}())
        : maxDepth_(maxDepth), rng_(seed) {}

    int chooseMove(Board& board, Cell me) override {
        nodes_ = 0;
        int bestScore = -INF;
        vector<int> bestMoves;

        for (int m : board.legalMoves()) {
            board.place(m, me);
            // alpha = bestScore - 1 keeps moves that tie with the best one exact,
            // so the set of best moves is the same as in plain minimax.
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
    static constexpr int INF = 1000000000;
    static constexpr int WIN = 1000000;

    int alphabeta(Board& board, Cell turn, Cell me, int depth, int alpha, int beta) {
        ++nodes_;

        Result r = board.result();
        if (r == winFor(me)) return WIN - depth;
        if (r == winFor(opponent(me))) return depth - WIN;
        if (r == Result::Draw) return 0;
        if (depth >= maxDepth_) return evaluate(board, me);

        if (turn == me) {                          // maximizing
            int best = -INF;
            for (int m : board.legalMoves()) {
                board.place(m, turn);
                int score = alphabeta(board, opponent(turn), me, depth + 1, alpha, beta);
                board.undo(m);
                best = max(best, score);
                alpha = max(alpha, best);
                if (alpha >= beta) break;          // beta cut-off
            }
            return best;
        } else {                                   // minimizing
            int best = INF;
            for (int m : board.legalMoves()) {
                board.place(m, turn);
                int score = alphabeta(board, opponent(turn), me, depth + 1, alpha, beta);
                board.undo(m);
                best = min(best, score);
                beta = min(beta, best);
                if (alpha >= beta) break;          // alpha cut-off
            }
            return best;
        }
    }

    int maxDepth_;
    mt19937 rng_;
    long long nodes_ = 0;
};