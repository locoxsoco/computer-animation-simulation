#include "forces.h"
#include <iostream>

void ForceConstAcceleration::apply() {
    for (Particle* p : particles) {
        // TODO
        if(p->lock) continue;
        p->force += p->mass*getAcceleration();
    }
}

void ForceSPH::apply() {
    for (Particle* p : particles) {
        // TODO
        if(p->lock) continue;
        p->force += getForce();
    }
}

void ForceDragLinear::apply() {
    for (Particle* p : particles) {
        // TODO
        if(p->lock) continue;
        p->force += -0.015 * p->vel;
    }
}

void ForceDragQuadratic::apply() {
    for (Particle* p : particles) {
        // TODO
        if(p->lock) continue;
        p->force += -0.015 * p->vel.norm() * p->vel;
    }
}

void ForceBlackhole::apply() {
    for (Particle* p : particles) {
        // TODO
        if(p->lock) continue;
        Vecd dirF = getPosition() - p->pos;
        double dist = dirF.norm();
        p->force += dirF*intensity*1000/(dist*dist*dist);
    }
}

void ForceSpring::apply() {
    Particle* p0 = particles[0];
    Particle* p1 = particles[1];
    Vec3 force_spring = (ke*((p1->pos-p0->pos).norm()-L)+kd*(p1->vel-p0->vel).dot(p1->pos-p0->pos)/(p1->pos-p0->pos).norm())*(p1->pos-p0->pos)/(p1->pos-p0->pos).norm();
    if(!p0->lock) p0->force += force_spring;
    if(!p1->lock) p1->force -= force_spring;
}
