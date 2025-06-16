#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <map>
#include <conio.h>
#include <asio.hpp>
#include "client.h"


/**
 * \file client.cpp
 * \brief Реализация клиента для двухпользовательской игры в лабиринт
 */

GameState game_state;
std::mutex game_mutex;
bool game_running = true;


char get_char() {
    return _getch();
}


void clear_screen() {
    system("cls");
}


void game_menu() {
    clear_screen();
    std::cout << "    --- Menu ---" << std::endl;
    std::cout << " N) Create new game" << std::endl;
    std::cout << " J) Join existing game" << std::endl;

    std::string choice;
    std::cout << "\n    Insert command (N/J): ";
    if (!(std::cin >> choice)) {
        throw std::runtime_error("Input error");
    }
    transform(choice.begin(), choice.end(), choice.begin(), ::toupper);

    if (choice == "N") {
        std::string level;
        std::cout << "\n    Insert level game (1,2,3): ";
        if (!(std::cin >> level)) {
            throw std::runtime_error("Input error");
        }

        if (level == "1" || level == "2" || level == "3") {
            game_state.level = stoi(level);
            game_state.codegame = "NEW";
        }
        else {
            throw std::invalid_argument("Invalid level. Please enter 1, 2, or 3.");
        }
    }
    else if (choice == "J") {
        std::cout << "\n    Insert code game: ";
        if (!(std::cin >> game_state.codegame)) {
            throw std::runtime_error("Input error");
        }

        if (!all_of(game_state.codegame.begin(), game_state.codegame.end(), ::isdigit)) {
            throw std::invalid_argument("Invalid code! Please restart the game.");
        }
    }
    else {
        throw std::invalid_argument("Invalid choice. Please restart the game.");
    }
}

void print_game_state(const GameState& state) {
    clear_screen();

    std::cout << "\n--- Current game state ---" << std::endl;

    if (!state.maze.empty()) {
        std::vector<std::vector<char>>display_maze = state.maze;

        for (const auto& player : state.players) {
            int y = player.second.y;
            int x = player.second.x;
            if (y < display_maze.size() && x < display_maze[y].size()) {
                if (x == 1 && y == 1) {
                    bool both_at_start = true;
                    for (const auto& p : state.players) {
                        if (p.second.x != 1 || p.second.y != 1) {
                            both_at_start = false;
                            break;
                        }
                    }
                    if (both_at_start) {
                        continue;
                    }
                }
                display_maze[y][x] = '0' + player.first;
            }
        }

        for (const auto& row : display_maze) {
            for (char c : row) {
                std::cout << c;
            }
            std::cout << std::endl;
        }
    }
    else {
        std::cout << "Maze is empty or not received yet" << std::endl;
    }

    std::cout << "\nPlayers:" << std::endl;
    for (const auto& player : state.players) {
        std::cout << "Player " << player.first << ": x=" << player.second.x
            << ", y=" << player.second.y << ", lives=" << player.second.lives
            << ", keys=" << player.second.keys << ", gems=" << player.second.gems << std::endl;
    }

    std::cout << "-----------------------------" << std::endl;
    std::cout << "Level: " << state.level << std::endl;

    if (!state.codegame.empty()) {
        std::cout << "Game code: " << state.codegame << std::endl;
    }

    if (!state.message.empty()) {
        std::cout << "Message: " << state.message << std::endl;
    }
}


void receive_data(asio::ip::tcp::socket* socket) {
    while (game_running) {
        asio::streambuf buffer;
        buffer.prepare(1024 * 10);
        asio::read_until(*socket, buffer, '\n');
        try {
            asio::read_until(*socket, buffer, '\n');
        }
        catch (const std::exception& e) {
            std::cout << "Disconnected from server: " << e.what() << std::endl;
            game_running = false;
            break;
        }

        std::istream is(&buffer);
        std::string line;
        std::vector<std::string> lines;

        while (getline(is, line)) {
            lines.push_back(line);
        }

        std::lock_guard<std::mutex> lock(game_mutex);
        GameState new_state;
        bool in_maze_section = true;

        for (const auto& current_line : lines) {
            if (current_line.empty()) continue;

            if (current_line == "---PLAYERS---") {
                in_maze_section = false;
                continue;
            }

            if (in_maze_section) {
                std::vector<char> row(current_line.begin(), current_line.end());
                new_state.maze.push_back(row);
            }
            else {
                if (current_line.rfind("Player ", 0) == 0) {
                    int id, x, y, lives, keys, gems;
                    if (sscanf(current_line.c_str(), "Player %d: x=%d, y=%d, lives=%d, keys=%d, gems=%d",
                        &id, &x, &y, &lives, &keys, &gems) == 6) {
                        new_state.players[id] = { x, y, lives, keys, gems };
                    }
                }
                else if (current_line.rfind("Message: ", 0) == 0) {
                    new_state.message = current_line.substr(9);
                }
                else if (current_line.rfind("Level: ", 0) == 0) {
                    new_state.level = stoi(current_line.substr(7));
                }
                else if (current_line.rfind("Game code: ", 0) == 0) {
                    new_state.codegame = current_line.substr(11);
                }
            }
        }

        game_state = new_state;
        print_game_state(game_state);

        if (!game_state.message.empty() &&
            (game_state.message.find("Game over!") != std::string::npos ||
                game_state.message.find("winner!") != std::string::npos)) {
            game_running = false;
        }
    }
}

