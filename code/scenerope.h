#ifndef SCENEROPE_H
#define SCENEROPE_H

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <list>
#include "scene.h"
#include "widgetrope.h"
#include "particlesystem.h"
#include "integrators.h"
#include "colliders.h"
#include "hash.h"
#include "rope.h"

class SceneRope : public Scene
{
    Q_OBJECT

public:
    SceneRope();
    virtual ~SceneRope();

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
    WidgetRope* widget = nullptr;

    QOpenGLShaderProgram* shader = nullptr;
    QOpenGLVertexArrayObject* vaoSphereS = nullptr, *vaoSphereBigS = nullptr, *vaoCube = nullptr;
    QOpenGLVertexArrayObject* vaoFloor   = nullptr;
    unsigned int numFacesSphereS=0, numFacesSphereBigS = 0, numFacesCube = 0;

    IntegratorVerlet integrator;
    ParticleSystem system;
    std::list<Particle*> deadParticles;
    ForceConstAcceleration* fGravity;
    ForceBlackhole* fBlackhole;
    ColliderPlane colliderFloor;
    ColliderSphere colliderSphere;
    ColliderAABB colliderAABB;

    double kBounce, kFriction;
    double emitRate;
    double maxParticleLife;

    Vec3 fountainPos;
    int mouseX, mouseY;

    Hash *hash;
    Rope* rope;
};

#endif // SCENEROPE_H
