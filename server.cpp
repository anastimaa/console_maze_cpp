#include "server.h"
#include "dfsmaze.h"
#include <asio.hpp>
#include <iostream>
#include <map>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

/**
 * \file server.cpp
 * \brief Реализация сервера для двухпользовательской игры в лабиринт
 */

GameState game_state;
std::mutex game_mutex;
std::map<int, std::shared_ptr<asio::ip::tcp::socket>> connections;
bool game_running = true;
int next_player_id = 1;

void generate_maze(int level, bool quiet_mode) {
  if (level < 1 || level > 3) {
    throw std::invalid_argument{"Invalid level " + std::to_string(level) +
                                ". Valid levels are 1, 2 or 3."};
  }

  int width, height, mobs_count;
  if (level == 1) {
    width = 15;
    height = 10;
    mobs_count = 2;
  } else if (level == 2) {
    width = 20;
    height = 15;
    mobs_count = 4;
  } else {
    width = 30;
    height = 20;
    mobs_count = 6;
  }

  game_state.maze = dfsmaze_generate(width, height);

  game_state.maze[1][1] = START;
  game_state.maze[2][1] = EMPTY;
  game_state.maze[1][2] = EMPTY;
  game_state.maze[2][2] = EMPTY;

  int exit_y = height - 2;
  int exit_x = width - 2;
  game_state.maze[exit_y][exit_x] = EXIT;
  game_state.maze[exit_y - 1][exit_x - 1] = EMPTY;
  game_state.maze[exit_y][exit_x - 1] = EMPTY;
  game_state.maze[exit_y - 1][exit_x] = EMPTY;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> x_dist(2, width - 3);
  std::uniform_int_distribution<> y_dist(2, height - 3);
  std::uniform_int_distribution<> d_dist(-1, 1);

  std::lock_guard<std::mutex> lock(game_mutex);
  game_state.mobs.clear();

  for (int i = 0; i < mobs_count; i++) {
    int x = x_dist(gen);
    int y = y_dist(gen);
    int d = d_dist(gen);

    if (d == 0) {
      d = 1;
    }

    game_state.mobs.push_back({x, y, d});
    game_state.maze[y][x] = MOB;
  }

  std::uniform_int_distribution<> key_x_dist(1, width - 2);
  std::uniform_int_distribution<> key_y_dist(1, height - 2);
  int num_keys = std::uniform_int_distribution<>(3, 5)(gen);

  for (int i = 0; i < num_keys; i++) {
    int x, y;
    do {
      x = key_x_dist(gen);
      y = key_y_dist(gen);
    } while (game_state.maze[y][x] != EMPTY);

    game_state.maze[y][x] = KEY;
  }

  game_state.maze[exit_y][exit_x - 1] = DOOR;
  game_state.maze[exit_y - 1][exit_x] = DOOR;

  int num_gems = std::uniform_int_distribution<>(3, 5)(gen);

  for (int i = 0; i < num_gems; i++) {
    int x, y;
    do {
      x = key_x_dist(gen);
      y = key_y_dist(gen);
    } while (game_state.maze[y][x] != EMPTY);

    game_state.maze[y][x] = GEM;
  }

  game_state.players[1] = {1, 1, 3, 0, 0};
  game_state.players[2] = {1, 1, 3, 0, 0};

  game_state.codegame = std::uniform_int_distribution<>(1, 20)(gen);
  game_state.level = level;
  if (!quiet_mode) {
    std::cout << "Code the game: " << game_state.codegame
              << ", level: " << game_state.level << std::endl;
  }
}

void reset_game_state() {
  std::lock_guard<std::mutex> lock(game_mutex);
  game_state.maze.clear();
  game_state.players.clear();
  game_state.mobs.clear();
  game_state.level = 0;
  game_state.codegame = 0;
  game_state.message.clear();
  game_running = true;
}

bool check_step(int x, int y, int player_id) {
  if (x < 0 || y < 0 || x >= game_state.maze[0].size() ||
      y >= game_state.maze.size()) {
    throw std::out_of_range{"Coordinates out of bounds of the maze."};
  }

  if (game_state.players.find(player_id) == game_state.players.end()) {
    throw std::invalid_argument{"Player ID " + std::to_string(player_id) +
                                " not found in the game."};
  }

  if (x >= 1 && x < game_state.maze[0].size() - 1 && y >= 1 &&
      y < game_state.maze.size() - 1) {
    if (game_state.maze[y][x] == DOOR &&
        game_state.players[player_id].keys > 0) {
      game_state.players[player_id].keys--;
      game_state.message =
          "!Player " + std::to_string(player_id) + " open the door!";
      return true;
    } else if (game_state.maze[y][x] == EMPTY ||
               game_state.maze[y][x] == EXIT || game_state.maze[y][x] == MOB ||
               game_state.maze[y][x] == KEY || game_state.maze[y][x] == GEM) {
      return true;
    }
  }
  return false;
}

