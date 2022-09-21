#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QElapsedTimer>
#include "camera.h"
#include "scene.h"

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    GLWidget(QWidget* = nullptr);
    ~GLWidget();

    void setScene(Scene* sc);
    Scene* getScene() const { return scene; }

public slots:
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void keyPressEvent(QKeyEvent *event);

    void doSimStep();
    void doSimLoop();
    void pauseSim();
    void resetSim();
    void setTimeStep(double t) { if(scene) scene->timeStep = t; timeStep = t; }
    void setBouncing(double b) { if(scene) scene->bouncing = b; bouncing = b; }
    void setFriction(double f) { if(scene) scene->friction = f; friction = f; }
    void setNoDrag() { dragType = 0; };
    void setLinearDrag() { dragType = 1; };
    void setQuadraticDrag() { dragType = 2; };

    void resetCamera();
    void cameraViewX();
    void cameraViewY();
    void cameraViewZ();

protected:
    virtual void initializeGL();
    virtual void resizeGL(int, int);
    virtual void paintGL();

    void paintTextOverlay();
    void updateFOV();

    virtual void simStep();

protected:

    // Simulation
    bool runningSim = false;
    double timeStep = 0.05;
    double bouncing = 0.5;
    double friction = 0.7;
    int    simSteps = 0;
    double simTime = 0;
    double simPerf = 0;
    double simMs = 0;

    // Scene
    Scene* scene;
    double sceneRad = 0;

    // Camera
    Camera camera;
    double halfFOV;
    int x0 = 0, y0 = 0;

    // Performance timer
    QElapsedTimer timer;

    unsigned int dragType = 0;
};



#endif // GLWIDGET_H
