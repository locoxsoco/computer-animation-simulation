#ifndef SCENECLOTH_H
#define SCENECLOTH_H

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <list>
#include "scene.h"
#include "widgetcloth.h"
#include "particlesystem.h"
#include "integrators.h"
#include "colliders.h"
#include "hash.h"
#include "cloth.h"

class SceneCloth : public Scene
{
    Q_OBJECT

public:
    SceneCloth();
    virtual ~SceneCloth();

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
    WidgetCloth* widget = nullptr;

    QOpenGLShaderProgram* shader = nullptr;
    QOpenGLVertexArrayObject* vaoSphereS = nullptr, *vaoSphereBigS = nullptr, *vaoCube = nullptr,*vaoMesh=nullptr;
    QOpenGLVertexArrayObject* vaoFloor   = nullptr;
    QOpenGLBuffer* vboMesh=nullptr, *iboMesh=nullptr;
    QOpenGLShaderProgram* shaderCloth = nullptr;
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
    Cloth* cloth;


    float deformation_rate = 0.1f;
    int relaxation_steps = 10;
    int numParticlesX=20, numParticlesY=20,numMeshIndices;
};

#endif // SCENECLOTH_H
