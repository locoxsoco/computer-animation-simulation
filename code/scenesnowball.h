#ifndef SceneSnowball_H
#define SceneSnowball_H

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <list>
#include "scene.h"
#include "widgetsnowball.h"
#include "particlesystem.h"
#include "integrators.h"
#include "colliders.h"
#include "hash.h"

class SceneSnowball : public Scene
{
    Q_OBJECT

public:
    SceneSnowball();
    virtual ~SceneSnowball();

    virtual void initialize(double dt, double bo, double fr, unsigned int dragt);
    virtual void reset(double dt, double bo, double fr, unsigned int dragt);
    virtual void update();
    virtual void paint(const Camera& cam);

    virtual void mousePressed(const QMouseEvent* e, const Camera& cam);
    virtual void mouseMoved(const QMouseEvent* e, const Camera& cam);

    virtual void getSceneBounds(Vec3& bmin, Vec3& bmax) {
        bmin = Vec3(-50, -10, -50);
        bmax = Vec3( 50, 100, 50);
    }
    virtual unsigned int getNumParticles() { return system.getNumParticles(); }

    virtual QWidget* sceneUI() { return widget; }

public slots:
    void updateSimParams();

protected:
    WidgetSnowball* widget = nullptr;

    QOpenGLShaderProgram* shader = nullptr;
    QOpenGLVertexArrayObject* vaoSphereS = nullptr, *vaoSphereBigS = nullptr;
    unsigned int numFacesSphereS=0, numFacesSphereBigS = 0;

    IntegratorEuler integrator;
    ParticleSystem system;
    std::list<Particle*> deadParticles;
    ForceConstAcceleration* fGravity;
    ForceBlackhole* fBlackhole;
    ColliderSnowball colliderSnowball;

    double kBounce, kFriction;
    double emitRate;
    double maxParticleLife;

    int mouseX, mouseY;

    Particle* pp;

    Hash *hash;
};

#endif // SceneSnowball_H
