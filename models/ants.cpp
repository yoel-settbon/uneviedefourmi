#include "ants.hpp"
#include <iostream>
#include <unordered_map>
#include <algorithm>

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
    capacity[u][v] = 1000; // CORRECTION: Capacité très élevée pour les tunnels
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

void Anthill::buildPaths() {
    std::vector<std::vector<std::string>> allPaths;
    auto tempFlow = flow;

    // Extraire TOUS les chemins du flux maximum
    while (true) {
        std::vector<std::string> path;
        std::string u = "S_v_out";
        std::string t = "S_d_in";
        std::set<std::string> visited;
        path.push_back(u);

        while (u != t) {
            visited.insert(u);
            bool advanced = false;
            for (const auto& [v, f] : tempFlow[u]) {
                if (f > 0 && !visited.count(v)) {
                    path.push_back(v);
                    u = v;
                    advanced = true;
                    break;
                }
            }
            if (!advanced) break;
        }

        if (path.back() == "S_d_in") {
            for (size_t i = 0; i < path.size() - 1; ++i)
                tempFlow[path[i]][path[i + 1]]--;
            allPaths.push_back(path);
        } else {
            break;
        }
    }

    std::cout << "\n=== CHEMINS DISPONIBLES ===\n";
    for (size_t i = 0; i < allPaths.size(); ++i) {
        int cost = calculateRealPathLength(allPaths[i]);
        std::cout << "Chemin " << (i + 1) << " (coût: " << cost << "): ";
        for (const auto& room : allPaths[i]) {
            std::cout << room << " ";
        }
        std::cout << "\n";
    }

    availablePaths = allPaths;
    
    // Initialiser toutes les fourmis à S_v_out
    for (auto& ant : ants) {
        ant.currentRoom = "S_v_out";
        ant.pathIndex = 0;
        ant.path.clear();
    }
    
    std::cout << "\n=== INITIALISATION: " << ants.size() << " fourmis démarrent en S_v_out ===\n";
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

std::vector<std::string> Anthill::findBestNextMove(const std::string& currentPos, 
                                                  const std::unordered_map<std::string, int>& occupancy) {
    if (availablePaths.empty()) {
        std::cout << "ERREUR: Aucun chemin disponible!\n";
        return {};
    }

    std::vector<std::pair<int, std::vector<std::string>>> options;
    
    for (const auto& path : availablePaths) {
        auto it = std::find(path.begin(), path.end(), currentPos);
        if (it != path.end() && it + 1 != path.end()) {
            std::vector<std::string> move = {*it, *(it + 1)};
            std::vector<std::string> remainingPath(it, path.end());
            
            int score = calculateRealPathLength(remainingPath) * 10;
            
            // Vérifier la destination du mouvement
            std::string nextRoom = *(it + 1);
            std::string baseNext = nextRoom.substr(0, nextRoom.find_last_of('_'));
            
            // Pénaliser selon l'occupation de la salle de destination
            if (occupancy.count(baseNext) && rooms.count(baseNext)) {
                int occ = occupancy.at(baseNext);
                int cap = rooms.at(baseNext).capacity;
                if (occ >= cap) {
                    score += 10000; // Interdire si plein
                } else {
                    score += occ * 3; // Pénalité légère pour congestion
                }
            }
            
            options.push_back({score, move});
        }
    }
    
    if (options.empty()) {
        std::cout << "ATTENTION: Aucun mouvement possible depuis " << currentPos << "\n";
        return {};
    }
    
    std::sort(options.begin(), options.end());
    return options[0].second;
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

    std::cout << "\n=== DÉMARRAGE SIMULATION ===\n";
    std::cout << "État initial: " << ants.size() << " fourmis en S_v\n";

    while (!allArrived) {
        std::vector<std::string> step;
        allArrived = true;
        
        // Copie de l'état actuel pour calculer les changements
        std::unordered_map<std::string, int> nextOccupancy;
        for (const auto& [name, room] : rooms) {
            if (name.find("_in") == std::string::npos && name.find("_out") == std::string::npos) {
                nextOccupancy[name] = room.occupancy;
            }
        }

        bool someMovement = false;

        // Trier les fourmis par ID pour déterminisme
        std::vector<Ant*> sortedAnts;
        for (auto& ant : ants) {
            sortedAnts.push_back(&ant);
        }
        std::sort(sortedAnts.begin(), sortedAnts.end(), 
                 [](const Ant* a, const Ant* b) { return a->id < b->id; });

        for (auto* ant : sortedAnts) {
            if (ant->currentRoom != "S_d_in") {
                allArrived = false;
                
                std::vector<std::string> bestMove = findBestNextMove(ant->currentRoom, nextOccupancy);
                
                if (bestMove.size() == 2) {
                    std::string from = bestMove[0];
                    std::string to = bestMove[1];
                    
                    std::string baseFrom = from.substr(0, from.find_last_of('_'));
                    std::string baseTo = to.substr(0, to.find_last_of('_'));
                    
                    bool isInternal = isInternalMovement(from, to);
                    bool isSink = baseTo == "S_d";
                    bool isSource = baseFrom == "S_v";
                    
                    bool canMove = false;
                    
                    if (isInternal) {
                        // Mouvements internes toujours autorisés
                        canMove = true;
                    } else if (isSink) {
                        // Mouvement vers la destination finale toujours autorisé
                        canMove = true;
                    } else if (isSource) {
                        // Mouvement depuis la source - vérifier la capacité de destination
                        canMove = nextOccupancy[baseTo] < rooms[baseTo].capacity;
                    } else {
                        // Mouvement entre salles normales
                        if (to.find("_in") != std::string::npos || to.find("_out") != std::string::npos) {
                            canMove = true;
                        } else {
                            canMove = nextOccupancy[baseTo] < rooms[baseTo].capacity;
                        }
                    }

                    if (canMove) {
                        someMovement = true;
                        ant->currentRoom = to;

                        // Gérer les mouvements entre salles réelles
                        if (isSource && !isInternal) {
                            // Fourmi quitte S_v pour aller vers une vraie salle
                            nextOccupancy["S_v"]--;
                            nextOccupancy[baseTo]++;
                            step.push_back("    Fourmi " + std::to_string(ant->id) + " - S_v vers " + baseTo);
                        } else if (!isInternal && !isSink && 
                                   from.find("_out") != std::string::npos && to.find("_in") != std::string::npos) {
                            // Fourmi passe d'une salle_out vers une autre salle_in (mouvement entre salles)
                            if (baseFrom != baseTo) {
                                nextOccupancy[baseFrom]--;
                                nextOccupancy[baseTo]++;
                                step.push_back("    Fourmi " + std::to_string(ant->id) + " - " + baseFrom + " vers " + baseTo);
                            }
                        } else if (isSink) {
                            // Fourmi arrive à destination
                            if (from.find("_out") != std::string::npos) {
                                nextOccupancy[baseFrom]--;
                            }
                            nextOccupancy["S_d"]++;
                            step.push_back("    Fourmi " + std::to_string(ant->id) + " - " + baseFrom + " vers S_d (ARRIVEE!)");
                        }
                    }
                }
            }
        }

        // Appliquer les changements d'occupancy
        for (const auto& [roomName, count] : nextOccupancy) {
            if (rooms.count(roomName)) {
                rooms[roomName].occupancy = count;
            }
        }

        // Vérification de sécurité
        bool capacityViolation = false;
        for (const auto& [name, room] : rooms) {
            if (name.find("_in") == std::string::npos && name.find("_out") == std::string::npos) {
                if (room.occupancy > room.capacity) {
                    std::cout << "⚠️  ERREUR: " << name << " a " << room.occupancy 
                              << " fourmis mais capacité = " << room.capacity << "\n";
                    capacityViolation = true;
                }
                if (room.occupancy < 0) {
                    std::cout << "⚠️  ERREUR: " << name << " a une occupation négative: " << room.occupancy << "\n";
                    capacityViolation = true;
                }
            }
        }

        if (capacityViolation) {
            std::cout << "❌ SIMULATION ARRÊTÉE - Violation détectée\n";
            break;
        }

        if (someMovement) {
            // MODIFICATION: N'afficher que les étapes avec de vrais mouvements
            if (!step.empty()) {
                std::cout << "\n        === ETAPE " << stepNumber << " ===\n";
                for (const auto& move : step) {
                    std::cout << move << "\n";
                }
                
                std::cout << "    Etat des salles: ";
                for (const auto& [name, room] : rooms) {
                    if (name.find("_in") == std::string::npos && name.find("_out") == std::string::npos) {
                        std::cout << name << "(" << room.occupancy << "/" << room.capacity << ") ";
                    }
                }
                std::cout << "\n";
            }
            stepNumber++;
        } else if (!allArrived) {
            std::cout << "\n    BLOCAGE DETECTE - Aucun mouvement possible\n";
            break;
        }
    }

    if (allArrived) {
        std::cout << "\n=== ✅ TOUTES LES FOURMIS ARRIVEES EN " << (stepNumber - 1) << " ETAPES ===\n";
    }
}

void Anthill::simulate(int numAnts) {
    ants.clear();
    steps.clear();
    for (int i = 0; i < numAnts; ++i)
        ants.push_back(Ant{i + 1});

    std::cout << "=== SIMULATION CORRIGEE (" << numAnts << " fourmis) ===\n";
    
    edmondsKarp("S_v_out", "S_d_in");
    buildPaths();
    scheduleMovements();
}