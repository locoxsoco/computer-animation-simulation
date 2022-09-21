#include "sceneprojectiles.h"
#include "glutils.h"
#include "model.h"
#include <QMatrix4x4>
#include <iostream>


SceneProjectiles::SceneProjectiles() {
    widget = new WidgetProjectiles();

    const std::vector<std::string> solvers = {
        "Euler", "Symplectic Euler", "Midpoint", "Verlet", "RK2", "RK4"
    };
    widget->setSolverTypes(solvers);
    widget->setSolver1(0); // Euler
    widget->setSolver2(1); // Symplectic Euler
    widget->setSolver3(2); // Midpoint
}

SceneProjectiles::~SceneProjectiles() {
    if (widget)      delete widget;
    if (shaderPhong) delete shaderPhong;
    if (shaderLines) delete shaderLines;
    if (vaoSphere)   delete vaoSphere;
    if (vaoFloor)    delete vaoFloor;
    if (vaoCube)     delete vaoCube;
    if (vaoTrajectory) delete vaoTrajectory;
    if (vboTrajectoryPoints) delete vboTrajectoryPoints;
    if (integrator1) delete integrator1;
    if (integrator2) delete integrator2;
    if (integrator3) delete integrator3;
    systemAnalytic.deleteParticles();
    systemAnalytic.deleteForces();
    systemNumerical1.deleteParticles();
    systemNumerical1.deleteForces();
    systemNumerical2.deleteParticles();
    systemNumerical2.deleteForces();
    systemNumerical3.deleteParticles();
    systemNumerical3.deleteForces();
}

