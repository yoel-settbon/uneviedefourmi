#include "models/ants.hpp"
#include <iostream>

int main() {
    Anthill anthill;

    anthill.addRoom("S_v", 100);
    anthill.addRoom("S_1", 5);
    anthill.addRoom("S_2", 3);
    anthill.addRoom("S_3", 2);
    anthill.addRoom("S_4");
    anthill.addRoom("S_d", 100);

    anthill.addTunnel("S_v", "S_1");
    anthill.addTunnel("S_v", "S_2");
    anthill.addTunnel("S_1", "S_2");
    anthill.addTunnel("S_2", "S_3");
    anthill.addTunnel("S_3", "S_4");
    anthill.addTunnel("S_4", "S_d");

    anthill.printGraph();

    anthill.simulate(5);

    return 0;
}
