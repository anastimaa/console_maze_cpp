#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "dfsmaze.h"
#include "server.h"
#include <doctest/doctest.h>
#include <string>
#include <vector>

TEST_SUITE_BEGIN("generate_maze");

TEST_CASE("Level 1 maze dimensions") {
  generate_maze(1, true);
  CHECK(game_state.maze.size() == 10);
  CHECK(game_state.maze[0].size() == 15);
}

TEST_CASE("Level 2 maze dimensions") {
  generate_maze(2, true);
  CHECK(game_state.maze.size() == 15);
  CHECK(game_state.maze[0].size() == 20);
}

TEST_CASE("Level 3 maze dimensions") {
  generate_maze(3, true);
  CHECK(game_state.maze.size() == 20);
  CHECK(game_state.maze[0].size() == 30);
}

TEST_CASE("Start position") {
  generate_maze(1, true);
  CHECK(game_state.maze[1][1] == START);
}

TEST_CASE("Exit position") {
  generate_maze(2, true);
  int exit_y = game_state.maze.size() - 2;
  int exit_x = game_state.maze[0].size() - 2;
  CHECK(game_state.maze[exit_y][exit_x] == EXIT);
}

TEST_CASE("Doors around exit") {
  generate_maze(3, true);
  int exit_y = game_state.maze.size() - 2;
  int exit_x = game_state.maze[0].size() - 2;
  CHECK(game_state.maze[exit_y][exit_x - 1] == DOOR);
  CHECK(game_state.maze[exit_y - 1][exit_x] == DOOR);
}

TEST_CASE("Mobs count for level 1") {
  generate_maze(1, true);
  CHECK(game_state.mobs.size() == 2);
}

TEST_CASE("Mobs count for level 2") {
  generate_maze(2, true);
  CHECK(game_state.mobs.size() == 4);
}

TEST_CASE("Mobs count for level 3") {
  generate_maze(3, true);
  CHECK(game_state.mobs.size() == 6);
}

TEST_CASE("Keys generation") {
  generate_maze(3, true);
  int keys_count = 0;
  for (const auto &row : game_state.maze) {
    keys_count += std::count(row.begin(), row.end(), KEY);
  }
  CHECK(keys_count >= 3);
  CHECK(keys_count <= 5);
}

TEST_CASE("Gems generation") {
  generate_maze(3, true);
  int gems_count = 0;
  for (const auto &row : game_state.maze) {
    gems_count += std::count(row.begin(), row.end(), GEM);
  }
  CHECK(gems_count >= 3);
  CHECK(gems_count <= 5);
}

TEST_CASE("Random generation") {
  generate_maze(1, true);
  auto maze1 = game_state.maze;
  generate_maze(1, true);
  auto maze2 = game_state.maze;
  CHECK(maze1 != maze2);
}

TEST_CASE("Invalid level") {
  CHECK_THROWS_AS(generate_maze(0, true), std::invalid_argument);
  CHECK_THROWS_AS(generate_maze(4, true), std::invalid_argument);
  CHECK_THROWS_AS(generate_maze(-1, true), std::invalid_argument);
}

TEST_SUITE_END();

TEST_SUITE_BEGIN("reset_game_state");

TEST_CASE("Reset clears maze") {
  generate_maze(1, true);
  REQUIRE_FALSE(game_state.maze.empty());
  reset_game_state();
  CHECK(game_state.maze.empty());
}

TEST_CASE("Reset clears players") {
  game_state.players[1] = {1, 1, 3, 0, 0};
  game_state.players[2] = {2, 2, 3, 1, 2};
  REQUIRE_FALSE(game_state.players.empty());
  reset_game_state();
  CHECK(game_state.players.empty());
}

TEST_CASE("Reset clears mobs") {
  game_state.mobs.push_back({1, 2, 1});
  game_state.mobs.push_back({3, 4, -1});
  REQUIRE_FALSE(game_state.mobs.empty());
  reset_game_state();
  CHECK(game_state.mobs.empty());
}

TEST_CASE("Reset resets level to 0") {
  game_state.level = 3;
  REQUIRE(game_state.level == 3);
  reset_game_state();
  CHECK(game_state.level == 0);
}

