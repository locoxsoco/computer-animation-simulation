#ifndef SCENE_H
#define SCENE_H

#include <QWidget>
#include <QMouseEvent>
#include "camera.h"

class Scene : public QObject
{
    Q_OBJECT
public:
    Scene() {}
    virtual ~Scene() {};

    virtual void initialize(double dt, double bo, double fr, unsigned int dragt) = 0;
    virtual void reset(double dt, double bo, double fr, unsigned int dragt) = 0;
    virtual void update() = 0;
    virtual void paint(const Camera& cam) = 0;

    virtual void mousePressed (const QMouseEvent*, const Camera&) {};
    virtual void mouseMoved   (const QMouseEvent*, const Camera&) {};
    virtual void mouseReleased(const QMouseEvent*, const Camera&) {};
    virtual void keyPressed   (const QKeyEvent*,   const Camera&) {};
    virtual void keyReleased   (const QKeyEvent*,   const Camera&) {};

    virtual void getSceneBounds(Vec3& bmin, Vec3& bmax) = 0;
    virtual unsigned int getNumParticles() { return 0; }

    virtual QWidget* sceneUI() = 0;

    double timeStep, bouncing, friction;
    unsigned int dragType;
};

#endif // SCENE_H
