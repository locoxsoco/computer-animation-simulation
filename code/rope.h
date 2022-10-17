#ifndef ROPE_H
#define ROPE_H

#include "defines.h"
#include "particle.h"
#include "forces.h"
#include <QVector>

class Rope
{
public:
    Rope(unsigned int n_particles, Vec3 start_position) {
        //Particle* rope_particle;
        for(unsigned int i=0;i<n_particles;i++) {
            //rope_particle = new Particle(start_position+Vec3(0.f,0.f,i));
            particles.push_back(new Particle(start_position+Vec3(0.f,0.f,i)));
        }
        particles[0]->lock = true;

        //ForceSpring* rope_force_spring;
        for(unsigned int i=0;i<n_particles-1;i++) {
            //rope_force_spring = new ForceSpring(particles[i],particles[i+1]);
            force_springs.push_back(new ForceSpring(particles[i],particles[i+1]));
        }
    }

    ~Rope() {
    }

    QVector<Particle*> particles;
    QVector<ForceSpring*> force_springs;
};


#endif // ROPE_H
