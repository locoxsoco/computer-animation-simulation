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
    Vecd v0 = system.getVelocities();

    Vecd v1 = v0 + dt*system.getAccelerations();
    Vecd p1 = p0 + dt*v1;
    system.setPositions(p1);
    system.setVelocities(v1);
    system.updateForces();
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
    Vecd p0 = system.getPositions();
    Vecd pp1 = system.getPreviousPositions();
    Vecd p1 = p0 + k*(p0 - pp1) + dt*dt*system.getAccelerations();

    system.setPositions(p1);
    system.setPreviousPositions(p0);
    system.setVelocities((p1-p0)/dt);
    system.updateForces();
}

void IntegratorRK2::step(ParticleSystem &system, double dt) {
    Vecd x0 = system.getState();
    Vecd k1 = system.getDerivative();

    Vecd xt = x0 + dt*k1;
    system.setState(xt);
    Vecd k2 = system.getDerivative();

    Vecd x1 = x0 + dt/2*(k1+k2);
    system.setState(x1);
}

void IntegratorRK4::step(ParticleSystem &system, double dt) {
    Vecd k1 = system.getDerivative();

    Vecd x0 = system.getState();
    Vecd xt = x0 + dt/2*k1;
    system.setState(xt);
    Vecd k2 = system.getDerivative();

    xt = x0 + dt/2*k2;
    system.setState(xt);
    Vecd k3 = system.getDerivative();

    xt = x0 + dt*k3;
    system.setState(xt);
    Vecd k4 = system.getDerivative();

    Vecd x1 = x0 + dt/6*(k1 + 2*k2 + 2*k3 + k4);
    system.setState(x1);
}