void process_player_move(int player_id, const std::string &move) {
  std::lock_guard<std::mutex> lock(game_mutex);
  Player &player = game_state.players[player_id];
  int current_x = player.x;
  int current_y = player.y;
  int new_x = current_x;
  int new_y = current_y;

  if (move == "up") {
    new_y = current_y - 1;
  } else if (move == "down") {
    new_y = current_y + 1;
  } else if (move == "left") {
    new_x = current_x - 1;
  } else if (move == "right") {
    new_x = current_x + 1;
  } else {
    return;
  }

  game_state.message = "";

  if (game_state.maze[new_y][new_x] == MOB) {
    player.lives--;
    std::cout << "Player " << player_id
              << " hit a mob! Lives left: " << player.lives << std::endl;
    game_state.message =
        "!Player " + std::to_string(player_id) +
        " hit a mob! Lives left: " + std::to_string(player.lives);
    game_state.maze[current_y][current_x] = EMPTY;
    player.x = 1;
    player.y = 1;
    new_y = 1;
    new_x = 1;
    game_state.maze[1][1] = '0' + player_id;

    if (player.lives == 0) {
      std::cout << "Player " << player_id
                << " lost! The other player is winner!" << std::endl;
      game_state.message = "Player " + std::to_string(player_id) +
                           " lost! The other player is winner!";
      game_running = false;
    }
  }

  if (game_state.maze[new_y][new_x] == KEY) {
    player.keys++;
    std::cout << "Player " << player_id
              << " picked up a key! Total keys: " << player.keys << std::endl;
    game_state.message =
        "!Player " + std::to_string(player_id) +
        " picked up a key! Total keys: " + std::to_string(player.keys);
    game_state.maze[new_y][new_x] = EMPTY;
  }

  if (game_state.maze[new_y][new_x] == GEM) {
    player.gems++;
    std::cout << "Player " << player_id
              << " picked up a gem!!!! Total gems: " << player.gems
              << std::endl;
    game_state.message =
        "!Player " + std::to_string(player_id) +
        " picked up a gem!!!! Total gems: " + std::to_string(player.gems);
    game_state.maze[new_y][new_x] = EMPTY;
  }

  if (check_step(new_x, new_y, player_id)) {
    bool was_start_point = (current_x == 1 && current_y == 1);
    bool other_player_remains = false;

    if (was_start_point) {
      for (const auto &p : game_state.players) {
        if (p.first != player_id && p.second.x == current_x &&
            p.second.y == current_y) {
          other_player_remains = true;
          break;
        }
      }
    }

    game_state.maze[current_y][current_x] = EMPTY;

    if (was_start_point && other_player_remains) {
      game_state.maze[current_y][current_x] = START;
    }

    player.x = new_x;
    player.y = new_y;

    if (game_state.maze[new_y][new_x] == EXIT) {
      std::cout << "Player " << player_id << " has exited the maze! Game over!"
                << std::endl;
      game_state.message = "Player " + std::to_string(player_id) +
                           " has escaped the maze! Game over!";
      game_running = false;
    } else {
      game_state.maze[new_y][new_x] = '0' + player_id;
    }
  }

  for (auto &mob : game_state.mobs) {
    if (mob.y >= 0 && mob.y < game_state.maze.size() && mob.x >= 0 &&
        mob.x < game_state.maze[0].size()) {
      game_state.maze[mob.y][mob.x] = EMPTY;
    }

    int new_mob_x = mob.x + mob.d;

    if (mob.y >= 0 && mob.y < game_state.maze.size() && new_mob_x >= 0 &&
        new_mob_x < game_state.maze[0].size() &&
        game_state.maze[mob.y][new_mob_x] != WALL &&
        game_state.maze[mob.y][new_mob_x] != DOOR &&
        game_state.maze[mob.y][new_mob_x] != GEM &&
        game_state.maze[mob.y][new_mob_x] != KEY) {
      mob.x = new_mob_x;
    } else {
      mob.d *= -1;
    }

    if (mob.y >= 0 && mob.y < game_state.maze.size() && mob.x >= 0 &&
        mob.x < game_state.maze[0].size()) {
      game_state.maze[mob.y][mob.x] = MOB;
    }
  }
}

