#include "ants.hpp"
#include <iostream>
#include <queue>
#include <set>
#include <fstream>
#include <sstream>

void Anthill::addRoom(std::string name, int capacity) {
    rooms[name] = Room{name, capacity};
}

void Anthill::addTunnel(std::string from, std::string to) {
    rooms[from].neighbors.push_back(to);
    rooms[to].neighbors.push_back(from);
}

void Anthill::printGraph() {
    std::cout << "====Graph of the anthill====\n";
    if (rooms.count("Sv")) {
        std::cout << "Sv ==> ";
        for (const auto& neighbor : rooms["Sv"].neighbors) {
            std::cout << neighbor << " ";
        }
        std::cout << "\n";
    }

    for (const auto& [name, room] : rooms) {
        if (name == "Sv") continue;
        std::cout << name << " ==> ";
        for (const auto& neighbor : room.neighbors) {
            std::cout << neighbor << " ";
        }
        std::cout << "\n";
    }
}

// Modification: added parameter 'avoid' to avoid some full rooms
std::vector<std::string> Anthill::bfs(const std::string& start, const std::string& end, const std::set<std::string>& avoid) {
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
            if (avoid.count(neighbor)) continue; // avoid rooms to be avoided
            auto newPath = path;
            newPath.push_back(neighbor);
            q.push(newPath);
        }
    }
    return {};
}

void Anthill::findPaths() {
    std::vector<std::string> path = bfs("Sv", "Sd", {});
    for (int i = 0; i < ants.size(); ++i) {
        ants[i].path = path;
        ants[i].currentRoom = "Sv";
        ants[i].pathIndex = 0;
    }
}

void Anthill::scheduleMovements() {
    bool allArrived = false;
    while (!allArrived) {
        std::vector<std::string> step;
        allArrived = true;

        // Reset occupancy of rooms except Sv and Sd
        for (auto& [name, room] : rooms) {
            room.occupancy = 0;
        }
        // Initial occupancy of ants in their rooms (except Sd)
        for (const auto& ant : ants) {
            if (ant.currentRoom != "Sd") {
                rooms[ant.currentRoom].occupancy++;
            }
        }

        for (auto& ant : ants) {
            if (ant.currentRoom != "Sd") {
                if (ant.pathIndex < (int)ant.path.size() - 1) {
                    std::string nextRoom = ant.path[ant.pathIndex + 1];
                    if (nextRoom == "Sd" || rooms[nextRoom].occupancy < rooms[nextRoom].capacity) {
                        // Movement possible to next room
                        step.push_back("    Ant " + std::to_string(ant.id) + " - " + ant.currentRoom + " to " + nextRoom);
                        // Update occupancy
                        rooms[ant.currentRoom].occupancy--;
                        ant.currentRoom = nextRoom;
                        ant.pathIndex++;
                        if (nextRoom != "Sd") rooms[nextRoom].occupancy++;
                    } else {
                        // Room full, look for alternative path
                        std::set<std::string> avoid = {nextRoom};
                        auto newPath = bfs(ant.currentRoom, "Sd", avoid);
                        if (!newPath.empty()) {
                            ant.path = newPath;
                            ant.pathIndex = 0;
                            // Try to move on the new path
                            if (ant.path.size() > 1) {
                                std::string newNextRoom = ant.path[1];
                                if (newNextRoom == "Sd" || rooms[newNextRoom].occupancy < rooms[newNextRoom].capacity) {
                                    step.push_back("    Ant " + std::to_string(ant.id) + " - " + ant.currentRoom + " to " + newNextRoom);
                                    rooms[ant.currentRoom].occupancy--;
                                    ant.currentRoom = newNextRoom;
                                    ant.pathIndex = 1;
                                    if (newNextRoom != "Sd") rooms[newNextRoom].occupancy++;
                                }
                            }
                        } else {
                            // No alternative path, ant blocked this step
                            allArrived = false;
                        }
                    }
                }
                if (ant.currentRoom != "Sd") allArrived = false;
            }
        }

        if (!step.empty()) steps.push_back(step);
    }
}

void Anthill::simulate(int numAnts) {
    ants.clear();
    steps.clear();
    for (int i = 0; i < numAnts; ++i) {
        ants.push_back(Ant{i + 1, "Sv"});
    }
    findPaths();
    scheduleMovements();

    for (size_t i = 0; i < steps.size(); ++i) {
        std::cout << "\n        === Step " << i + 1 << " ===\n";
        for (const auto& move : steps[i]) {
            std::cout << move << "\n";
        }
    }
}

bool Anthill::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: unable to open file " << path << std::endl;
        return false;
    }

    rooms.clear();
    ants.clear();
    steps.clear();

    std::string line;
    while (std::getline(file, line)) {
        // Cleaning
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (line.empty() || line[0] == '#') continue;
        if (line[0] == 'f') continue;

        // TUNNEL
        if (line.find('-') != std::string::npos) {
            std::istringstream iss(line);
            std::string from, dash, to;
            iss >> from >> dash >> to;
            if (!from.empty() && !to.empty()) {
                addTunnel(from, to);
            }
        }

        else {
            std::string roomName;
            int capacity = 1;

            size_t braceStart = line.find('{');
            size_t braceEnd = line.find('}');

            if (braceStart != std::string::npos && braceEnd != std::string::npos && braceEnd > braceStart) {
                roomName = line.substr(0, braceStart);
                roomName.erase(roomName.find_last_not_of(" \t") + 1);
                std::string capStr = line.substr(braceStart + 1, braceEnd - braceStart - 1);
                try {
                    capacity = std::stoi(capStr);
                } catch (...) {
                    std::cerr << "Capacity error in : " << line << std::endl;
                }
            } else {
                roomName = line;
            }

            roomName.erase(roomName.find_last_not_of(" \t\r\n") + 1);
            addRoom(roomName, capacity);
        }
    }

    return true;
}