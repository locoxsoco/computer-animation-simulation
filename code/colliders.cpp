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

void ColliderPlane::resolveCollision(Particle* p, double kElastic, double kFriction) const
{
    // TODO
    p->pos = p->pos - (1+kElastic)*(planeN.dot(p->pos)+planeD)*planeN;
    p->vel = p->vel - (1+kElastic)*planeN.dot(p->vel)*planeN;

    Vecd velT = p->vel - planeN.dot(p->vel)*planeN;
    p->vel = p->vel - (kFriction)*velT;
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

void ColliderSphere::resolveCollision(Particle* p, double kElastic, double kFriction) const
{
    // TODO
    // Decided to use prevPos instead of pos to improve bouncing visualization and avoid stickness glitch to the sphere
    Vecd planeN = p->prevPos-sphereC;
    //double eqScale = planeN.norm();
    planeN = planeN/planeN.norm();
    double planeD = -planeN.dot(p->prevPos);

    p->pos = p->prevPos - (1+kElastic)*(planeN.dot(p->prevPos)+planeD)*planeN;
    p->vel = p->vel - (1+kElastic)*planeN.dot(p->vel)*planeN;

    Vecd velT = p->vel - planeN.dot(p->vel)*planeN;
    p->vel = p->vel - (kFriction)*velT;
}

/*
 * AABB
 */

bool ColliderAABB::testCollision(const Particle* p) const
{
    return false;
}

void ColliderAABB::resolveCollision(Particle* p, double kElastic, double kFriction) const
{

}
