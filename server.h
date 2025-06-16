#ifndef SERVER_H
#define SERVER_H

/**
 * \file server.h
 * \brief Заголовочный файл сервера для двухпользовательской игры в лабиринт
 */

#include <asio.hpp>
#include <map>
#include <mutex>
#include <string>
#include <vector>

constexpr char DOOR =
    static_cast<char>(176); ///< Символ, обозначающий закрытую дверь
const char GEM = '*';       ///< Символ, обозначающий алмаз
const char START = 'S';     ///< Символ, обозначающий стартовую позицию игроков
const char EXIT = 'E';      ///< Символ, обозначающий выход из лабиринта
const char MOB = 'M';       ///< Символ, обозначающий мобов
const char KEY = 'K';       ///< Символ, обозначающий ключ

/**
 * \brief Структура, описывающая игрока
 */
struct Player {
  int x;     ///< Координата x игрока в лабиринте
  int y;     ///< Координата y игрока в лабиринте
  int lives; ///< Количество жизней игрока
  int keys;  ///< Количество собранных ключей
  int gems;  ///< Количество собранных алмазов
};

/**
 * \brief Структура, описывающая моба
 */
struct Mob {
  int x, y; ///< Координаты моба в лабиринте
  int d;    ///< Направление движения
};

/**
 * \brief Структура, представляющая текущее состояние игры
 */
struct GameState {
  std::vector<std::vector<char>>
      maze; ///< Двумерный вектор, представляющий игровое поле лабиринта
  std::map<int, Player> players; ///< Список игроков по их ID
  std::vector<Mob> mobs;         ///< Список мобов
  int level;                     ///< Текущий уровень игры
  int codegame;                  ///< Уникальный код текущей игры
  std::string message;           ///< Сообщение для игроков
};

extern GameState game_state;  ///< Текущее состояние игры
extern std::mutex game_mutex; ///< Мьютекс для синхронизации доступа
extern std::map<int, std::shared_ptr<asio::ip::tcp::socket>>
    connections;           ///< Активные подключения игроков
extern bool game_running;  ///< Флаг, определяющий, активна ли игра
extern int next_player_id; ///< ID следующего подключающегося игрока

/**
 * \brief Генерирует лабиринт и размещает объекты в зависимости от уровня
 * \param[in] level Уровень сложности (1, 2 или 3)
 * \param[in] quiet_mode По умолчанию false, но если true, то подавляет вывод
 * сообщения в консоль
 * \throw std::invalid_argument В случае, если введен неверный уровень сложности
 * (не 1, 2 или 3)
 */
void generate_maze(int level, bool quiet_mode = false);

/**
 * \brief Сбрасывает текущее состояние игры к начальным значениям
 */
void reset_game_state();

/**
 * \brief Проверяет возможность хода в указанную клетку
 * \param[in] x Координата x
 * \param[in] y Координата y
 * \param[in] player_id ID игрока (1 или 2)
 * \return true, если игрок может сделать ход в указанную клетку, иначе false
 * \throw std::out_of_range В случае, если координаты x или y находятся вне
 * границ лабиринта
 * \throw std::invalid_argument В случае, если указанного игрока (player_id) нет
 * в текущем состоянии игры
 */
bool check_step(int x, int y, int player_id);

/**
 * \brief Обрабатывает ход игрока, обновляет состояние игры
 * \param[in] player_id ID игрока (1 или 2)
 * \param[in] move Направление хода ("up", "down", "left", "right")
 */
void process_player_move(int player_id, const std::string &move);

/**
 * \brief Обрабатывает подключение клиента: прием команд, отправка состояния
 * \param[in] socket Сокет подключенного клиента
 * \param[in] player_id ID игрока (1 или 2)
 */
void handle_client(std::shared_ptr<asio::ip::tcp::socket> socket,
                   int player_id);

/**
 * \brief Отправляет текущее состояние игры всем подключенным игрокам
 */
void broadcast_game_state();

/**
 * \brief Отправляет состояние игры одному клиенту
 * \param[in] socket Сокет клиента
 * \param[in] state Состояние игры для отправки
 */
void send_game_state(std::shared_ptr<asio::ip::tcp::socket> socket,
                     const GameState &state);

/**
 * \brief Запускает сервер для обработки подключений игроков и управления
 * игровыми сессиями
 * \param[in] io_context Контекст ввода-вывода ASIO для асинхронных операций
 * \param[in] port Порт, на котором сервер ожидает подключений
 */
void run_server(asio::io_context &io_context, unsigned short port);

#endif
