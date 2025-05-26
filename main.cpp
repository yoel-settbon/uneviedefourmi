#include "models/ants.hpp"

int main() {
    Anthill anthill;
    anthill.addRoom("S_v", 100);
    anthill.addRoom("S1", 8);
    anthill.addRoom("S2", 4);
    anthill.addRoom("S3", 2);
    anthill.addRoom("S4", 4);
    anthill.addRoom("S5", 2);
    anthill.addRoom("S6", 4);
    anthill.addRoom("S7", 2);
    anthill.addRoom("S8", 5);
    anthill.addRoom("S9", 1);
    anthill.addRoom("S10", 1);
    anthill.addRoom("S11", 1);
    anthill.addRoom("S12", 1);
    anthill.addRoom("S13", 4);
    anthill.addRoom("S14", 2);
    anthill.addRoom("S_d", 100);


    anthill.addTunnel("S1", "S2");
    anthill.addTunnel("S2", "S3");
    anthill.addTunnel("S3", "S4");
    anthill.addTunnel("S4", "S_d");
    anthill.addTunnel("S_v", "S1");
    anthill.addTunnel("S2", "S5");
    anthill.addTunnel("S5", "S4");
    anthill.addTunnel("S13", "S_d");
    anthill.addTunnel("S8", "S12");
    anthill.addTunnel("S12", "S13");
    anthill.addTunnel("S6", "S7");
    anthill.addTunnel("S7", "S9");
    anthill.addTunnel("S9", "S14");
    anthill.addTunnel("S14", "S_d");
    anthill.addTunnel("S7", "S10");
    anthill.addTunnel("S10", "S14");
    anthill.addTunnel("S1", "S6");
    anthill.addTunnel("S6", "S8");
    anthill.addTunnel("S8", "S11");
    anthill.addTunnel("S11", "S13");


    anthill.simulate(50);
    return 0;
}
