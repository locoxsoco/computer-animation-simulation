#ifndef INTEGRATORS_H
#define INTEGRATORS_H

#include "particlesystem.h"

class Integrator {
public:
    Integrator() {};
    virtual ~Integrator() {};
    virtual void step(ParticleSystem& system, double dt) = 0;
};


class IntegratorEuler : public Integrator {
public:
    virtual void step(ParticleSystem& system, double dt);
};


class IntegratorSymplecticEuler : public Integrator {
public:
    virtual void step(ParticleSystem& system, double dt);
};


class IntegratorMidpoint : public Integrator {
public:
    virtual void step(ParticleSystem& system, double dt);
};


class IntegratorVerlet : public Integrator {
public:
    virtual void step(ParticleSystem& system, double dt);
    float k = 0.95f;
};

class IntegratorRK2 : public Integrator {
public:
    virtual void step(ParticleSystem& system, double dt);
};

class IntegratorRK4 : public Integrator {
public:
    virtual void step(ParticleSystem& system, double dt);
};


#endif // INTEGRATORS_H