void SceneProjectiles::initialize(double dt, double bo, double fr, unsigned int dragt) {

    timeStep = dt;
    bouncing = bo;
    friction = fr;
    dragType = dragt;

    /*
     * RENDER INITS
     */

    // load shader programs
    shaderPhong = glutils::loadShaderProgram(":/shaders/phong.vert", ":/shaders/phong.frag");
    shaderLines = glutils::loadShaderProgram(":/shaders/lines.vert", ":/shaders/lines.geom", ":/shaders/lines.frag");

    // create sphere VAO
    Model sphere = Model::createIcosphere(3);
    vaoSphere = glutils::createVAO(shaderPhong, &sphere);
    numSphereFaces = sphere.numFaces();
    glutils::checkGLError();

    // create floor VAO
    Model quad = Model::createQuad();
    vaoFloor = glutils::createVAO(shaderPhong, &quad);
    glutils::checkGLError();

    // create cube VAO
    Model cube = Model::createCube();
    vaoCube = glutils::createVAO(shaderPhong, &cube);
    glutils::checkGLError();

    // initialize the buffer we'll use to render trajectories
    vaoTrajectory = new QOpenGLVertexArrayObject();
    vaoTrajectory->create();
    vaoTrajectory->bind();
    vboTrajectoryPoints = new QOpenGLBuffer(QOpenGLBuffer::Type::VertexBuffer);
    vboTrajectoryPoints->create();
    vboTrajectoryPoints->bind();
    vboTrajectoryPoints->setUsagePattern(QOpenGLBuffer::UsagePattern::DynamicDraw);
    vboTrajectoryPoints->allocate(MAX_TRAJ_POINTS*3*sizeof(float));
    shaderLines->setAttributeBuffer("vertex", GL_FLOAT, 0, 3, 0);
    shaderLines->enableAttributeArray("vertex");
    vaoTrajectory->release();


    /*
     * PHYSICS INITS
     */

    // create the different systems, with one particle each
    systemAnalytic.addParticle(new Particle(Vec3( 0, 0, -15), Vec3(0,0,0), 1, timeStep));
    systemAnalytic.getParticle(0)->color = Vec3(1.0, 1.0, 1.0);
    systemAnalytic.getParticle(0)->radius = 2;
    systemNumerical1.addParticle(new Particle(Vec3( 0, 0, -5), Vec3(0,0,0), 1, timeStep));
    systemNumerical1.getParticle(0)->color = Vec3(0.5, 0, 0);
    systemNumerical1.getParticle(0)->radius = 2;
    systemNumerical2.addParticle(new Particle(Vec3( 0, 0, 5), Vec3(0,0,0), 1, timeStep));
    systemNumerical2.getParticle(0)->color = Vec3(0, 0.5, 0);
    systemNumerical2.getParticle(0)->radius = 2;
    systemNumerical3.addParticle(new Particle(Vec3( 0, 0, 15), Vec3(0,0,0), 1, timeStep));
    systemNumerical3.getParticle(0)->color = Vec3(0, 0, 0.5);
    systemNumerical3.getParticle(0)->radius = 2;

    // only one force: gravity, but we need to create one per system to assign its particle
    fGravity1 = new ForceConstAcceleration(Vec3(0, -gravityAccel, 0));
    fGravity1->addInfluencedParticle(systemNumerical1.getParticle(0));
    systemNumerical1.addForce(fGravity1);

    fGravity2 = new ForceConstAcceleration(Vec3(0, -gravityAccel, 0));
    fGravity2->addInfluencedParticle(systemNumerical2.getParticle(0));
    systemNumerical2.addForce(fGravity2);

    fGravity3 = new ForceConstAcceleration(Vec3(0, -gravityAccel, 0));
    fGravity3->addInfluencedParticle(systemNumerical3.getParticle(0));
    systemNumerical3.addForce(fGravity3);

    // Adding wind force
    fWind1 = new ForceConstAcceleration(Vec3(windX, windY, windZ));
    fWind1->addInfluencedParticle(systemNumerical1.getParticle(0));
    systemNumerical1.addForce(fWind1);

    fWind2 = new ForceConstAcceleration(Vec3(windX, windY, windZ));
    fWind2->addInfluencedParticle(systemNumerical2.getParticle(0));
    systemNumerical2.addForce(fWind2);

    fWind3 = new ForceConstAcceleration(Vec3(windX, windY, windZ));
    fWind3->addInfluencedParticle(systemNumerical3.getParticle(0));
    systemNumerical3.addForce(fWind3);

    // Adding drag force
    if(dragType == 1) {
        fDragLinear1 = new ForceDragLinear(-0.000015 * systemNumerical1.getParticle(0)->vel);
        fDragLinear1->addInfluencedParticle(systemNumerical1.getParticle(0));
        systemNumerical1.addForce(fDragLinear1);

        fDragLinear2 = new ForceDragLinear(-0.000015 * systemNumerical2.getParticle(0)->vel);
        fDragLinear2->addInfluencedParticle(systemNumerical2.getParticle(0));
        systemNumerical2.addForce(fDragLinear2);

        fDragLinear3 = new ForceDragLinear(-0.000015 * systemNumerical3.getParticle(0)->vel);
        fDragLinear3->addInfluencedParticle(systemNumerical3.getParticle(0));
        systemNumerical3.addForce(fDragLinear3);
    }
    else if (dragType == 2) {
        fDragQuadratic1 = new ForceDragQuadratic(-0.000015 * systemNumerical1.getParticle(0)->vel.norm() * systemNumerical1.getParticle(0)->vel);
        fDragQuadratic1->addInfluencedParticle(systemNumerical1.getParticle(0));
        systemNumerical1.addForce(fDragQuadratic1);

        fDragQuadratic2 = new ForceDragQuadratic(-0.000015 * systemNumerical2.getParticle(0)->vel.norm() * systemNumerical2.getParticle(0)->vel);
        fDragQuadratic2->addInfluencedParticle(systemNumerical2.getParticle(0));
        systemNumerical2.addForce(fDragQuadratic2);

        fDragQuadratic3 = new ForceDragQuadratic(-0.000015 * systemNumerical3.getParticle(0)->vel.norm() * systemNumerical3.getParticle(0)->vel);
        fDragQuadratic3->addInfluencedParticle(systemNumerical3.getParticle(0));
        systemNumerical3.addForce(fDragQuadratic3);
    }
}


Integrator* createIntegrator(int type) {
    switch(type) {
        case 0: return new IntegratorEuler();
        case 1: return new IntegratorSymplecticEuler();
        case 2: return new IntegratorMidpoint();
        case 3: return new IntegratorVerlet();
        case 4: return new IntegratorRK2();
        case 5: return new IntegratorRK4();
        default: return nullptr;
    }
}


