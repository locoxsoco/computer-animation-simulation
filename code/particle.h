#ifndef PARTICLE_H
#define PARTICLE_H

#include "defines.h"

class Particle
{
public:

    static const int PhaseDimension = 6;

    Vec3 pos, prevPos;
    Vec3 vel;
    Vec3 force;
    double mass;
    float density;
    float pressure;
    int type = 0; // 1 is boundary, else 0
    double radius = 1.0;
    double life   = 0.0;
    Vec3 color    = Vec3(1, 1, 1);
    unsigned int id = 0;
    bool lock = false;
    int id_height,id_width;

    Particle() {
        pos	    = Vec3(0.0, 0.0, 0.0);
        vel	    = Vec3(0.0, 0.0, 0.0);
        force   = Vec3(0.0, 0.0, 0.0);
        prevPos = pos;
        mass    = 1.0;
        lock = false;
    }

    Particle(const Vec3& p, const Vec3& v, float m, double timeStep) {
        pos		= p;
        vel		= v;
        force	= Vec3(0.0, 0.0, 0.0);
        prevPos = pos - timeStep*vel;
        mass	= m;
        lock = false;
    }

    Particle(const Vec3& p, const Vec3& v, float m) {
        pos		= p;
        vel		= v;
        force	= Vec3(0.0, 0.0, 0.0);
        prevPos = pos;
        mass	= m;
        lock = false;
    }

    Particle(const Vec3& p) {
        pos	    = p;
        vel	    = Vec3(0.0, 0.0, 0.0);
        force   = Vec3(0.0, 0.0, 0.0);
        prevPos = pos;
        mass    = 1.0;
        lock = false;
    }

    Particle(const Vec3& p, int id_h, int id_w) {
        pos	    = p;
        vel	    = Vec3(0.0, 0.0, 0.0);
        force   = Vec3(0.0, 0.0, 0.0);
        prevPos = pos;
        mass    = 1.0;
        lock = false;
        id_height = id_h;
        id_width= id_w;
    }

    Particle(const Particle& p) {
        id      = p.id;
        pos     = p.pos;
        vel     = p.vel;
        force   = p.force;
        prevPos = p.pos;
        mass    = p.mass;
        color   = p.color;
        radius  = p.radius;
        life    = p.life;
        lock = false;
    }

    ~Particle() {
    }
};


#endif // PARTICLE_H
