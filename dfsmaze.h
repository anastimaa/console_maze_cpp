#ifndef DFSMAZE_H
#define DFSMAZE_H

/**
 * \file dfsmaze.h
 * \brief Заголовочный файл для генерации лабиринта с использованием алгоритма
 * DFS
 */

#include <vector>

constexpr char WALL =
static_cast<char>(219); ///< Символ, обозначающий стену в лабиринте
constexpr char EMPTY =
' '; ///< Символ, обозначающий пустое пространство (проход)

/**
 * \brief Генерирует лабиринт заданного размера с помощью алгоритма поиска в
 * глубину (DFS)
 *
 * \param[in] width Ширина лабиринта (минимум 3)
 * \param[in] height Высота лабиринта (минимум 3)
 *
 * \return Двумерный вектор символов, представляющий лабиринт (WALL - стены,
 * EMPTY - проходы)
 *
 * \throw std::invalid_argument В случае, если размеры лабиринта меньше 3×3
 */
std::vector<std::vector<char>> dfsmaze_generate(int width, int height);

/**
 * \brief Рекурсивная реализация алгоритма DFS для генерации лабиринта.
 * Заимствовано.
 *
 * \param[in,out] maze Двумерный вектор лабиринта (изменяется в процессе
 * генерации)
 * \param[in] height Высота лабиринта
 * \param[in] width Ширина лабиринта
 * \param[in] start_x Начальная координата X
 * \param[in] start_y Начальная координата Y
 *
 * \throw std::out_of_range В случае, если начальные координаты выходят за
 * границы лабиринта
 * \throw std::invalid_argument В случае, если стартовая позиция не является
 * пустой клеткой
 *
 * \note Функция используется внутренне в dfsmaze_generate()
 */

void dfs(std::vector<std::vector<char>>& maze, int height, int width,
    int start_x, int start_y);

#endif