void SceneProjectiles::reset(double dt, double bo, double fr, unsigned int dragt) {

    timeStep = dt;
    bouncing = bo;
    friction = fr;
    dragType = dragt;

    // get new interface values
    shotHeight   = widget->getHeight();
    shotAngle    = Math::toRad(widget->getAngle());
    shotSpeed    = widget->getSpeed();
    gravityAccel = widget->getGravity();
    windX        = widget->getWindX();
    windY        = widget->getWindY();
    windZ        = widget->getWindZ();

    // integrators
    if (integrator1) delete integrator1;
    if (integrator2) delete integrator2;
    if (integrator3) delete integrator3;
    integrator1 = createIntegrator(widget->getSolver1());
    integrator2 = createIntegrator(widget->getSolver2());
    integrator3 = createIntegrator(widget->getSolver3());

    // update initial particle positions
    const double zdist = 15;
    systemAnalytic.getParticle(0)->pos = Vec3(0, shotHeight, -zdist);
    systemAnalytic.getParticle(0)->vel = shotSpeed*Vec3(std::cos(shotAngle), std::sin(shotAngle), 0);
    systemAnalytic.getParticle(0)->prevPos = systemAnalytic.getParticle(0)->pos - timeStep*systemAnalytic.getParticle(0)->vel;
    systemNumerical1.getParticle(0)->pos = Vec3(0, shotHeight, -zdist/3);
    systemNumerical1.getParticle(0)->vel = shotSpeed*Vec3(std::cos(shotAngle), std::sin(shotAngle), 0);
    systemNumerical1.getParticle(0)->prevPos = systemNumerical1.getParticle(0)->pos - timeStep*systemNumerical1.getParticle(0)->vel;;
    systemNumerical2.getParticle(0)->pos = Vec3(0, shotHeight, zdist/3);
    systemNumerical2.getParticle(0)->vel = shotSpeed*Vec3(std::cos(shotAngle), std::sin(shotAngle), 0);
    systemNumerical2.getParticle(0)->prevPos = systemNumerical2.getParticle(0)->pos - timeStep*systemNumerical2.getParticle(0)->vel;
    systemNumerical3.getParticle(0)->pos = Vec3(0, shotHeight, zdist);
    systemNumerical3.getParticle(0)->vel = shotSpeed*Vec3(std::cos(shotAngle), std::sin(shotAngle), 0);
    systemNumerical3.getParticle(0)->prevPos = systemNumerical3.getParticle(0)->pos - timeStep*systemNumerical3.getParticle(0)->vel;

    //Clear forces
    systemNumerical1.clearForces();
    systemNumerical2.clearForces();
    systemNumerical3.clearForces();

    // update gravity accelerations
    fGravity1 = new ForceConstAcceleration(Vec3(0, -gravityAccel, 0));
    fGravity1->addInfluencedParticle(systemNumerical1.getParticle(0));
    systemNumerical1.addForce(fGravity1);

    fGravity2 = new ForceConstAcceleration(Vec3(0, -gravityAccel, 0));
    fGravity2->addInfluencedParticle(systemNumerical2.getParticle(0));
    systemNumerical2.addForce(fGravity2);

    fGravity3 = new ForceConstAcceleration(Vec3(0, -gravityAccel, 0));
    fGravity3->addInfluencedParticle(systemNumerical3.getParticle(0));
    systemNumerical3.addForce(fGravity3);

    // update wind accelerations
    fWind1 = new ForceConstAcceleration(Vec3(windX, windY, windZ));
    fWind1->addInfluencedParticle(systemNumerical1.getParticle(0));
    systemNumerical1.addForce(fWind1);

    fWind2 = new ForceConstAcceleration(Vec3(windX, windY, windZ));
    fWind2->addInfluencedParticle(systemNumerical2.getParticle(0));
    systemNumerical2.addForce(fWind2);

    fWind3 = new ForceConstAcceleration(Vec3(windX, windY, windZ));
    fWind3->addInfluencedParticle(systemNumerical3.getParticle(0));
    systemNumerical3.addForce(fWind3);

    // Adding drag force
    if(dragType == 1) {
        fDragLinear1 = new ForceDragLinear(-0.000015 * systemNumerical1.getParticle(0)->vel);
        fDragLinear1->addInfluencedParticle(systemNumerical1.getParticle(0));
        systemNumerical1.addForce(fDragLinear1);

        fDragLinear2 = new ForceDragLinear(-0.000015 * systemNumerical2.getParticle(0)->vel);
        fDragLinear2->addInfluencedParticle(systemNumerical2.getParticle(0));
        systemNumerical2.addForce(fDragLinear2);

        fDragLinear3 = new ForceDragLinear(-0.000015 * systemNumerical3.getParticle(0)->vel);
        fDragLinear3->addInfluencedParticle(systemNumerical3.getParticle(0));
        systemNumerical3.addForce(fDragLinear3);
    }
    else if (dragType == 2) {
        fDragQuadratic1 = new ForceDragQuadratic(-0.000015 * systemNumerical1.getParticle(0)->vel.norm() * systemNumerical1.getParticle(0)->vel);
        fDragQuadratic1->addInfluencedParticle(systemNumerical1.getParticle(0));
        systemNumerical1.addForce(fDragQuadratic1);

        fDragQuadratic2 = new ForceDragQuadratic(-0.000015 * systemNumerical2.getParticle(0)->vel.norm() * systemNumerical2.getParticle(0)->vel);
        fDragQuadratic2->addInfluencedParticle(systemNumerical2.getParticle(0));
        systemNumerical2.addForce(fDragQuadratic2);

        fDragQuadratic3 = new ForceDragQuadratic(-0.000015 * systemNumerical3.getParticle(0)->vel.norm() * systemNumerical3.getParticle(0)->vel);
        fDragQuadratic3->addInfluencedParticle(systemNumerical3.getParticle(0));
        systemNumerical3.addForce(fDragQuadratic3);
    }

    // update system forces
    systemNumerical1.updateForces();
    systemNumerical2.updateForces();
    systemNumerical3.updateForces();

    // trajectories
    trajectoryAnalytic.clear();
    trajectoryAnalytic.push_back(systemAnalytic.getParticle(0)->pos);
    trajectoryNumerical1.clear();
    trajectoryNumerical1.push_back(systemNumerical1.getParticle(0)->pos);
    trajectoryNumerical2.clear();
    trajectoryNumerical2.push_back(systemNumerical2.getParticle(0)->pos);
    trajectoryNumerical3.clear();
    trajectoryNumerical3.push_back(systemNumerical3.getParticle(0)->pos);

    // put particles to run
    system1active = true;
    system2active = true;
    system3active = true;

    // reset timer
    time = 0;
}


