#ifndef FORCES_H
#define FORCES_H

#include <vector>
#include "particle.h"

class Force
{
public:
    Force(void) {}
    virtual ~Force(void) {}

    virtual void apply() = 0;
    //virtual void setAcceleration(const Vec3& a) = 0;

    void addInfluencedParticle(Particle* p) {
        particles.push_back(p);
    }

    void setInfluencedParticles(const std::vector<Particle*>& vparticles) {
        particles = vparticles;
    }

    void clearInfluencedParticles() {
        particles.clear();
    }

    std::vector<Particle*> getInfluencedParticles() const {
        return particles;
    }

protected:
    std::vector<Particle*>	particles;
};


class ForceConstAcceleration : public Force
{
public:
    ForceConstAcceleration() { acceleration = Vec3(0,0,0); }
    ForceConstAcceleration(const Vec3& a) { acceleration = a; }
    virtual ~ForceConstAcceleration() {}

    virtual void apply();

    void setAcceleration(const Vec3& a) { acceleration = a; }
    Vec3 getAcceleration() const { return acceleration; }

protected:
    Vec3 acceleration;
};


class ForceDragLinear : public Force
{
public:
    ForceDragLinear() { acceleration = Vec3(0,0,0); }
    ForceDragLinear(const Vec3& a) { acceleration = a; }
    virtual ~ForceDragLinear() {}

    virtual void apply();

    void setAcceleration(const Vec3& a) { acceleration = a; }
    Vec3 getAcceleration() const { return acceleration; }

protected:
    Vec3 acceleration;
};


class ForceDragQuadratic : public Force
{
public:
    ForceDragQuadratic() { acceleration = Vec3(0,0,0); }
    ForceDragQuadratic(const Vec3& a) { acceleration = a; }
    virtual ~ForceDragQuadratic() {}

    virtual void apply();

    void setAcceleration(const Vec3& a) { acceleration = a; }
    Vec3 getAcceleration() const { return acceleration; }

protected:
    Vec3 acceleration;
};


class ForceBlackhole : public Force
{
public:
    ForceBlackhole() { position = Vec3(0,0,0); intensity = 0.f; }
    ForceBlackhole(const Vec3& p, const float i) { position = p; intensity = i; }
    virtual ~ForceBlackhole() {}

    virtual void apply();

    void setPosition(const Vec3& p) { position = p; }
    Vec3 getPosition() const { return position; }
    void setIntensity(const float i) { intensity = i; }
    float getIntensity() const { return intensity; }


    Vec3 position;
    float intensity;
};


#endif // FORCES_H
