#include "client.h"
#include <algorithm>
#include <asio.hpp>
#include <chrono>
#include <conio.h>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

/**
 * \file main.cpp
 * \brief Точка входа клиента для игры в лабиринт
 */

 /**
  * \brief Точка входа программы, инициализирует и впускает клиента
  * \return Код завершения программы
  */
int main() {
    try {
        bool restartGame = true;
        while (restartGame) {
            game_menu();

            asio::io_context io_context;
            asio::ip::tcp::resolver resolver(io_context);
            auto endpoints = resolver.resolve("127.0.0.1", "65434");
            asio::ip::tcp::socket* socket = new asio::ip::tcp::socket(io_context);

            try {
                asio::connect(*socket, endpoints);
                asio::streambuf id_buffer;
                asio::read_until(*socket, id_buffer, '\n');
                std::istream id_stream(&id_buffer);
                std::string id_response;
                std::getline(id_stream, id_response);

                if (id_response.find("PLAYER_ID ") == 0) {
                    int assigned_id = stoi(id_response.substr(10));
                    std::cout << "Server assigned you Player ID: " << assigned_id
                        << std::endl;
                }
                else {
                    std::cerr << "Unexpected server response: " << id_response
                        << std::endl;
                    return 1;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error connecting to server: " << e.what() << std::endl;
                return 1;
            }

            std::string init_data;
            if (game_state.codegame == "NEW") {
                init_data = std::to_string(game_state.level);
            }
            else {
                init_data = "JOIN " + game_state.codegame;
            }

            asio::write(*socket, asio::buffer(init_data + "\n"));

            std::cout << "Connected to server successfully. Waiting for game setup..."
                << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));

            std::thread recv_thread(receive_data, socket);
            game_running = true;

            while (game_running) {
                char press = _getch();
                std::string move;

                switch (tolower(press)) {
                case 'a':
                    move = "left";
                    break;
                case 'd':
                    move = "right";
                    break;
                case 'w':
                    move = "up";
                    break;
                case 's':
                    move = "down";
                    break;
                case 'q':
                    std::cout << "Returning to menu..." << std::endl;
                    game_running = false;
                    break;
                default:
                    continue;
                }

                if (!game_running)
                    break;

                try {
                    asio::write(*socket, asio::buffer(move + "\n"));
                }
                catch (const std::exception& e) {
                    std::cerr << "Error sending move: " << e.what() << std::endl;
                    break;
                }
            }

            recv_thread.join();
            socket->close();

            std::cout << "\nPlay again? (Y/N): ";
            char choice;
            std::cin >> choice;
            restartGame = (tolower(choice) == 'y');
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}