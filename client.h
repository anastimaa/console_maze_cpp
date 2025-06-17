/**
 * \file client.h
 * \brief Заголовочный файл клиента для двухпользовательской игры в лабиринт
 */

#ifndef CLIENT_H
#define CLIENT_H

#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <winsock2.h>
#include <asio.hpp>

 /**
  * \brief Структура для представления мобов в игре
  */
struct Mob {
    int x; ///< Координата X моба
    int y; ///< Координата Y моба
};

/**
 * \brief Структура для хранения состояния игры на клиенте
 */
struct GameState {
    /**
     * \brief Структура игрока
     */
    struct Player {
        int x;      ///< Координата X игрока
        int y;      ///< Координата Y игрока
        int lives;  ///< Количество жизней игрока
        int keys;   ///< Количество ключей
        int gems;   ///< Количество драгоценностей
    };

    std::vector<std::vector<char>> maze;  ///< Двумерный массив лабиринта
    std::map<int, Player> players;        ///< Игроки (ID -> Player)
    std::vector<Mob> mobs;                ///< Список мобов
    int level;                            ///< Уровень сложности
    std::string codegame;                 ///< Код игры
    std::string message;                  ///< Сообщение от сервера
};

// Глобальные переменные
extern GameState game_state;      ///< Текущее состояние игры
extern std::mutex game_mutex;     ///< Мьютекс для синхронизации доступа к game_state
extern bool game_running;         ///< Флаг работы игрового цикла

/**
 * \brief Получает символ без ожидания Enter
 * \return Символ, введённый пользователем
 */
char get_char();

/**
 * \brief Очищает экран консоли
 */
void clear_screen();

/**
 * \brief Отображает меню игры и обрабатывает выбор пользователя
 */
void game_menu();

/**
 * \brief Выводит текущее состояние игры
 * \param[in] state Состояние игры для отображения
 */
void print_game_state(const GameState& state);

/**
 * \brief Получает данные от сервера (запускается в отдельном потоке)
 * \param[in] socket Указатель на сокет для обмена данными с сервером
 */
void receive_data(std::shared_ptr<asio::ip::tcp::socket> socket);

#endif
