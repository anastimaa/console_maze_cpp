#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "dfsmaze.h"
#include "doctest.h"
#include <stdexcept>
#include <vector>

TEST_CASE("dfsmaze_generate invalid size") {
    SUBCASE("width too small") {
        CHECK_THROWS_AS(dfsmaze_generate(2, 5), std::invalid_argument);
    }

    SUBCASE("height too small") {
        CHECK_THROWS_AS(dfsmaze_generate(5, 2), std::invalid_argument);
    }

    SUBCASE("both dimensions too small") {
        CHECK_THROWS_AS(dfsmaze_generate(2, 2), std::invalid_argument);
    }
}

TEST_CASE("dfsmaze_generate valid size") {
    SUBCASE("minimum size 3x3") {
        auto maze = dfsmaze_generate(3, 3);
        CHECK(maze.size() == 3);
        CHECK(maze[0].size() == 3);

        for (int i = 0; i < 3; i++) {
            CHECK(maze[0][i] == WALL);
            CHECK(maze[2][i] == WALL);
            CHECK(maze[i][0] == WALL);
            CHECK(maze[i][2] == WALL);
        }

        CHECK(maze[1][1] == EMPTY);
    }

    SUBCASE("odd dimensions") {
        auto maze = dfsmaze_generate(11, 11);
        CHECK(maze.size() == 11);
        CHECK(maze[0].size() == 11);

        for (int i = 0; i < 11; i++) {
            CHECK(maze[0][i] == WALL);
            CHECK(maze[10][i] == WALL);
            CHECK(maze[i][0] == WALL);
            CHECK(maze[i][10] == WALL);
        }
    }

    SUBCASE("even dimensions") {
        auto maze = dfsmaze_generate(10, 10);
        CHECK(maze.size() == 10);
        CHECK(maze[0].size() == 10);
    }
}

TEST_CASE("dfs function validation") {
    SUBCASE("invalid start position - out of bounds") {
        std::vector<std::vector<char>> maze(5, std::vector<char>(5, WALL));
        CHECK_THROWS_AS(dfs(maze, 5, 5, -1, 2), std::out_of_range);
        CHECK_THROWS_AS(dfs(maze, 5, 5, 2, -1), std::out_of_range);
        CHECK_THROWS_AS(dfs(maze, 5, 5, 5, 2), std::out_of_range);
        CHECK_THROWS_AS(dfs(maze, 5, 5, 2, 5), std::out_of_range);
    }

    SUBCASE("invalid start position - not empty") {
        std::vector<std::vector<char>> maze(5, std::vector<char>(5, WALL));
        CHECK_THROWS_AS(dfs(maze, 5, 5, 1, 1), std::invalid_argument);
    }

    SUBCASE("valid maze generation") {
        const int height = 5;
        const int width = 5;
        std::vector<std::vector<char>> maze(height, std::vector<char>(width, WALL));

        int start_x = 1;
        int start_y = 1;
        maze[start_x][start_y] = EMPTY;

        CHECK_NOTHROW(dfs(maze, height, width, start_x, start_y));

        bool has_empty = false;
        for (int i = 1; i < height - 1; i++) {
            for (int j = 1; j < width - 1; j++) {
                if (maze[i][j] == EMPTY) {
                    has_empty = true;
                    break;
                }
            }
            if (has_empty)
                break;
        }
        CHECK(has_empty);
    }
}

TEST_CASE("dfsmaze_generate connectivity") {
    auto maze = dfsmaze_generate(11, 11);

    int empty_count = 0;
    for (const auto& row : maze) {
        for (char cell : row) {
            if (cell == EMPTY)
                empty_count++;
        }
    }
    CHECK(empty_count > 1);

    bool has_wall = false;
    for (const auto& row : maze) {
        for (char cell : row) {
            if (cell == WALL) {
                has_wall = true;
                break;
            }
        }
        if (has_wall)
            break;
    }
    CHECK(has_wall);
}