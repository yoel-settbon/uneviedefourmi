#include "models/ants.hpp"

int main() {
    Anthill anthill;
    anthill.addRoom("S_v", 100);
    anthill.addRoom("A", 2);
    anthill.addRoom("B", 3);
    anthill.addRoom("C", 1);
    anthill.addRoom("S_d", 100);

    anthill.addTunnel("S_v", "A");
    anthill.addTunnel("S_v", "B");
    anthill.addTunnel("A", "C");
    anthill.addTunnel("B", "C");
    anthill.addTunnel("C", "S_d");

    anthill.simulate(8);
    return 0;
}
