#include "colliders.h"
#include <cmath>


/*
 * Plane
 */

bool ColliderPlane::testCollision(const Particle* p) const
{
    // TODO
    return (planeN.dot(p->pos)+planeD)*(planeN.dot(p->prevPos)+planeD)<=0;
}

void ColliderPlane::resolveCollision(Particle* p, double kElastic, double kFriction, double dt) const
{
    // TODO
    p->pos = p->pos - (1+kElastic)*(planeN.dot(p->pos)+planeD)*planeN;
    Vecd velElastic = - (1+kElastic)*planeN.dot(p->vel)*planeN;
    p->vel = p->vel + velElastic;

    Vecd velT = p->vel - planeN.dot(p->vel)*planeN;
    p->vel = p->vel - (kFriction)*velT;

    //for verlet integration
    p->prevPos -= (velElastic - (kFriction)*velT)*dt;
}

/*
 * Sphere
 */

bool ColliderSphere::testCollision(const Particle* p) const
{
    // TODO
    Vecd pointDiff = p->pos-sphereC;
    return pointDiff.dot(pointDiff.transpose())<=sphereR*sphereR;
}

void ColliderSphere::resolveCollision(Particle* p, double kElastic, double kFriction, double dt) const
{
    // TODO
    // Decided to use prevPos instead of pos to improve bouncing visualization and avoid stickness glitch to the sphere
    Vecd planeN = p->prevPos-sphereC;
    //double eqScale = planeN.norm();
    planeN = planeN/planeN.norm();
    double planeD = -planeN.dot(p->prevPos);

    p->pos = p->prevPos - (1+kElastic)*(planeN.dot(p->prevPos)+planeD)*planeN;
    Vecd velElastic = - (1+kElastic)*planeN.dot(p->vel)*planeN;
    p->vel = p->vel+velElastic;

    Vecd velT = p->vel - planeN.dot(p->vel)*planeN;
    p->vel = p->vel - (kFriction)*velT;

    Vecd pointDiff = p->pos-sphereC;
    if(pointDiff.dot(pointDiff.transpose())<=sphereR*sphereR && !p->lock){
        p->vel = p->vel - (1+kElastic)*planeN.dot(-pointDiff*0.7f)*planeN;
    }

    //for verlet integration
    p->prevPos -= 0.7*(velElastic - (kFriction)*velT)*dt;
}

/*
 * AABB
 */

bool ColliderAABB::testCollision(const Particle* p) const
{
    return p->pos.x() >= (pos.x()-scale.x()) && p->pos.x() <= (pos.x()+scale.x()) &&
           p->pos.y() >= (pos.y()-scale.y()) && p->pos.y() <= (pos.y()+scale.y()) &&
           p->pos.z() >= (pos.z()-scale.z()) && p->pos.z() <= (pos.z()+scale.z()) ;
}