void send_game_state(std::shared_ptr<asio::ip::tcp::socket> socket,
                     const GameState &state) {
  std::ostringstream serialized;

  for (const auto &row : state.maze) {
    for (char c : row) {
      serialized << c;
    }
    serialized << '\n';
  }

  serialized << "---PLAYERS---\n";

  for (const auto &player : state.players) {
    serialized << "Player " << player.first << ": "
               << "x=" << player.second.x << ", "
               << "y=" << player.second.y << ", "
               << "lives=" << player.second.lives << ", "
               << "keys=" << player.second.keys << ", "
               << "gems=" << player.second.gems << '\n';
  }

  serialized << "Message: " << state.message << '\n';
  serialized << "Level: " << state.level << '\n';
  serialized << "Game code: " << state.codegame << '\n';

  try {
    asio::write(*socket, asio::buffer(serialized.str()));
  } catch (const std::exception &e) {
    std::cerr << "Error sending game state: " << e.what() << std::endl;
  }
}

void broadcast_game_state() {
  std::lock_guard<std::mutex> lock(game_mutex);
  for (const auto &conn : connections) {
    if (conn.second->is_open()) {
      send_game_state(conn.second, game_state);
    }
  }
}

void handle_client(std::shared_ptr<asio::ip::tcp::socket> socket,
                   int player_id) {
  try {
    while (game_running) {
      asio::streambuf buffer;
      asio::error_code ec;
      asio::read_until(*socket, buffer, '\n', ec);

      if (ec == asio::error::eof || ec == asio::error::connection_reset) {
        std::cout << "Player " << player_id << " disconnected." << std::endl;
        break;
      } else if (ec) {
        throw asio::system_error(ec);
      }

      std::istream is(&buffer);
      std::string move;
      std::getline(is, move);

      if (move.empty()) {
        std::cout << "Player " << player_id << " disconnected." << std::endl;
        break;
      }

      std::cout << "Received move from Player " << player_id << ": " << move
                << std::endl;

      if (game_state.maze.empty() && game_state.codegame == 0) {
        int level = std::stoi(move);
        generate_maze(level);
        send_game_state(socket, game_state);
      } else {
        size_t colon_pos = move.find(':');
        if (colon_pos != std::string::npos) {
          std::string code_str = move.substr(colon_pos + 1);
          int received_code = std::stoi(code_str);
          if (received_code != game_state.codegame) {
            std::cout << "Received wrong code the game " << player_id
                      << std::endl;
            game_state.message = "Player " + std::to_string(player_id) +
                                 " inserted wrong code:" + code_str +
                                 ", the game closed!";
            game_running = false;
            break;
          }

          move = move.substr(0, colon_pos);
        }

        process_player_move(player_id, move);
        broadcast_game_state();
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "Error with Player " << player_id << ": " << e.what()
              << std::endl;
  }

  std::lock_guard<std::mutex> lock(game_mutex);
  connections.erase(player_id);
  std::cout << "Player " << player_id << " removed." << std::endl;
}

void run_server(asio::io_context &io_context, unsigned short port) {
  asio::ip::tcp::acceptor acceptor(
      io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
  std::cout << "Server started. Waiting for connections..." << std::endl;

  while (true) {
    reset_game_state();
    game_running = true;
    std::cout << "\nWaiting for new players..." << std::endl;

    std::vector<std::thread> client_threads;
    next_player_id = 1;

    while (next_player_id <= 2) {
      auto socket = std::make_shared<asio::ip::tcp::socket>(io_context);

      acceptor.accept(*socket);

      try {
        int player_id = next_player_id++;
        std::string client_ip = socket->remote_endpoint().address().to_string();
        std::cout << "Player " << player_id << " connected from " << client_ip
                  << std::endl;
        connections[player_id] = socket;
        asio::write(*socket, asio::buffer("PLAYER_ID " +
                                          std::to_string(player_id) + "\n"));
        client_threads.emplace_back(handle_client, socket, player_id);
      } catch (const std::exception &e) {
        std::cerr << "Error during player connection: " << e.what()
                  << std::endl;
        if (socket->is_open()) {
          socket->close();
        }
        continue;
      }
    }

    std::cout << "Both players connected. Game started!" << std::endl;

    while (game_running) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Game finished. Closing connections..." << std::endl;

    for (auto &conn : connections) {
      if (conn.second->is_open()) {
        conn.second->close();
      }
    }

    for (auto &t : client_threads) {
      if (t.joinable())
        t.join();
    }
    connections.clear();

    std::cout << "Ready for new game session." << std::endl;
  }
}