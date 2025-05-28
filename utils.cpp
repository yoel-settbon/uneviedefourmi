#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <regex>

void loadAnthillFromFile(const std::string& filename, Anthill& anthill, int& antCount) {
    std::ifstream file(filename);
    std::string line;
    std::regex roomRegex(R"(^S(\w+)(?:\s*\{\s*(\d+)\s*\})?$)");
    std::regex tunnelRegex(R"(^(\w+)\s*-\s*(\w+)$)");

    if (!file.is_open()) {
        std::cerr << "❌ Impossible d'ouvrir le fichier : " << filename << "\n";
        return;
    }

    while (getline(file, line)) {
        if (line.empty()) continue;
        if (line[0] == 'f' && line.find('=') != std::string::npos) {
            antCount = std::stoi(line.substr(line.find('=') + 1));
        } else if (line.find('-') != std::string::npos) {
            std::smatch match;
            if (std::regex_match(line, match, tunnelRegex)) {
                std::string from = match[1].str();
                std::string to = match[2].str();

                if (from[0] != 'S') from = "S" + from;
                if (to[0] != 'S') to = "S" + to;

                anthill.addTunnel(from, to);
            }
        } else if (line[0] == 'S') {
            std::smatch match;
            if (std::regex_match(line, match, roomRegex)) {
            std::string name = match[1].str();
            if (name[0] != 'S') name = "S" + name;
                int cap = match[2].matched ? std::stoi(match[2].str()) : 1;
                anthill.addRoom(name, cap);
            }
        }
    }
}
