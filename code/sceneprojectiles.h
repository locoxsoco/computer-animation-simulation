#ifndef SCENEPROJECTILES_H
#define SCENEPROJECTILES_H

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include "scene.h"
#include "widgetprojectiles.h"
#include "particlesystem.h"
#include "integrators.h"

class SceneProjectiles : public Scene
{
    Q_OBJECT

public:
    SceneProjectiles();
    virtual ~SceneProjectiles();

    virtual void initialize(double dt, double bo, double fr, unsigned int dragt);
    virtual void reset(double dt, double bo, double fr, unsigned int dragt);
    virtual void update();
    virtual void paint(const Camera& cam);

    virtual void getSceneBounds(Vec3& bmin, Vec3& bmax) {
        bmin = Vec3(-10,  -10, -30);
        bmax = Vec3( 200, 150,  30);
    }
    virtual unsigned int getNumParticles() { return 4; }

    virtual QWidget* sceneUI() { return widget; }

protected:
    void updateTrajectoryCoordsBuffer(const std::list<Vec3>& trajectory, bool sameZ);

protected:
    WidgetProjectiles* widget = nullptr;

    QOpenGLShaderProgram* shaderPhong = nullptr;
    QOpenGLShaderProgram* shaderLines = nullptr;
    QOpenGLVertexArrayObject* vaoSphere = nullptr;
    QOpenGLVertexArrayObject* vaoFloor = nullptr;
    QOpenGLVertexArrayObject* vaoCube = nullptr;
    QOpenGLVertexArrayObject* vaoTrajectory = nullptr;
    QOpenGLBuffer* vboTrajectoryPoints = nullptr;
    unsigned int numSphereFaces = 0;
    const unsigned int MAX_TRAJ_POINTS = 1000;

    Integrator* integrator1 = nullptr;
    Integrator* integrator2 = nullptr;
    Integrator* integrator3 = nullptr;
    ParticleSystem systemAnalytic;
    ParticleSystem systemNumerical1;
    ParticleSystem systemNumerical2;
    ParticleSystem systemNumerical3;
    ForceConstAcceleration *fGravity1 = nullptr;
    ForceConstAcceleration *fGravity2 = nullptr;
    ForceConstAcceleration *fGravity3 = nullptr;
    ForceConstAcceleration *fWind1 = nullptr;
    ForceConstAcceleration *fWind2 = nullptr;
    ForceConstAcceleration *fWind3 = nullptr;
    ForceDragLinear *fDragLinear1 = nullptr;
    ForceDragLinear *fDragLinear2 = nullptr;
    ForceDragLinear *fDragLinear3 = nullptr;
    ForceDragQuadratic *fDragQuadratic1 = nullptr;
    ForceDragQuadratic *fDragQuadratic2 = nullptr;
    ForceDragQuadratic *fDragQuadratic3 = nullptr;
    bool system1active, system2active, system3active;

    std::list<Vec3> trajectoryAnalytic;
    std::list<Vec3> trajectoryNumerical1;
    std::list<Vec3> trajectoryNumerical2;
    std::list<Vec3> trajectoryNumerical3;

    double time = 0;
    double gravityAccel, windX, windY, windZ, dragX, dragY, dragZ;
    double shotHeight, shotAngle, shotSpeed;
};
#endif // SCENEPROJECTILES_H