void ColliderAABB::resolveCollision(Particle* p, double kElastic, double kFriction, double dt) const
{
    Vecd planeN;
    Vecd velT= Vec3(0.f,0.f,0.f);
    double planeD;
    Vecd velElastic= Vec3(0.f,0.f,0.f);
    // Collision with axis -x
    if( p->prevPos.x() <= (pos.x()-scale.x()) && p->pos.x() >= (pos.x()-scale.x()) ){
        planeN = Vec3(-1.f,0.f,0.f);
        planeD = (pos.x()-scale.x());

        p->pos = p->pos - (1+kElastic)*(planeN.dot(p->pos)+planeD)*planeN;
        velElastic = - (1+kElastic)*planeN.dot(p->vel)*planeN;
        p->vel = p->vel + velElastic;

        velT = p->vel - planeN.dot(p->vel)*planeN;
        p->vel = p->vel - (kFriction)*velT;
    }
    // Collision with axis x
    else if( p->prevPos.x() >= (pos.x()+scale.x()) && p->pos.x() <= (pos.x()+scale.x()) ){
        planeN = Vec3(1.f,0.f,0.f);
        planeD = -(pos.x()+scale.x());

        p->pos = p->pos - (1+kElastic)*(planeN.dot(p->pos)+planeD)*planeN;
        velElastic = - (1+kElastic)*planeN.dot(p->vel)*planeN;
        p->vel = p->vel + velElastic;

        velT = p->vel - planeN.dot(p->vel)*planeN;
        p->vel = p->vel - (kFriction)*velT;
    }
    // Collision with axis -y
    else if( p->prevPos.y() <= (pos.y()-scale.y()) && p->pos.y() >= (pos.y()-scale.y()) ){
        planeN = Vec3(0.f,-1.f,0.f);
        planeD = (pos.y()-scale.y());

        p->pos = p->pos - (1+kElastic)*(planeN.dot(p->pos)+planeD)*planeN;
        velElastic = - (1+kElastic)*planeN.dot(p->vel)*planeN;
        p->vel = p->vel + velElastic;

        velT = p->vel - planeN.dot(p->vel)*planeN;
        p->vel = p->vel - (kFriction)*velT;
    }
    // Collision with axis y
    else if( p->prevPos.y() >= (pos.y()+scale.y()) && p->pos.y() <= (pos.y()+scale.y()) ){
        planeN = Vec3(0.f,1.f,0.f);
        planeD = -(pos.y()+scale.y());

        p->pos = p->pos - (1+kElastic)*(planeN.dot(p->pos)+planeD)*planeN;
        velElastic = - (1+kElastic)*planeN.dot(p->vel)*planeN;
        p->vel = p->vel + velElastic;

        velT = p->vel - planeN.dot(p->vel)*planeN;
        p->vel = p->vel - (kFriction)*velT;
    }
    // Collision with axis -z
    else if( p->prevPos.z() <= (pos.z()-scale.z()) && p->pos.z() >= (pos.z()-scale.z()) ){
        planeN = Vec3(0.f,0.f,-1.f);
        planeD = (pos.z()-scale.z());

        p->pos = p->pos - (1+kElastic)*(planeN.dot(p->pos)+planeD)*planeN;
        velElastic = - (1+kElastic)*planeN.dot(p->vel)*planeN;
        p->vel = p->vel + velElastic;

        velT = p->vel - planeN.dot(p->vel)*planeN;
        p->vel = p->vel - (kFriction)*velT;
    }
    // Collision with axis z
    else if( p->prevPos.z() >= (pos.z()+scale.z()) && p->pos.z() <= (pos.z()+scale.z()) ){
        planeN = Vec3(0.f,0.f,1.f);
        planeD = -(pos.z()+scale.z());

        p->pos = p->pos - (1+kElastic)*(planeN.dot(p->pos)+planeD)*planeN;
        velElastic = - (1+kElastic)*planeN.dot(p->vel)*planeN;
        p->vel = p->vel + velElastic;

        velT = p->vel - planeN.dot(p->vel)*planeN;
        p->vel = p->vel - (kFriction)*velT;
    }    

    //for verlet integration
    p->prevPos -= (velElastic - (kFriction)*velT)*dt;
}

/*
 * Inner AABB
 */

bool ColliderLambdaInnerAABB::testCollision(const Particle* p) const
{
    return !(p->pos.x() >= (pos.x()-scale.x()) && p->pos.x() <= (pos.x()+scale.x()) &&
           p->pos.y() >= (pos.y()-scale.y()) && p->pos.y() <= (pos.y()+scale.y()) &&
           p->pos.z() >= (pos.z()-scale.z()) && p->pos.z() <= (pos.z()+scale.z())) ;
}

