CXX = clang++
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra

game: src/main.cpp src/board.hpp
	$(CXX) $(CXXFLAGS) src/main.cpp -o game

run: game
	./game

clean:
	rm -f game