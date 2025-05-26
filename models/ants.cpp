#include "ants.hpp"
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <limits>
#include <functional>

void Anthill::addRoom(const std::string& name, int capacityVal) {
    rooms[name] = Room{name, capacityVal};
    std::string in = name + "_in";
    std::string out = name + "_out";
    capacity[in][out] = capacityVal;

    rooms[in] = Room{in, 1000};
    rooms[out] = Room{out, 1000};
    capacity[out][in] = 0;
    flow[in][out] = 0;
    flow[out][in] = 0;

    rooms[name].neighbors.push_back(in); // for debugging
}

void Anthill::addTunnel(const std::string& from, const std::string& to) {
    std::string u = from + "_out";
    std::string v = to + "_in";
    capacity[u][v] = 1;
    flow[u][v] = 0;
    flow[v][u] = 0;
    rooms[u].neighbors.push_back(v);
    rooms[v].neighbors.push_back(u); // useful for residual graph
}

bool Anthill::bfs(const std::string& s, const std::string& t, std::map<std::string, std::string>& parent) {
    std::queue<std::string> q;
    std::set<std::string> visited;
    q.push(s);
    visited.insert(s);
    parent.clear();

    while (!q.empty()) {
        std::string u = q.front(); q.pop();

        for (const auto& [v, cap] : capacity[u]) {
            if (!visited.count(v) && cap - flow[u][v] > 0) {
                parent[v] = u;
                visited.insert(v);
                q.push(v);
                if (v == t) return true;
            }
        }
    }
    return false;
}

void Anthill::edmondsKarp(const std::string& source, const std::string& sink) {
    std::map<std::string, std::string> parent;

    while (bfs(source, sink, parent)) {
        int pathFlow = INT_MAX;

        std::string v = sink;
        while (v != source) {
            std::string u = parent[v];
            pathFlow = std::min(pathFlow, capacity[u][v] - flow[u][v]);
            v = u;
        }

        v = sink;
        while (v != source) {
            std::string u = parent[v];
            flow[u][v] += pathFlow;
            flow[v][u] -= pathFlow;
            v = u;
        }
    }
}

std::vector<std::vector<std::string>> Anthill::findAllPaths(const std::string& start, const std::string& end, int maxPaths) {
    std::vector<std::vector<std::string>> allPaths;
    
    // Utiliser une approche DFS avec backtracking pour trouver plusieurs chemins
    std::function<void(std::string, std::vector<std::string>&, std::set<std::string>&)> dfs;
    
    dfs = [&](std::string current, std::vector<std::string>& path, std::set<std::string>& visited) {
        if (current == end) {
            allPaths.push_back(path);
            return;
        }
        
        if (allPaths.size() >= maxPaths) return; // Limite pour éviter l'explosion combinatoire
        
        // Explorer tous les voisins possibles
        if (capacity.count(current)) {
            std::vector<std::string> neighbors;
            for (const auto& [neighbor, cap] : capacity[current]) {
                if (cap > 0 && !visited.count(neighbor)) {
                    neighbors.push_back(neighbor);
                }
            }
            
            // Trier les voisins par nom pour un ordre déterministe mais varié
            std::sort(neighbors.begin(), neighbors.end());
            
            for (const std::string& neighbor : neighbors) {
                visited.insert(neighbor);
                path.push_back(neighbor);
                dfs(neighbor, path, visited);
                path.pop_back();
                visited.erase(neighbor);
            }
        }
    };
    
    std::vector<std::string> path = {start};
    std::set<std::string> visited = {start};
    dfs(start, path, visited);
    
    // Trier les chemins par longueur
    std::sort(allPaths.begin(), allPaths.end(), [this](const auto& a, const auto& b) {
        return calculateRealPathLength(a) < calculateRealPathLength(b);
    });
    
    return allPaths;
}

void Anthill::buildPaths() {
    // Trouver plusieurs chemins différents (pas seulement ceux du flux)
    availablePaths = findAllPaths("S_v_out", "S_d_in", 10); // Limiter à 10 chemins max
    
    if (availablePaths.empty()) {
        std::cout << "ERREUR: Aucun chemin trouvé!\n";
        return;
    }

    std::cout << "\n=== CHEMINS DISPONIBLES ===\n";
    for (size_t i = 0; i < availablePaths.size(); ++i) {
        int cost = calculateRealPathLength(availablePaths[i]);
        std::cout << "Chemin " << (i + 1) << " (coût: " << cost << "): ";
        for (const auto& room : availablePaths[i]) {
            std::cout << room << " ";
        }
        std::cout << "\n";
    }
}

int Anthill::calculateRealPathLength(const std::vector<std::string>& path) {
    int realSteps = 0;
    for (size_t i = 0; i < path.size() - 1; ++i) {
        if (!isInternalMovement(path[i], path[i + 1])) {
            realSteps++;
        }
    }
    return realSteps;
}

