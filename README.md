# Tic-Tac-Toe: Algorithms for Playing and Their Comparison

A C++20 console implementation of tic-tac-toe, built as a university course project on the topic **"Development and study of software solutions"**. The project goes beyond a simple game: it implements several AI players and compares them experimentally.

## Goals

- Implement a flexible game engine that supports an NxN board with K in a row (3x3 with 3 in a row by default).
- Implement several computer players of increasing strength:
  - random moves
  - a simple rule-based (heuristic) player: win, then block, then take the center
  - minimax
  - minimax with alpha-beta pruning
- Run experiments (games between every pair of AIs) and compare win/draw/loss results, nodes explored, and running time.

## Project status

- [x] Board representation, move validation, win/draw detection
- [x] Human vs. human console game
- [ ] Random and heuristic players
- [ ] Minimax player
- [ ] Alpha-beta pruning
- [ ] Human vs. computer and computer vs. computer modes
- [ ] Experiments (1000 games per pair, node counts, timing)
- [ ] Support for larger boards (for example 4x4 with 4 in a row)

## Project structure

```
final_project/
├── src/
│   ├── main.cpp          # menu and game loop
│   ├── board.hpp         # board, rules, win/draw detection
│   ├── players.hpp       # AI players (planned)
│   └── experiment.cpp    # automated experiments (planned)
├── results/              # experiment output for the paper
├── Makefile
└── README.md
```

The game logic (board, rules, AI) is kept separate from input/output, so the same core can later be reused behind a different interface (for example a web version).

## Requirements

- A C++20 compiler (`clang++` or `g++`)
- `make`

On macOS, install the command line tools with `xcode-select --install`.

## Build and run

```bash
git clone https://github.com/YOUR_USERNAME/final_project.git
cd final_project
make run
```

To build without running: `make`. To remove the compiled binary: `make clean`.

Or compile manually:

```bash
clang++ -std=c++20 -O2 -Wall -Wextra src/main.cpp -o game && ./game
```

## How to play

The board is shown with numbered rows and columns. On your turn, enter the row and column separated by a space:

```
X, enter row and column (e.g. 2 3): 2 2
```

Invalid moves (an occupied cell or a position outside the board) are rejected and you are asked again.

## Planned experiments

- Every pair of players plays 1000 games, and the win/draw/loss results are tabulated.
- Minimax and alpha-beta pruning are compared by the number of nodes explored and by time taken.
- Expected conclusion: minimax never loses on 3x3, and alpha-beta pruning gives the same result while exploring far fewer nodes.

Results will be added to the `results/` folder and summarized in the course paper.

## Author

Your Name, faculty number XXXXX

University of Plovdiv "Paisii Hilendarski"
