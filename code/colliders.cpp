#include "colliders.h"
#include <cmath>


/*
 * Plane
 */

bool ColliderPlane::testCollision(const Particle* p) const
{
    // TODO
    if((planeN.dot(p->pos)+planeD)*(planeN.dot(p->prevPos)+planeD)<=0) return true;
    return false;
}

void ColliderPlane::resolveCollision(Particle* p, double kElastic, double kFriction) const
{
    // TODO
    p->pos.y() = p->pos.y() - (1+kElastic)*p->pos.y();
    p->vel.y() = p->vel.y() - (1+kElastic)*p->vel.y();

    Vecd velT = p->vel - planeN.dot(p->vel)*planeN;
    p->vel = p->vel - (kFriction)*velT;
}