void ColliderLambdaInnerAABB::calcLambda(Particle* p, Vec3 (&planesN)[6], double (&planesD)[6], double &lambda, unsigned int &idx) const{
    for(unsigned int i=0;i<3;i++)
        for(unsigned int j=0;j<2;j++){
            idx = i*2 + j;
            lambda = (-planesD[idx]/planesN[idx][i]-p->prevPos[i])/(p->pos[i]-p->prevPos[i]);
            if(lambda > 0.f && lambda < 1.f)
                return;
        }
}

void ColliderLambdaInnerAABB::resolveCollision(Particle* p, double kElastic, double kFriction, double dt) const
{
    /* The collision can be represented as the intersection of a ray with Po prevPos,
     * direction (pos-prePos), and a lambda [0,1], with one of the planes of the cube.
     *
     *        -----------
     *       |           |
     *       |  prevPos  |
     *       |        o--|-->o
     *       |           |   pos
     *       |           |
     *        -----------
     * In order to create 6 equations representing the intersection of each cube face
     * with the ray, if possible. There will be only one with a lambda [0,1], so that
     * will be the one chosen for the collision resolve (unless the ray intersects
     * in the intersection of two planes, aka a corner or seam). If the collision is near a
     * corner or seam, the resolve will probably throw it outside the cube. If the resulting
     * pos is outside the cube, the resolve collision will be replaced by an inverse
     * direction (like if its intersection is exactly at the corner).
     */

    // Array for plane normals +x, -x, +y, -y, +z, -z respectively
    Vec3 planesN[6] = {
        Vec3(-1.f,  0.f,  0.f),
        Vec3( 1.f,  0.f,  0.f),
        Vec3( 0.f, -1.f,  0.f),
        Vec3( 0.f,  1.f,  0.f),
        Vec3( 0.f,  0.f, -1.f),
        Vec3( 0.f,  0.f,  1.f),
    };
    // Array for plane distances +x, -x, +y, -y, +z, -z respectively
    double planesD[6] = {
         this->pos.x() + this->scale.x(),
        -this->pos.x() + this->scale.x(),
         this->pos.y() + this->scale.y(),
        -this->pos.y() + this->scale.y(),
         this->pos.z() + this->scale.z(),
        -this->pos.z() + this->scale.z(),
    };

    double lambda;
    unsigned int idx;
    calcLambda(p,planesN,planesD,lambda,idx);


    //Vec3 posCollision = p->prevPos + lambda*(p->pos-p->prevPos);

    //p->prevPos = p->pos;
    p->pos = p->pos - (1+kElastic)*(planesN[idx].dot(p->pos)+planesD[idx])*planesN[idx];

    Vec3 velElastic = - (1+kElastic)*planesN[idx].dot(p->vel)*planesN[idx];
    p->vel = p->vel + velElastic;

    Vec3 velT = p->vel - planesN[idx].dot(p->vel)*planesN[idx];
    p->vel = p->vel - (kFriction)*velT;
}

/*
 * Snowball
 */

bool ColliderSnowball::testCollision(const Particle* p) const
{
    // TODO
    Vecd pointDiff = p->pos-sphereC;
    return pointDiff.dot(pointDiff.transpose())>=sphereR*sphereR;
}

void ColliderSnowball::resolveCollision(Particle* p, double kElastic, double kFriction, double dt) const
{
    // TODO
    // Decided to use prevPos instead of pos to improve bouncing visualization and avoid stickness glitch to the sphere
    Vecd planeN = -(p->prevPos-sphereC);
    //double eqScale = planeN.norm();
    planeN = planeN/planeN.norm();
    double planeD = -planeN.dot(p->prevPos);

    p->pos = p->prevPos - (1+kElastic)*(planeN.dot(p->prevPos)+planeD)*planeN;
    Vecd velElastic = - (1+kElastic)*planeN.dot(p->vel)*planeN;
    p->vel = p->vel +velElastic;

    Vecd velT = p->vel - planeN.dot(p->vel)*planeN;
    p->vel = p->vel - (kFriction)*velT;

    //for verlet integration
    p->prevPos -= (velElastic - (kFriction)*velT)*dt;
}
