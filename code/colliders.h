#ifndef COLLIDERS_H
#define COLLIDERS_H

#include "defines.h"
#include "particle.h"


class Collider  // Abstract interface
{
public:
    Collider() {}
    virtual ~Collider() {}

    virtual bool testCollision(const Particle* p) const = 0;
    virtual void resolveCollision(Particle* p, double kElastic, double kFriction) const = 0;
};


class ColliderPlane : public Collider
{
public:
    ColliderPlane() { planeN = Vec3(0,0,0); planeD = 0; }
    ColliderPlane(const Vec3& n, double d) : planeN(n), planeD(d) {}
    virtual ~ColliderPlane() {}

    void setPlane(const Vec3& n, double d) { this->planeN = n; this->planeD = d; }

    virtual bool testCollision(const Particle* p) const;
    virtual void resolveCollision(Particle* p, double kElastic, double kFriction) const;

protected:
    Vec3 planeN;
    double planeD;
};

class ColliderAABB: public Collider
{
public:
    ColliderAABB() { pointA = Vec3(0,0,0); pointB = Vec3(0,0,0); }
    ColliderAABB(const Vec3& a, const Vec3& b) : pointA(a), pointB(b) {}
    virtual ~ColliderAABB() {}

    void setAABB(const Vec3& a, const Vec3& b) { this->pointA = a; this->pointB = b; }

    virtual bool testCollision(const Particle* p) const;
    virtual void resolveCollision(Particle* p, double kElastic, double kFriction) const;

protected:
    Vec3 pointA, pointB;
};

class ColliderSphere : public Collider
{
public:
    ColliderSphere() { sphereC = Vec3(0,0,0); sphereR = 0; }
    ColliderSphere(const Vec3& c, double r) : sphereC(c), sphereR(r) {}
    virtual ~ColliderSphere() {}

    void setSphere(const Vec3& c, double r) { this->sphereC = c; this->sphereR = r; }

    virtual bool testCollision(const Particle* p) const;
    virtual void resolveCollision(Particle* p, double kElastic, double kFriction) const;

    Vec3 sphereC;
    double sphereR;
};

#endif // COLLIDERS_H
