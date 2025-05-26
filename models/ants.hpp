#ifndef ANTS_HPP
#define ANTS_HPP

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <set>

struct Room {
    std::string name;
    int capacity;
    int occupancy = 0;
    std::vector<std::string> neighbors;
};

struct Ant {
    int id;
    std::vector<std::string> path;
    int pathIndex = 0;
    std::string currentRoom;
};

class Anthill {
public:
    void addRoom(const std::string& name, int capacity);
    void addTunnel(const std::string& from, const std::string& to);
    void simulate(int numAnts);
    void printGraph();

private:
    std::map<std::string, Room> rooms;
    std::vector<Ant> ants;
    std::vector<std::vector<std::string>> steps;

    std::map<std::string, std::map<std::string, int>> capacity;
    std::map<std::string, std::map<std::string, int>> flow;

    bool bfs(const std::string& s, const std::string& t, std::map<std::string, std::string>& parent);
    void edmondsKarp(const std::string& source, const std::string& sink);
    bool isInternalMovement(const std::string& from, const std::string& to);
    void buildPaths();
    void scheduleMovements();
};

#endif
