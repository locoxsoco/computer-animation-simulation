#include "forces.h"
#include <iostream>

void ForceConstAcceleration::apply() {
    for (Particle* p : particles) {
        // TODO
        p->force += p->mass*getAcceleration();
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
    }
}

void ForceBlackhole::apply() {
    for (Particle* p : particles) {
        // TODO
        Vecd dirF = getPosition() - p->pos;
        double dist = dirF.norm();
        p->force += dirF*intensity*1000/(dist*dist*dist);
    }
}