void SceneProjectiles::update() {
    double dt = timeStep;

    // total ellapsed time (needed for analytic solution)
    time += dt;

    // ANALYTIC: projectile motion equations until we reach the ground
    Particle* p = systemAnalytic.getParticle(0);
    double vy0 = shotSpeed*std::sin(shotAngle);
    double tGround = (vy0 + std::sqrt(vy0*vy0 + 2*gravityAccel*shotHeight))/gravityAccel;
    if (time - dt <= tGround) {
        double t = std::min(time, tGround);
        p->pos[0] = t * shotSpeed * std::cos(shotAngle);
        p->pos[1] = shotHeight + t*vy0 - 0.5*gravityAccel*t*t;
        p->vel    = Vec3(shotSpeed*std::cos(shotAngle),
                         shotSpeed*std::sin(shotAngle) - gravityAccel*t, 0);

        trajectoryAnalytic.push_back(p->pos);
        if (trajectoryAnalytic.size() > MAX_TRAJ_POINTS) trajectoryAnalytic.pop_front();
    }


    // NUMERICAL INTEGRATORS:

    if (system1active) {
        // integration step
        integrator1->step(systemNumerical1, dt);

        // collision test
        Particle* p = systemNumerical1.getParticle(0);
        if (p->pos.y() < 0) {
            // resolve
            // TODO
            p->pos.y() = p->pos.y() - (1+bouncing)*p->pos.y();
            p->vel.y() = p->vel.y() - (1+bouncing)*p->vel.y();
            //p->force.y() = 0;
            p->vel.x() = p->vel.x() - (friction)*p->vel.x();
            p->vel.z() = p->vel.z() - (friction)*p->vel.z();
            p->prevPos.x() = p->pos.x() - p->vel.x()*dt;
            p->prevPos.y() = p->pos.y() - p->vel.y()*dt;

            // stop sim for this system
            //system1active = false;
        }

        // record trajectory
        trajectoryNumerical1.push_back(p->pos);
        if (trajectoryNumerical1.size() > MAX_TRAJ_POINTS) {
            trajectoryNumerical1.pop_front();
        }
    }

    if (system2active) {
        // integration step
        integrator2->step(systemNumerical2, dt);

        // collision test
        Particle* p = systemNumerical2.getParticle(0);
        if (p->pos.y() < 0) {
            // resolve
            // TODO
            p->pos.y() = p->pos.y() - (1+bouncing)*p->pos.y();
            p->vel.y() = p->vel.y() - (1+bouncing)*p->vel.y();
            //p->force.y() = 0;
            p->vel.x() = p->vel.x() - (friction)*p->vel.x();
            p->vel.z() = p->vel.z() - (friction)*p->vel.z();
            p->prevPos.x() = p->pos.x() - p->vel.x()*dt;
            p->prevPos.y() = p->pos.y() - p->vel.y()*dt;

            // stop sim for this system
            //system2active = false;
        }

        // record trajectory
        trajectoryNumerical2.push_back(p->pos);
        if (trajectoryNumerical2.size() > MAX_TRAJ_POINTS) {
            trajectoryNumerical2.pop_front();
        }
    }

    if (system3active) {
        // integration step
        integrator3->step(systemNumerical3, dt);

        // collision test
        Particle* p = systemNumerical3.getParticle(0);
        if (p->pos.y() < 0) {
            // resolve
            // TODO
            p->pos.y() = p->pos.y() - (1+bouncing)*p->pos.y();
            p->vel.y() = p->vel.y() - (1+bouncing)*p->vel.y();
            //p->force.y() = 0;
            p->vel.x() = p->vel.x() - (friction)*p->vel.x();
            p->vel.z() = p->vel.z() - (friction)*p->vel.z();
            p->prevPos.x() = p->pos.x() - p->vel.x()*dt;
            p->prevPos.y() = p->pos.y() - p->vel.y()*dt;

            // stop sim for this system
            //system3active = false;
        }

        // record trajectory
        trajectoryNumerical3.push_back(p->pos);
        if (trajectoryNumerical3.size() > MAX_TRAJ_POINTS) {
            trajectoryNumerical3.pop_front();
        }
    }


    // print particle heights
    std::cout << "time: " << time << std::endl;
    std::cout << "analytic sol: " << systemAnalytic.getParticle(0)->pos[1] << std::endl;
    std::cout << "numerical 1: " << systemNumerical1.getParticle(0)->pos[1] << std::endl;
    std::cout << "numerical 2: " << systemNumerical2.getParticle(0)->pos[1] << std::endl;
    std::cout << "numerical 3: " << systemNumerical3.getParticle(0)->pos[1] << std::endl;
    std::cout << std::endl;
}


