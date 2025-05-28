#include "models/ants.hpp"
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

namespace fs = std::filesystem;

// Extracts the number of ants from the first line of the file
int extractAntCount(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return 0;

    std::string line;
    if (std::getline(file, line)) {
        if (line.rfind("f=", 0) == 0) {
            try {
                return std::stoi(line.substr(2));
            } catch (...) {
                return 0;
            }
        }
    }
    return 0;
}

int main() {
    std::string folder = "anthill/";
    std::vector<fs::directory_entry> entries;

    std::cout << "==============================\n";
    std::cout << "        PICK AN ANTHILL       \n";
    std::cout << "==============================\n";

    // Collect and sort all .txt files in the folder
    for (const auto& entry : fs::directory_iterator(folder)) {
        if (entry.path().extension() == ".txt") {
            entries.push_back(entry);
        }
    }

    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        return a.path().filename() < b.path().filename();
    });

    if (entries.empty()) {
        std::cout << "No anthill found in the folder '" << folder << "'.\n";
        return 1;
    }

    for (size_t i = 0; i < entries.size(); ++i) {
        std::cout << " " << i + 1 << ". Anthill " << i << "\n";
    }

    std::cout << "\nEnter the number of the anthill to load: ";
    int choice;
    std::cin >> choice;

    if (choice < 1 || choice > static_cast<int>(entries.size())) {
        std::cout << "Invalid choice.\n";
        return 1;
    }

    std::string selectedFile = entries[choice - 1].path().string();
    int antCount = extractAntCount(selectedFile);

    if (antCount <= 0) {
        std::cout << "Invalid or missing number of ants in the file.\n";
        return 1;
    }

    Anthill anthill;

    if (!anthill.loadFromFile(selectedFile)) {
        std::cout << "Error loading the anthill from file.\n";
        return 1;
    }

    std::cout << "\n=== Solving Anthill " << (choice - 1) << " ===\n";
    anthill.printGraph();
    anthill.simulate(antCount);

    return 0;
}