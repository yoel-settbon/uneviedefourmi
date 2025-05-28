#include "models/ants.hpp"
#include "utils.hpp"
#include <iostream>
#include <vector>

int main() {
    std::vector<std::string> files = {
        "anthill_0.txt", "anthill_1.txt", "anthill_2.txt",
        "anthill_3.txt", "anthill_4.txt", "anthill_5.txt"
    };

    std::cout << "\n=== CHOISISSEZ UNE FOURMILIÈRE ===\n";
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << i + 1 << ". " << files[i] << "\n";
    }

    int choice = 0;
    std::cout << "\nEntrez le numéro de la fourmilière : ";
    std::cin >> choice;

    if (choice < 1 || choice > files.size()) {
        std::cerr << "❌ Choix invalide.\n";
        return 1;
    }

    Anthill anthill;
    int antCount = 0;
    loadAnthillFromFile(files[choice - 1], anthill, antCount);
    anthill.simulate(antCount);

    return 0;
}
