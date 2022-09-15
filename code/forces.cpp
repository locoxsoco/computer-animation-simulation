#include "forces.h"

void ForceConstAcceleration::apply() {
    for (Particle* p : particles) {
        // TODO
        p->force += getAcceleration();
    }
}
