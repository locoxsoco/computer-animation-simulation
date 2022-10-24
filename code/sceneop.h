#ifndef SCENEOP_H
#define SCENEOP_H

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMediaPlaylist>
#include <QMediaPlayer>
#include <list>
#include "scene.h"
#include "widgetop.h"
#include "particlesystem.h"
#include "integrators.h"
#include "colliders.h"
#include "hash.h"
#include "sail.h"

class SceneOP : public Scene
{
    Q_OBJECT

public:
    SceneOP();
    virtual ~SceneOP();

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
    void releaseSimLockedParticles();

protected:
    WidgetOP* widget = nullptr;

    QOpenGLShaderProgram* shader = nullptr;
    QOpenGLVertexArrayObject* vaoSphereS = nullptr, *vaoSphereBigS = nullptr, *vaoCube = nullptr,*vaoMesh=nullptr;
    QOpenGLVertexArrayObject* vaoFloor   = nullptr;
    QOpenGLBuffer* vboMesh=nullptr, *iboMesh=nullptr;
    QOpenGLShaderProgram* shaderCloth = nullptr;
    unsigned int numFacesSphereS=0, numFacesSphereBigS = 0, numFacesCube = 0;

    IntegratorRK2 integrator;
    ParticleSystem system;
    std::list<Particle*> deadParticles;
    ForceConstAcceleration* fGravity;
    ForceBlackhole* fBlackhole;
    ForceConstAcceleration *fWind = nullptr;
    ColliderPlane colliderFloor;
    ColliderSphere colliderSphere;
    ColliderAABB colliderBoat;
    ColliderAABB colliderMast;
    ColliderAABB colliderSailwood1;
    ColliderAABB colliderSailwood2;

    double emitRate;
    double maxParticleLife;

    Vec3 fountainPos;
    Vec3 fountainPos2;
    QVector<Particle*> fountainParticles;
    QVector<Particle*> fountainParticles2;
    int mouseX, mouseY;

    Hash *hash;
    Sail *cloth;


    float deformation_rate = 0.1f;
    int relaxation_steps = 10;
    int numParticlesX=60, numParticlesY=40,numMeshIndices;

    Particle* selectedPi=nullptr;
    int select_pi_status=0;//0:available; 1:found; 2:failed;

    int curr_step=0;
    QMediaPlaylist* playlist;
    QMediaPlayer* player;

    QSet<int> keysPressed;
};

#endif // SCENEOP_H
