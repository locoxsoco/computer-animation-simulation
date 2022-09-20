#include "forces.h"
#include <iostream>

void ForceConstAcceleration::apply() {
    for (Particle* p : particles) {
        // TODO
        p->force += getAcceleration();
    }
}

void ForceDragLinear::apply() {
    for (Particle* p : particles) {
        // TODO
        p->force += -0.015 * p->vel;
    }
}

void ForceDragQuadratic::apply() {
    for (Particle* p : particles) {
        // TODO
        p->force += -0.015 * p->vel.norm() * p->vel;
        std::cout << "force Quadratic Drag: " << p->vel << std::endl;
    }
}