std::vector<std::string> Anthill::getBestAvailablePath(const std::string& currentPosition,
                                                      const std::unordered_map<std::string, int>& currentOccupancy,
                                                      int antId) {
    std::vector<std::pair<int, std::vector<std::string>>> pathOptions;
    
    // Évaluer tous les chemins disponibles depuis la position actuelle
    for (const auto& fullPath : availablePaths) {
        // Trouver où cette fourmi se trouve dans ce chemin
        auto it = std::find(fullPath.begin(), fullPath.end(), currentPosition);
        if (it != fullPath.end()) {
            // Extraire le sous-chemin depuis la position actuelle
            std::vector<std::string> subPath(it, fullPath.end());
            
            // Calculer un score basé sur la longueur et la congestion
            int pathScore = calculatePathScore(subPath, currentOccupancy, antId);
            pathOptions.push_back({pathScore, subPath});
        }
    }
    
    // Si aucun chemin existant ne convient, faire un BFS simple
    if (pathOptions.empty()) {
        return findShortestPath(currentPosition, "S_d_in", currentOccupancy);
    }
    
    // Trier par score (plus petit = meilleur)
    std::sort(pathOptions.begin(), pathOptions.end());
    
    return pathOptions[0].second;
}

int Anthill::calculatePathScore(const std::vector<std::string>& path, 
                               const std::unordered_map<std::string, int>& currentOccupancy, 
                               int antId) {
    int score = calculateRealPathLength(path) * 10; // Base: longueur du chemin
    
    // Ajouter une pénalité pour la congestion
    for (const auto& room : path) {
        std::string baseRoom = room.substr(0, room.find_last_of('_'));
        if (currentOccupancy.count(baseRoom) && rooms.count(baseRoom)) {
            int occupancy = currentOccupancy.at(baseRoom);
            int capacity = rooms.at(baseRoom).capacity;
            
            if (occupancy >= capacity) {
                score += 100; // Forte pénalité pour les salles pleines
            } else {
                score += occupancy * 5; // Pénalité proportionnelle à l'occupation
            }
        }
    }
    
    // Ajouter une petite variation basée sur l'ID de la fourmi pour diversifier
    score += (antId % 3); // Variation de 0-2 pour éviter que toutes les fourmis fassent le même choix
    
    return score;
}

