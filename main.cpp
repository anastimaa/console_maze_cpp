#include "server.h"
#include <asio.hpp>
#include <iostream>
#include <thread>

/**
 * \file main.cpp
 * \brief Точка входа сервера для игры в лабиринт
 */

/**
 * \brief Точка входа программы, инициализирует и запускает игровой сервер
 * \return Код завершения программы
 */
int main() {
  try {
    asio::io_context io_context;
    run_server(io_context, 65434);
  } catch (const std::exception &e) {
    std::cerr << "Server error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}