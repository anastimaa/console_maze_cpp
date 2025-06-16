#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "client.h"
#include "doctest.h"
#include <sstream>
#include <stdexcept>

TEST_CASE("game_menu invalid choice") {
    std::istringstream input("Q\n");
    std::cin.rdbuf(input.rdbuf());

    CHECK_THROWS_AS(game_menu(), std::invalid_argument);
}

TEST_CASE("game_menu create new game valid level") {
    SUBCASE("level 1") {
        std::istringstream input("N\n1\n");
        std::cin.rdbuf(input.rdbuf());

        game_menu();
        CHECK(game_state.level == 1);
        CHECK(game_state.codegame == "NEW");
    }

    SUBCASE("level 2") {
        std::istringstream input("N\n2\n");
        std::cin.rdbuf(input.rdbuf());

        game_menu();
        CHECK(game_state.level == 2);
        CHECK(game_state.codegame == "NEW");
    }

    SUBCASE("level 3") {
        std::istringstream input("N\n3\n");
        std::cin.rdbuf(input.rdbuf());

        game_menu();
        CHECK(game_state.level == 3);
        CHECK(game_state.codegame == "NEW");
    }
}

TEST_CASE("game_menu create new game invalid level") {
    std::istringstream input("N\n4\n");
    std::cin.rdbuf(input.rdbuf());

    CHECK_THROWS_AS(game_menu(), std::invalid_argument);
}

TEST_CASE("game_menu join game valid code") {
    std::istringstream input("J\n12345\n");
    std::cin.rdbuf(input.rdbuf());

    game_menu();
    CHECK(game_state.codegame == "12345");
}

TEST_CASE("game_menu join game invalid code (non-digit)") {
    std::istringstream input("J\nABC12\n");
    std::cin.rdbuf(input.rdbuf());

    CHECK_THROWS_AS(game_menu(), std::invalid_argument);
}

TEST_CASE("game_menu input error") {
    std::istringstream input;
    input.setstate(std::ios::failbit);
    std::cin.rdbuf(input.rdbuf());

    CHECK_THROWS_AS(game_menu(), std::runtime_error);
}