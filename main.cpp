#include "models/ants.hpp"

int main() {
    Anthill anthill;
    // Configuration pour f=10 fourmis
    anthill.addRoom("S_v", 100);
    anthill.addRoom("S1", 2);
    anthill.addRoom("S2", 1);  // Capacité par défaut = 1 si non spécifiée
    anthill.addRoom("S3", 1);  // Capacité par défaut = 1 si non spécifiée
    anthill.addRoom("S4", 2);
    anthill.addRoom("S5", 1);  // Capacité par défaut = 1 si non spécifiée
    anthill.addRoom("S6", 1);  // Capacité par défaut = 1 si non spécifiée
    anthill.addRoom("S_d", 100);

    anthill.addTunnel("S_v", "S1");
    anthill.addTunnel("S1", "S2");
    anthill.addTunnel("S2", "S4");
    anthill.addTunnel("S4", "S5");
    anthill.addTunnel("S5", "S_d");
    anthill.addTunnel("S4", "S6");
    anthill.addTunnel("S6", "S_d");
    anthill.addTunnel("S1", "S3");
    anthill.addTunnel("S3", "S4");

    anthill.simulate(10);
    return 0;
}
