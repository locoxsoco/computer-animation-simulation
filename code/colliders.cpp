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

bool ColliderInnerAABB::testCollision(const Particle* p) const
{
    return !(p->pos.x() >= (pos.x()-scale.x()) && p->pos.x() <= (pos.x()+scale.x()) &&
           p->pos.y() >= (pos.y()-scale.y()) && p->pos.y() <= (pos.y()+scale.y()) &&
           p->pos.z() >= (pos.z()-scale.z()) && p->pos.z() <= (pos.z()+scale.z())) ;
}

void ColliderInnerAABB::resolveCollision(Particle* p, double kElastic, double kFriction, double dt) const
{
    Vecd planeN;
    Vecd velT= Vec3(0.f,0.f,0.f);
    double planeD;
    Vecd velElastic= Vec3(0.f,0.f,0.f);
    // Collision with axis -x
    if( p->prevPos.x() >= (pos.x()-scale.x()) && p->pos.x() <= (pos.x()-scale.x()) ){
        planeN = Vec3(1.f,0.f,0.f);
        planeD = -(pos.x()-scale.x());

        p->pos = p->pos - (1+kElastic)*(planeN.dot(p->pos)+planeD)*planeN;
        velElastic = - (1+kElastic)*planeN.dot(p->vel)*planeN;
        p->vel = p->vel + velElastic;

        velT = p->vel - planeN.dot(p->vel)*planeN;
        p->vel = p->vel - (kFriction)*velT;
    }
    // Collision with axis x
    else if( p->prevPos.x() <= (pos.x()+scale.x()) && p->pos.x() >= (pos.x()+scale.x()) ){
        planeN = Vec3(-1.f,0.f,0.f);
        planeD = (pos.x()+scale.x());

        p->pos = p->pos - (1+kElastic)*(planeN.dot(p->pos)+planeD)*planeN;
        velElastic = - (1+kElastic)*planeN.dot(p->vel)*planeN;
        p->vel = p->vel + velElastic;

        velT = p->vel - planeN.dot(p->vel)*planeN;
        p->vel = p->vel - (kFriction)*velT;
    }
    // Collision with axis -y
    else if( p->prevPos.y() >= (pos.y()-scale.y()) && p->pos.y() <= (pos.y()-scale.y()) ){
        planeN = Vec3(0.f,1.f,0.f);
        planeD = -(pos.y()-scale.y());

        p->pos = p->pos - (1+kElastic)*(planeN.dot(p->pos)+planeD)*planeN;
        velElastic = - (1+kElastic)*planeN.dot(p->vel)*planeN;
        p->vel = p->vel + velElastic;

        velT = p->vel - planeN.dot(p->vel)*planeN;
        p->vel = p->vel - (kFriction)*velT;
    }
    // Collision with axis y
    else if( p->prevPos.y() <= (pos.y()+scale.y()) && p->pos.y() >= (pos.y()+scale.y()) ){
        planeN = Vec3(0.f,-1.f,0.f);
        planeD = (pos.y()+scale.y());

        p->pos = p->pos - (1+kElastic)*(planeN.dot(p->pos)+planeD)*planeN;
        velElastic = - (1+kElastic)*planeN.dot(p->vel)*planeN;
        p->vel = p->vel + velElastic;

        velT = p->vel - planeN.dot(p->vel)*planeN;
        p->vel = p->vel - (kFriction)*velT;
    }
    // Collision with axis -z
    else if( p->prevPos.z() >= (pos.z()-scale.z()) && p->pos.z() <= (pos.z()-scale.z()) ){
        planeN = Vec3(0.f,0.f,1.f);
        planeD = -(pos.z()-scale.z());

        p->pos = p->pos - (1+kElastic)*(planeN.dot(p->pos)+planeD)*planeN;
        velElastic = - (1+kElastic)*planeN.dot(p->vel)*planeN;
        p->vel = p->vel + velElastic;

        velT = p->vel - planeN.dot(p->vel)*planeN;
        p->vel = p->vel - (kFriction)*velT;
    }
    // Collision with axis z
    else if( p->prevPos.z() <= (pos.z()+scale.z()) && p->pos.z() >= (pos.z()+scale.z()) ){
        planeN = Vec3(0.f,0.f,-1.f);
        planeD = (pos.z()+scale.z());

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
