#include "ants.hpp"
#include <iostream>
#include <unordered_map>

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

void Anthill::buildPaths() {
    std::vector<std::vector<std::string>> allPaths;

    while (true) {
        std::vector<std::string> path;
        std::string u = "S_v_out";
        std::string t = "S_d_in";
        std::set<std::string> visited;
        path.push_back(u);

        while (u != t) {
            visited.insert(u);
            bool advanced = false;
            for (const auto& [v, f] : flow[u]) {
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
                flow[path[i]][path[i + 1]]--;
            allPaths.push_back(path);
        } else {
            break;
        }
    }

    for (int i = 0; i < ants.size(); ++i) {
        if (i < (int)allPaths.size()) {
            ants[i].path = allPaths[i];
        } else {
            ants[i].path = allPaths[i % allPaths.size()];
        }
        ants[i].currentRoom = ants[i].path[0];
    }
    std::cout << "\n=== CHEMINS DES FOURMIS ===\n";
    for (const auto& ant : ants) {
        std::cout << "Ant " << ant.id << " path: ";
        for (const auto& room : ant.path)
            std::cout << room << " ";
        std::cout << "\n";
    }
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

    // Les fourmis commencent dans S_v
    if (rooms.count("S_v")) {
        rooms["S_v"].occupancy = ants.size();
    }

    while (!allArrived) {
        std::vector<std::string> step;
        allArrived = true;
        
        // Créer une copie de l'occupancy actuelle pour calculer les changements
        std::unordered_map<std::string, int> nextOccupancy;
        for (const auto& [name, room] : rooms) {
            if (name.find("_in") == std::string::npos && name.find("_out") == std::string::npos) {
                nextOccupancy[name] = room.occupancy;
            }
        }

        bool someMovement = false;

        // Traiter chaque fourmi
        for (auto& ant : ants) {
            if (ant.pathIndex < (int)ant.path.size() - 1) {
                allArrived = false;
                
                std::string from = ant.path[ant.pathIndex];
                std::string to = ant.path[ant.pathIndex + 1];
                
                // Extraire les noms de base des salles
                std::string baseFrom = from.substr(0, from.find_last_of('_'));
                std::string baseTo = to.substr(0, to.find_last_of('_'));

                bool isInternal = isInternalMovement(from, to);
                bool isSink = baseTo == "S_d";
                bool isSource = baseFrom == "S_v";
                
                // Logique de mouvement améliorée
                bool canMove = false;
                
                if (isInternal) {
                    // Les mouvements internes sont toujours autorisés
                    canMove = true;
                } else if (isSink) {
                    // Mouvement vers la destination finale
                    canMove = true;
                } else if (isSource) {
                    // Mouvement depuis la source - vérifier la capacité de destination
                    canMove = nextOccupancy[baseTo] < rooms[baseTo].capacity;
                } else {
                    // Mouvement entre salles normales
                    // Une fourmi peut toujours sortir de sa salle actuelle si elle peut avancer
                    // Même si la prochaine salle "réelle" est pleine, elle peut passer par les salles _out/_in
                    
                    // Si on va vers une salle _in ou _out, on peut toujours bouger
                    if (to.find("_in") != std::string::npos || to.find("_out") != std::string::npos) {
                        canMove = true;
                    } else {
                        // Sinon vérifier la capacité
                        canMove = nextOccupancy[baseTo] < rooms[baseTo].capacity;
                    }
                }

                if (canMove) {
                    someMovement = true;
                    // Effectuer le mouvement
                    ant.currentRoom = to;
                    ant.pathIndex++;

                    // Gérer les mouvements entre salles réelles
                    if (isSource && !isInternal) {
                        // Fourmi quitte S_v pour aller vers une vraie salle
                        nextOccupancy["S_v"]--;
                        nextOccupancy[baseTo]++;
                        step.push_back("    Ant " + std::to_string(ant.id) + " - S_v to " + baseTo);
                    } else if (!isInternal && !isSink && 
                               from.find("_out") != std::string::npos && to.find("_in") != std::string::npos) {
                        // Fourmi passe d'une salle_out vers une autre salle_in (mouvement entre salles)
                        if (baseFrom != baseTo) {
                            nextOccupancy[baseFrom]--;
                            nextOccupancy[baseTo]++;
                            step.push_back("    Ant " + std::to_string(ant.id) + " - " + baseFrom + " to " + baseTo);
                        }
                    } else if (isSink) {
                        // Fourmi arrive à destination
                        if (from.find("_out") != std::string::npos) {
                            nextOccupancy[baseFrom]--;
                        }
                        nextOccupancy["S_d"]++;
                        step.push_back("    Ant " + std::to_string(ant.id) + " - " + baseFrom + " to S_d");
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

        // Afficher l'étape si des mouvements ont eu lieu
        if (someMovement) {
            std::cout << "\n        === E " << stepNumber << " ===\n";
            if (step.empty()) {
                std::cout << "    (Mouvements internes seulement)\n";
            } else {
                for (const auto& move : step) {
                    std::cout << move << "\n";
                }
            }
            stepNumber++;
            
            // Afficher l'état des salles pour debug
            std::cout << "    Etat des salles: ";
            for (const auto& [name, room] : rooms) {
                if (name.find("_in") == std::string::npos && name.find("_out") == std::string::npos) {
                    std::cout << name << "(" << room.occupancy << "/" << room.capacity << ") ";
                }
            }
            std::cout << "\n";
        } else if (!allArrived) {
            // Si aucun mouvement n'est possible mais toutes les fourmis ne sont pas arrivées
            std::cout << "\n    BLOCAGE DETECTE - Aucun mouvement possible\n";
            break;
        }
    }

    if (allArrived) {
        std::cout << "\n=== SIMULATION TERMINEE - Toutes les fourmis sont arrivees ===\n";
    }
}

void Anthill::simulate(int numAnts) {
    ants.clear();
    steps.clear();
    for (int i = 0; i < numAnts; ++i)
        ants.push_back(Ant{i + 1});

    edmondsKarp("S_v_out", "S_d_in");
    buildPaths();
    scheduleMovements();
}