TEST_CASE("Reset resets codegame to 0") {
  game_state.codegame = 9;
  REQUIRE(game_state.codegame == 9);
  reset_game_state();
  CHECK(game_state.codegame == 0);
}

TEST_CASE("Reset clears message") {
  game_state.message = "Test message";
  REQUIRE_FALSE(game_state.message.empty());
  reset_game_state();
  CHECK(game_state.message.empty());
}

TEST_CASE("Reset sets game_running to true") {
  game_running = false;
  REQUIRE_FALSE(game_running);
  reset_game_state();
  CHECK(game_running);
}

TEST_SUITE_END();

TEST_SUITE_BEGIN("check_step");

class CheckStepTestFixture {
public:
  CheckStepTestFixture() {
    reset_game_state();
    game_state.maze = {{WALL, WALL, WALL, WALL},
                       {WALL, EMPTY, EMPTY, WALL},
                       {WALL, KEY, DOOR, WALL},
                       {WALL, WALL, WALL, WALL}};

    game_state.players[1] = {1, 1, 3, 1, 0};
    game_state.players[2] = {1, 1, 3, 0, 0};
  }
};

TEST_CASE_FIXTURE(CheckStepTestFixture, "Positive empty cell") {
  CHECK(check_step(2, 1, 1) == true);
}

TEST_CASE_FIXTURE(CheckStepTestFixture, "Cell with key") {
  CHECK(check_step(1, 2, 2) == true);
}

TEST_CASE_FIXTURE(CheckStepTestFixture, "Open door with key") {
  CHECK(check_step(2, 2, 1) == true);
  CHECK(game_state.players[1].keys == 0);
}

TEST_CASE_FIXTURE(CheckStepTestFixture, "Into wall") {
  CHECK(check_step(0, 0, 1) == false);
}

TEST_CASE_FIXTURE(CheckStepTestFixture, "Open door without key") {
  CHECK(check_step(2, 2, 2) == false);
}

TEST_CASE_FIXTURE(CheckStepTestFixture, "Invalid coordinates") {
  CHECK_THROWS_AS(check_step(-1, 0, 1), std::out_of_range);
  CHECK_THROWS_AS(check_step(10, 10, 2), std::out_of_range);
}

TEST_CASE_FIXTURE(CheckStepTestFixture, "Invalid player ID") {
  CHECK_THROWS_AS(check_step(2, 1, 3), std::invalid_argument);
  CHECK_THROWS_AS(check_step(1, 1, -5), std::invalid_argument);
}
TEST_SUITE_END();

TEST_SUITE_BEGIN("process_player_move");

class PlayerMoveTestFixture {
public:
  PlayerMoveTestFixture() {
    reset_game_state();
    game_state.maze = {{WALL, WALL, WALL, WALL},
                       {WALL, EMPTY, KEY, WALL},
                       {WALL, MOB, DOOR, WALL},
                       {WALL, WALL, WALL, WALL}};

    game_state.players[1] = {1, 1, 3, 0, 0};
    game_state.mobs.push_back({1, 2, 1});
    game_state.message = "";
  }
};

TEST_CASE_FIXTURE(PlayerMoveTestFixture, "Pick up key") {
  process_player_move(1, "right");
  CHECK(game_state.players[1].keys == 1);
  CHECK(game_state.message == "!Player 1 picked up a key! Total keys: 1");
}

TEST_CASE_FIXTURE(PlayerMoveTestFixture, "Hit a mob") {
  process_player_move(1, "down");
  CHECK(game_state.players[1].lives == 2);
  CHECK(game_state.players[1].x == 1);
  CHECK(game_state.players[1].y == 1);
  CHECK(game_state.message == "!Player 1 hit a mob! Lives left: 2");
}

TEST_CASE_FIXTURE(PlayerMoveTestFixture, "Player opens door with key") {
  process_player_move(1, "right");
  CHECK(game_state.players[1].keys == 1);
  CHECK(game_state.maze[1][2] == '1');
  process_player_move(1, "down");
  CHECK(game_state.players[1].keys == 0);
  CHECK(game_state.maze[2][2] == '1');
}

TEST_CASE_FIXTURE(PlayerMoveTestFixture, "Invalid move direction") {
  process_player_move(1, "diagonally");
  CHECK(game_state.players[1].x == 1);
  CHECK(game_state.players[1].y == 1);
}

TEST_SUITE_END();