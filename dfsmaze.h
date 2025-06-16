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
 * \param[in] width Ширина лабиринта
 * \param[in] height Высота лабиринта
 * \return Двумерный вектор символов, представляющий лабиринт (WALL - стены,
 * EMPTY - проходы)
 * \throw std::invalid_argument В случае, если размеры лабиринта меньше 3×3
 */
std::vector<std::vector<char>> dfsmaze_generate(int width, int height);

#endif