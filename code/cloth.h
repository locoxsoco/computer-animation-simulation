#ifndef CLOTH_H
#define CLOTH_H

#include "defines.h"
#include "particle.h"
#include "forces.h"
#include <QVector>

class Cloth
{
public:
    Cloth(int n_particles_width, int n_particles_height, Vec3 start_position) {
        //Particle* cloth_particle;
        for(int i=0;i<n_particles_height;i++) {
            for(int j=0;j<n_particles_width;j++) {
                //cloth_particle = new Particle(start_position+Vec3(0.f,0.f,i));
                particles.push_back(new Particle(start_position+Vec3(j,0.f,i)));
            }
        }
        particles[0]->lock = true;
        particles[0]->color = Vec3(0.1f,0.1f,0.1f);
        particles[n_particles_width-1]->lock = true;
        particles[n_particles_width-1]->color = Vec3(0.1f,0.1f,0.1f);

        //ForceSpring* cloth_force_spring;
        for(int i=1;i<n_particles_height-1;i++) {
            for(int j=1;j<n_particles_width-1;j++) {
                //cloth_force_spring = new ForceSpring(particles[i],particles[i+1]);
                // Stretch
                force_springs.push_back(new ForceSpring(particles[i*n_particles_width+j],particles[(i-1)*n_particles_width+j],0));
                force_springs.push_back(new ForceSpring(particles[i*n_particles_width+j],particles[(i+1)*n_particles_width+j],0));
                force_springs.push_back(new ForceSpring(particles[i*n_particles_width+j],particles[i*n_particles_width+(j-1)],0));
                force_springs.push_back(new ForceSpring(particles[i*n_particles_width+j],particles[i*n_particles_width+(j+1)],0));

                // Shear
                force_springs.push_back(new ForceSpring(particles[i*n_particles_width+j],particles[(i-1)*n_particles_width+(j-1)],1));
                force_springs.push_back(new ForceSpring(particles[i*n_particles_width+j],particles[(i+1)*n_particles_width+(j+1)],1));
                force_springs.push_back(new ForceSpring(particles[i*n_particles_width+j],particles[(i-1)*n_particles_width+(j+1)],1));
                force_springs.push_back(new ForceSpring(particles[i*n_particles_width+j],particles[(i+1)*n_particles_width+(j-1)],1));
            }
        }
        for(int i=0;i<n_particles_height;i++) {
            for(int j=2;j<n_particles_width-2;j++) {
                // Bend
                force_springs.push_back(new ForceSpring(particles[i*n_particles_width+j],particles[i*n_particles_width+(j-2)],2));
                force_springs.push_back(new ForceSpring(particles[i*n_particles_width+j],particles[i*n_particles_width+(j+2)],2));
            }
        }
        for(int i=2;i<n_particles_height-2;i++) {
            for(int j=0;j<n_particles_width;j++) {
                // Bend
                force_springs.push_back(new ForceSpring(particles[i*n_particles_width+j],particles[(i-2)*n_particles_width+j],2));
                force_springs.push_back(new ForceSpring(particles[i*n_particles_width+j],particles[(i+2)*n_particles_width+j],2));
            }
        }
        numParticles = n_particles_height * n_particles_width;
    }

    ~Cloth() {
    }

    QVector<Particle*> particles;
    QVector<ForceSpring*> force_springs;
    float thickness = 1.f;
    int numParticles;
};


#endif // CLOTH_H
