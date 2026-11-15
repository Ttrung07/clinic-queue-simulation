#include "ConsoleUI.h"
#include <exception>
#include <iostream>

int main() {
    try {
        // runConsole owns the store and simulation; construct store first (C contract).
        return clinic::runConsole(std::cin, std::cout);
    } catch (const std::exception& error) {
        std::cerr << "Loi: " << error.what() << '\n';
        return 1;
    }
}
