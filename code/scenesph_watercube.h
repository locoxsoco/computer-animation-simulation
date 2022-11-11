#ifndef SCENESPHWATERCUBE_H
#define SCENESPHWATERCUBE_H

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <list>
#include "scene.h"
#include "widgetop.h"
#include "particlesystem.h"
#include "integrators.h"
#include "colliders.h"
#include "hash.h"

class SceneSPHWaterCube : public Scene
{
    Q_OBJECT

public:
    SceneSPHWaterCube();
    virtual ~SceneSPHWaterCube();

    virtual void initialize(double dt, double bo, double fr, unsigned int dragt);
    virtual void reset(double dt, double bo, double fr, unsigned int dragt);
    virtual void update();
    virtual void paint(const Camera& cam);

    virtual void mousePressed(const QMouseEvent* e, const Camera& cam);
    virtual void mouseMoved(const QMouseEvent* e, const Camera& cam);
    virtual void mouseReleased(const QMouseEvent* e, const Camera& cam);
    virtual void keyPressed(const QKeyEvent* e, const Camera& cam);
    virtual void keyReleased(const QKeyEvent* e, const Camera& cam);

    virtual void getSceneBounds(Vec3& bmin, Vec3& bmax) {
        bmin = Vec3(-50, -10, -50);
        bmax = Vec3( 50, 100, 50);
    }
    virtual unsigned int getNumParticles() { return system.getNumParticles(); }

    virtual QWidget* sceneUI() { return widget; }

public slots:
    void updateSimParams();

protected:
    WidgetOP* widget = nullptr;

    QOpenGLShaderProgram* shader = nullptr;
    QOpenGLVertexArrayObject* vaoSphereS = nullptr, *vaoSphereBigS = nullptr, *vaoCube = nullptr,*vaoMesh=nullptr;
    QOpenGLVertexArrayObject* vaoFloor   = nullptr;
    QOpenGLBuffer* vboMesh=nullptr, *iboMesh=nullptr;
    unsigned int numFacesSphereS=0, numFacesSphereBigS = 0, numFacesCube = 0;

    IntegratorSymplecticEuler integrator;
    ParticleSystem system;
    QVector<ForceConstAcceleration*> fSPHSystem;
    std::list<Particle*> deadParticles;
    ForceConstAcceleration* fGravity;
    ForceBlackhole* fBlackhole;
    ForceConstAcceleration *fWind = nullptr;
    ColliderPlane colliderFloor;
    ColliderSphere colliderSphere;
    ColliderInnerAABB colliderCube;

    double emitRate;
    double maxParticleLife;

    QVector<Particle*> poolParticles;
    QVector<Particle*> dropParticles;
    float water_radius=1.f;
    Vec3i poolSize = Vec3i(18,5,18);
    Vec3i dropSize = Vec3i(18,18,18);
    int mouseX, mouseY;

    Hash *hash;

    Particle* selectedPi=nullptr;
    int select_pi_status=0;//0:available; 1:found; 2:failed;

    int curr_step=0;

    QSet<int> keysPressed;
};

#endif // SCENESPHWATERCUBE_H
