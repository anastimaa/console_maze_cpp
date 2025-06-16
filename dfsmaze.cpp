#include "dfsmaze.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <random>
#include <stdexcept>
#include <vector>

void dfs(std::vector<std::vector<char>>& maze, int height, int width,
    int start_x, int start_y) {
    if (!(0 <= start_x && start_x < height && 0 <= start_y && start_y < width)) {
        throw std::out_of_range("Invalid starting point: (" +
            std::to_string(start_x) + "; " +
            std::to_string(start_y) + ") is out of bounds");
    }
    if (maze[start_x][start_y] != EMPTY) {
        throw std::invalid_argument("Start position must be an empty cell");
    }

    int directions[4] = { 1, 2, 3, 4 };
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::shuffle(std::begin(directions), std::end(directions), gen);

    for (int direction : directions) {
        if (direction == 1) {
            if (start_x - 2 > 0 && maze[start_x - 2][start_y] != EMPTY) {
                maze[start_x - 1][start_y] = EMPTY;
                maze[start_x - 2][start_y] = EMPTY;
                dfs(maze, height, width, start_x - 2, start_y);
            }
        }
        else if (direction == 2) {
            if (start_x + 2 < height - 1 && maze[start_x + 2][start_y] != EMPTY) {
                maze[start_x + 1][start_y] = EMPTY;
                maze[start_x + 2][start_y] = EMPTY;
                dfs(maze, height, width, start_x + 2, start_y);
            }
        }
        else if (direction == 3) {
            if (start_y - 2 > 0 && maze[start_x][start_y - 2] != EMPTY) {
                maze[start_x][start_y - 1] = EMPTY;
                maze[start_x][start_y - 2] = EMPTY;
                dfs(maze, height, width, start_x, start_y - 2);
            }
        }
        else if (direction == 4) {
            if (start_y + 2 < width - 1 && maze[start_x][start_y + 2] != EMPTY) {
                maze[start_x][start_y + 1] = EMPTY;
                maze[start_x][start_y + 2] = EMPTY;
                dfs(maze, height, width, start_x, start_y + 2);
            }
        }
    }
}

std::vector<std::vector<char>> dfsmaze_generate(int width, int height) {
    if (width < 3 || height < 3) {
        throw std::invalid_argument("Maze size must be at least 3x3");
    }

    std::vector<std::vector<char>> maze(height, std::vector<char>(width, WALL));
    int start_x = 1;
    int start_y = 1;
    maze[start_x][start_y] = EMPTY;
    dfs(maze, height, width, start_x, start_y);

    std::srand(std::time(nullptr));
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            if (std::rand() % 100 >= 90) {
                maze[y][x] = EMPTY;
            }
        }
    }
    return maze;
}