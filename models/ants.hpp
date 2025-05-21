#ifndef ANTS_HPP
#define ANTS_HPP

#include <string>
#include <vector>
#include <map>

struct Room {
    std::string name;
    int capacity;
    std::vector<std::string> neighbors;
    int occupancy = 0;
};

struct Ant {
    int id;
    std::string currentRoom;
    std::vector<std::string> path;
    int pathIndex = 0;
};

class Anthill {
public:
    void addRoom(std::string name, int capacity = 1);
    void addTunnel(std::string from, std::string to);
    void printGraph();
    void simulate(int numAnts);

private:
    std::map<std::string, Room> rooms;
    std::vector<Ant> ants;
    std::vector<std::vector<std::string>> steps;

    std::vector<std::string> bfs(const std::string& start, const std::string& end);
    void findPaths();
    void scheduleMovements();
};

#endif