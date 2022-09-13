#include "integrators.h"


void IntegratorEuler::step(ParticleSystem &system, double dt) {
    Vecd x0 = system.getState();
    Vecd dx = system.getDerivative();
    Vecd x1 = x0 + dt*dx;
    system.setState(x1);
}


void IntegratorSymplecticEuler::step(ParticleSystem &system, double dt) {
    // TODO
    Vecd p0 = system.getPositions();

    Vecd x0 = system.getState();
    Vecd dx = system.getDerivative();
    Vecd x1 = x0 + dt*dx;
    system.setState(x1);
    Vecd v1 = system.getVelocities();
    Vecd p1 = p0 + dt*v1;
    system.setPositions(p1);
}


void IntegratorMidpoint::step(ParticleSystem &system, double dt) {
    // TODO
    Vecd x0 = system.getState();
    Vecd dx = system.getDerivative();
    Vecd x1_2 = x0 + dt/2*dx;
    system.setState(x1_2);
    Vecd dx1_2 = system.getDerivative();
    Vecd x1 = x0 + dt*dx1_2;
    system.setState(x1);
}


void IntegratorVerlet::step(ParticleSystem &system, double dt) {
    // TODO
    float k = 0.99;
    Vecd p0 = system.getPositions();
    Vecd pp1 = system.getPreviousPositions();
    Vecd p1 = p0 + k*(p0 - pp1) + dt*dt*system.getAccelerations();

    system.setPositions(p1);
    system.setPreviousPositions(p0);
    system.updateForces();
}