void SceneProjectiles::paint(const Camera& camera) {

    // pointer to current context OpenGL functions
    QOpenGLFunctions *glFuncs = QOpenGLContext::currentContext()->functions();

    // start using phong shader
    shaderPhong->bind();

    // camera matrices
    QMatrix4x4 camProj = camera.getPerspectiveMatrix();
    QMatrix4x4 camView = camera.getViewMatrix();
    shaderPhong->setUniformValue("ProjMatrix", camProj);
    shaderPhong->setUniformValue("ViewMatrix", camView);

    // lighting
    const int numLights = 2;
    const QVector3D lightPosWorld[numLights] = {QVector3D(1000,1000,1000), QVector3D(-1000,1000,-1000)};
    const QVector3D lightColor[numLights] = {QVector3D(1,1,1), QVector3D(1,1,1)};
    QVector3D lightPosCam[numLights];
    for (int i = 0; i < numLights; i++) {
        lightPosCam[i] = camView * lightPosWorld[i];
    }
    shaderPhong->setUniformValue("numLights", numLights);
    shaderPhong->setUniformValueArray("lightPos", lightPosCam, numLights);
    shaderPhong->setUniformValueArray("lightColor", lightColor, numLights);

    // draw floor
    vaoFloor->bind();
    QMatrix4x4 modelMat;
    modelMat.translate(150, 0, 0);
    modelMat.scale(200, 1, 50);
    shaderPhong->setUniformValue("ModelMatrix", modelMat);
    shaderPhong->setUniformValue("matdiff", 0.7f, 0.7f, 0.7f);
    shaderPhong->setUniformValue("matspec", 0.0f, 0.0f, 0.0f);
    shaderPhong->setUniformValue("matshin", 0.0f);
    glFuncs->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // draw cube
    vaoCube->bind();
    modelMat = QMatrix4x4();
    modelMat.translate(-1, 0.5*shotHeight - 1, 0);
    modelMat.scale(2, 0.5*shotHeight - 1, 25.0f);
    shaderPhong->setUniformValue("ModelMatrix", modelMat);
    glFuncs->glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // draw the different spheres
    vaoSphere->bind();
    const Particle* particles[4] = { systemAnalytic.getParticle(0),
                                     systemNumerical1.getParticle(0),
                                     systemNumerical2.getParticle(0),
                                     systemNumerical3.getParticle(0)};
    for (const Particle* particle : particles) {
        Vec3   p = particle->pos;
        Vec3   c = particle->color;
        double r = particle->radius;

        modelMat = QMatrix4x4();
        modelMat.translate(p[0], p[1], widget->renderSameZ() ? 0 : p[2]);
        modelMat.scale(r);
        shaderPhong->setUniformValue("ModelMatrix", modelMat);

        shaderPhong->setUniformValue("matdiff", GLfloat(c[0]), GLfloat(c[1]), GLfloat(c[2]));
        shaderPhong->setUniformValue("matspec", 1.0f, 1.0f, 1.0f);
        shaderPhong->setUniformValue("matshin", 100.f);

        glFuncs->glDrawElements(GL_TRIANGLES, 3*numSphereFaces, GL_UNSIGNED_INT, 0);
    }
    vaoSphere->release();

    // we're done with this shader
    shaderPhong->release();

    // do we need to draw trajectories?
    if (widget->renderTrajectory()) {

        shaderLines->bind();
        shaderLines->setUniformValue("ProjMatrix", camProj);
        shaderLines->setUniformValue("ViewMatrix", camView);
        shaderLines->setUniformValue("radius", float(0.25));
        shaderLines->setUniformValue("shading", false);

        updateTrajectoryCoordsBuffer(trajectoryAnalytic, widget->renderSameZ());
        Vec3 c = systemAnalytic.getParticle(0)->color;
        shaderLines->setUniformValue("matdiff", GLfloat(c[0]), GLfloat(c[1]), GLfloat(c[2]));
        vaoTrajectory->bind();
        glFuncs->glDrawArrays(GL_LINE_STRIP, 0, std::min(static_cast<unsigned int>(trajectoryAnalytic.size()),
                                                         MAX_TRAJ_POINTS));
        vaoTrajectory->release();

        updateTrajectoryCoordsBuffer(trajectoryNumerical1, widget->renderSameZ());
        c = systemNumerical1.getParticle(0)->color;
        shaderLines->setUniformValue("matdiff", GLfloat(c[0]), GLfloat(c[1]), GLfloat(c[2]));
        vaoTrajectory->bind();
        glFuncs->glDrawArrays(GL_LINE_STRIP, 0, std::min(static_cast<unsigned int>(trajectoryNumerical1.size()),
                                                         MAX_TRAJ_POINTS));
        vaoTrajectory->release();

        updateTrajectoryCoordsBuffer(trajectoryNumerical2, widget->renderSameZ());
        c = systemNumerical2.getParticle(0)->color;
        shaderLines->setUniformValue("matdiff", GLfloat(c[0]), GLfloat(c[1]), GLfloat(c[2]));
        vaoTrajectory->bind();
        glFuncs->glDrawArrays(GL_LINE_STRIP, 0, std::min(static_cast<unsigned int>(trajectoryNumerical2.size()),
                                                         MAX_TRAJ_POINTS));
        vaoTrajectory->release();

        updateTrajectoryCoordsBuffer(trajectoryNumerical3, widget->renderSameZ());
        c = systemNumerical3.getParticle(0)->color;
        shaderLines->setUniformValue("matdiff", GLfloat(c[0]), GLfloat(c[1]), GLfloat(c[2]));
        vaoTrajectory->bind();
        glFuncs->glDrawArrays(GL_LINE_STRIP, 0, std::min(static_cast<unsigned int>(trajectoryNumerical3.size()),
                                                         MAX_TRAJ_POINTS));
        vaoTrajectory->release();

        shaderLines->release();
    }
}


void SceneProjectiles::updateTrajectoryCoordsBuffer(const std::list<Vec3>& trajectory, bool sameZ) {
    vboTrajectoryPoints->bind();
    //float* pos = new float [3*trajectory.size()];
    //float pos[3*trajectory.size()];
    float pos[3*1000];
    unsigned int i = 0;
    for (auto it = trajectory.begin(); it != trajectory.end(); it++) {
        pos[3*i  ] = it->x();
        pos[3*i+1] = it->y();
        pos[3*i+2] = sameZ ? 0 : it->z();
        i++;
    }
    void* bufptr = vboTrajectoryPoints->mapRange(0, sizeof(pos), QOpenGLBuffer::RangeInvalidateBuffer | QOpenGLBuffer::RangeWrite);
    memcpy(bufptr, (void*)(pos), sizeof(pos));
    //void* bufptr = vboTrajectoryPoints->mapRange(0, sizeof(pos)*3*trajectory.size(), QOpenGLBuffer::RangeInvalidateBuffer | QOpenGLBuffer::RangeWrite);
    //memcpy(bufptr, (void*)(pos), sizeof(pos)*3*trajectory.size());
    vboTrajectoryPoints->unmap();
    vboTrajectoryPoints->release();
}