std::vector<std::string> Anthill::findShortestPath(const std::string& start, const std::string& end,
                                                  const std::unordered_map<std::string, int>& currentOccupancy) {
    std::queue<std::string> q;
    std::map<std::string, std::string> parent;
    std::set<std::string> visited;
    
    q.push(start);
    visited.insert(start);
    parent[start] = "";

    while (!q.empty()) {
        std::string current = q.front();
        q.pop();

        if (current == end) {
            std::vector<std::string> path;
            std::string node = end;
            while (!node.empty()) {
                path.push_back(node);
                node = parent[node];
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        if (capacity.count(current)) {
            for (const auto& [neighbor, cap] : capacity[current]) {
                if (cap > 0 && !visited.count(neighbor)) {
                    std::string baseNeighbor = neighbor.substr(0, neighbor.find_last_of('_'));
                    bool canUse = true;
                    
                    if (neighbor.find("_in") == std::string::npos && 
                        neighbor.find("_out") == std::string::npos &&
                        baseNeighbor != "S_d") {
                        auto it = currentOccupancy.find(baseNeighbor);
                        if (it != currentOccupancy.end() && rooms.count(baseNeighbor)) {
                            canUse = it->second < rooms.at(baseNeighbor).capacity;
                        }
                    }
                    
                    if (canUse) {
                        visited.insert(neighbor);
                        parent[neighbor] = current;
                        q.push(neighbor);
                    }
                }
            }
        }
    }
    
    return {};
}

bool Anthill::isInternalMovement(const std::string& from, const std::string& to) {
    std::string baseFrom = from.substr(0, from.find_last_of('_'));
    std::string baseTo = to.substr(0, to.find_last_of('_'));
    return baseFrom == baseTo && from != to;
}

void Anthill::scheduleMovements() {
    bool allArrived = false;
    int stepNumber = 1;

    // Initialiser l'occupancy des salles au début
    for (auto& [name, room] : rooms) {
        room.occupancy = 0;
    }

    if (rooms.count("S_v")) {
        rooms["S_v"].occupancy = ants.size();
    }

    // Initialiser toutes les fourmis au départ
    for (auto& ant : ants) {
        ant.currentRoom = "S_v_out";
        ant.pathIndex = 0;
        ant.path.clear();
    }

    // Assignment initial diversifié
    std::cout << "\n=== ASSIGNMENT INITIAL DES FOURMIS ===\n";
    for (size_t i = 0; i < ants.size(); ++i) {
        // Assigner chaque fourmi à un chemin différent (round-robin)
        int pathIndex = i % availablePaths.size();
        ants[i].preferredPathIndex = pathIndex;
        std::cout << "Fourmi " << ants[i].id << " -> Chemin préféré " << (pathIndex + 1) << "\n";
    }

    while (!allArrived) {
        std::vector<std::string> step;
        allArrived = true;
        
        std::unordered_map<std::string, int> nextOccupancy;
        for (const auto& [name, room] : rooms) {
            if (name.find("_in") == std::string::npos && name.find("_out") == std::string::npos) {
                nextOccupancy[name] = room.occupancy;
            }
        }

        bool someMovement = false;

        // Trier les fourmis par ID pour un ordre déterministe
        std::vector<Ant*> sortedAnts;
        for (auto& ant : ants) {
            sortedAnts.push_back(&ant);
        }
        std::sort(sortedAnts.begin(), sortedAnts.end(), 
                 [](const Ant* a, const Ant* b) { return a->id < b->id; });

        for (auto* ant : sortedAnts) {
            if (ant->currentRoom != "S_d_in") {
                allArrived = false;
                
                // Utiliser le chemin préféré ou chercher une alternative
                std::vector<std::string> bestPath = getBestAvailablePath(ant->currentRoom, nextOccupancy, ant->id);
                
                if (bestPath.size() >= 2) {
                    std::string from = bestPath[0];
                    std::string to = bestPath[1];
                    
                    std::string baseFrom = from.substr(0, from.find_last_of('_'));
                    std::string baseTo = to.substr(0, to.find_last_of('_'));

                    bool isInternal = isInternalMovement(from, to);
                    bool isSink = baseTo == "S_d";
                    bool isSource = baseFrom == "S_v";
                    
                    bool canMove = false;
                    
                    if (isInternal) {
                        canMove = true;
                    } else if (isSink) {
                        canMove = true;
                    } else if (isSource) {
                        canMove = nextOccupancy[baseTo] < rooms[baseTo].capacity;
                    } else {
                        if (to.find("_in") != std::string::npos || to.find("_out") != std::string::npos) {
                            canMove = true;
                        } else {
                            canMove = nextOccupancy[baseTo] < rooms[baseTo].capacity;
                        }
                    }

                    if (canMove) {
                        someMovement = true;
                        ant->currentRoom = to;

                        if (isSource && !isInternal) {
                            nextOccupancy["S_v"]--;
                            nextOccupancy[baseTo]++;
                            step.push_back("    Fourmi " + std::to_string(ant->id) + " - S_v vers " + baseTo);
                        } else if (!isInternal && !isSink && 
                                   from.find("_out") != std::string::npos && to.find("_in") != std::string::npos) {
                            if (baseFrom != baseTo) {
                                nextOccupancy[baseFrom]--;
                                nextOccupancy[baseTo]++;
                                step.push_back("    Fourmi " + std::to_string(ant->id) + " - " + baseFrom + " vers " + baseTo);
                            }
                        } else if (isSink) {
                            if (from.find("_out") != std::string::npos) {
                                nextOccupancy[baseFrom]--;
                            }
                            nextOccupancy["S_d"]++;
                            step.push_back("    Fourmi " + std::to_string(ant->id) + " - " + baseFrom + " vers S_d (ARRIVÉE!)");
                        }
                    }
                }
            }
        }

        for (const auto& [roomName, count] : nextOccupancy) {
            if (rooms.count(roomName)) {
                rooms[roomName].occupancy = count;
            }
        }

        if (someMovement) {
            std::cout << "\n        === ÉTAPE " << stepNumber << " ===\n";
            if (step.empty()) {
                std::cout << "    (Mouvements internes seulement)\n";
            } else {
                for (const auto& move : step) {
                    std::cout << move << "\n";
                }
            }
            stepNumber++;
            
            std::cout << "    État des salles: ";
            for (const auto& [name, room] : rooms) {
                if (name.find("_in") == std::string::npos && name.find("_out") == std::string::npos) {
                    std::cout << name << "(" << room.occupancy << "/" << room.capacity << ") ";
                }
            }
            std::cout << "\n";
        } else if (!allArrived) {
            std::cout << "\n    BLOCAGE DÉTECTÉ - Aucun mouvement possible\n";
            break;
        }
    }

    if (allArrived) {
        std::cout << "\n=== SIMULATION TERMINÉE - Toutes les fourmis sont arrivées en " << (stepNumber - 1) << " étapes ===\n";
    }
}

void Anthill::simulate(int numAnts) {
    ants.clear();
    steps.clear();
    for (int i = 0; i < numAnts; ++i)
        ants.push_back(Ant{i + 1});

    std::cout << "=== DÉMARRAGE SIMULATION AVEC DIVERSIFICATION FORCÉE (" << numAnts << " fourmis) ===\n";
    
    edmondsKarp("S_v_out", "S_d_in");
    buildPaths();
    scheduleMovements();
}