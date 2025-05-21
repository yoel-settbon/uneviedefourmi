#include "ants.hpp"
#include <iostream>
#include <queue>
#include <set>

void Anthill::addRoom(std::string name, int capacity) {
    rooms[name] = Room{name, capacity};
}

void Anthill::addTunnel(std::string from, std::string to) {
    rooms[from].neighbors.push_back(to);
    rooms[to].neighbors.push_back(from);
}

void Anthill::printGraph() {
    std::cout << "====Graph of the anthill====\n";
    if (rooms.count("S_v")) {
        std::cout << "S_v ==> ";
        for (const auto& neighbor : rooms["S_v"].neighbors) {
            std::cout << neighbor << " ";
        }
        std::cout << "\n";
    }

    for (const auto& [name, room] : rooms) {
        if (name == "S_v") continue;
        std::cout << name << " ==> ";
        for (const auto& neighbor : room.neighbors) {
            std::cout << neighbor << " ";
        }
        std::cout << "\n";
    }
}

std::vector<std::string> Anthill::bfs(const std::string& start, const std::string& end) {
    std::queue<std::vector<std::string>> q;
    std::set<std::string> visited;
    q.push({start});

    while (!q.empty()) {
        auto path = q.front();
        q.pop();
        std::string current = path.back();

        if (current == end) return path;
        if (visited.count(current)) continue;
        visited.insert(current);

        for (const auto& neighbor : rooms[current].neighbors) {
            auto newPath = path;
            newPath.push_back(neighbor);
            q.push(newPath);
        }
    }
    return {};
}

void Anthill::findPaths() {
    std::vector<std::string> path = bfs("S_v", "S_d");
    for (int i = 0; i < ants.size(); ++i) {
        ants[i].path = path;
        ants[i].currentRoom = "S_v";
    }
}

void Anthill::scheduleMovements() {
    bool allArrived = false;
    while (!allArrived) {
        std::vector<std::string> step;
        allArrived = true;

        // libérer les salles à cette étape
        for (auto& [name, room] : rooms) {
            room.occupancy = 0;
        }

        for (auto& ant : ants) {
            if (ant.pathIndex < ant.path.size() - 1) {
                std::string nextRoom = ant.path[ant.pathIndex + 1];
                if (nextRoom == "S_d" || rooms[nextRoom].occupancy < rooms[nextRoom].capacity) {
                    step.push_back("    Ant " + std::to_string(ant.id) + " - " + ant.path[ant.pathIndex] + " to " + nextRoom);
                    ant.currentRoom = nextRoom;
                    ant.pathIndex++;
                    if (nextRoom != "S_d") rooms[nextRoom].occupancy++;
                } else {
                    allArrived = false;
                }
            }
        }

        if (!step.empty()) steps.push_back(step);

        for (const auto& ant : ants) {
            if (ant.currentRoom != "S_d") allArrived = false;
        }
    }
}

void Anthill::simulate(int numAnts) {
    ants.clear();
    steps.clear();
    for (int i = 0; i < numAnts; ++i) {
        ants.push_back(Ant{i + 1, "S_v"});
    }
    findPaths();
    scheduleMovements();

    for (size_t i = 0; i < steps.size(); ++i) {
        std::cout << "\n        === E " << i + 1 << " ===\n";
        for (const auto& move : steps[i]) {
            std::cout << move << "\n";
        }
    }
}
