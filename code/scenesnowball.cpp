#include "SceneSnowball.h"
#include "glutils.h"
#include "model.h"
#include <QOpenGLFunctions_3_3_Core>


SceneSnowball::SceneSnowball() {
    widget = new WidgetSnowball();
    connect(widget, SIGNAL(updatedParameters()), this, SLOT(updateSimParams()));
}


SceneSnowball::~SceneSnowball() {
    if (widget)     delete widget;
    if (shader)     delete shader;
    if (vaoSphereS) delete vaoSphereS;
    if (vaoSphereBigS) delete vaoSphereBigS;
    if (fGravity)   delete fGravity;
    if (fBlackhole) delete fBlackhole;
}


void SceneSnowball::initialize(double dt, double bo, double fr, unsigned int dragt) {

    timeStep = dt;
    bouncing = bo;
    friction = fr;
    dragType = dragt;

    // load shader
    shader = glutils::loadShaderProgram(":/shaders/phong.vert", ":/shaders/phong.frag");

    // create sphere VAOs
    Model sphere = Model::createIcosphere(1);
    vaoSphereS = glutils::createVAO(shader, &sphere);
    numFacesSphereS = sphere.numFaces();
    glutils::checkGLError();

    // create big sphere VAOs
    Model sphereBig = Model::createIcosphereNI(5);
    vaoSphereBigS = glutils::createVAO(shader, &sphereBig);
    numFacesSphereBigS = sphereBig.numFaces();
    glutils::checkGLError();

    // create gravity force
    fGravity = new ForceConstAcceleration();
    system.addForce(fGravity);

    // create blackhole force
    fBlackhole = new ForceBlackhole(Vec3(50, 50, 50), 100);
    system.addForce(fBlackhole);

    // scene
    colliderSnowball.setSphere(Vec3(0, 0, 0), 50);

    // initialize particles
    //int emitParticles = std::max(1, int(std::round(emitRate * dt)));
    for (int i = 0; i < 1000; i++) {

        // create new particle
        pp = new Particle();
        system.addParticle(pp);

        // don't forget to add particle to forces that affect it
        fGravity->addInfluencedParticle(pp);
        fBlackhole->addInfluencedParticle(pp);

        pp->color = Vec3(153/255.0, 217/255.0, 234/255.0);
        pp->radius = 1.0;

        double alpha = Random::get(0,100)*2*3.1415/100.0;
        double beta = Random::get(0,100)/100.0;
        double d = Random::get(0,100)/100.0;

        beta = acos(1-2*beta)-3.1415/2;
        d=50*cbrt(d);
        pp->pos = Vec3(cos(alpha)*cos(beta)*d,sin(beta)*d,sin(alpha)*cos(beta)*d);
        pp->vel = Vec3(0,0,0);
        pp->prevPos = pp->pos;
    }

    // create spatial hashing
    //hash = new Hash(2.0,1000);

}


void SceneSnowball::reset(double dt, double bo, double fr, unsigned int dragt)
{
    timeStep = dt;
    bouncing = bo;
    friction = fr;
    dragType = dragt;

    // update values from UI
    updateSimParams();

    // reset random seed
    Random::seed(1337);

    // erase all particles
    fGravity->clearInfluencedParticles();
    fBlackhole->clearInfluencedParticles();
    system.deleteParticles();
    deadParticles.clear();

    // initialize particles
    //int emitParticles = std::max(1, int(std::round(emitRate * dt)));
    for (int i = 0; i < 1000; i++) {

        // create new particle
        pp = new Particle();
        system.addParticle(pp);

        // don't forget to add particle to forces that affect it
        fGravity->addInfluencedParticle(pp);
        fBlackhole->addInfluencedParticle(pp);

        pp->color = Vec3(153/255.0, 217/255.0, 234/255.0);
        pp->radius = 1.0;

        double alpha = Random::get(0,100)*2*3.1415/100.0;
        double beta = Random::get(0,100)/100.0;
        double d = Random::get(0,100)/100.0;

        beta = acos(1-2*beta)-3.1415/2;
        d=50*cbrt(d);
        pp->pos = Vec3(cos(alpha)*cos(beta)*d,sin(beta)*d,sin(alpha)*cos(beta)*d);
        pp->vel = Vec3(0,0,0);
        pp->prevPos = pp->pos;
    }

    //hash->create(system.getParticles());
}


void SceneSnowball::updateSimParams()
{
    // get gravity from UI and update force
    double g = widget->getGravity();
    fGravity->setAcceleration(Vec3(0, -g, 0));
    fBlackhole->setIntensity(widget->getBlackholeIntensity());

    // get other relevant UI values and update simulation params
    kBounce = 0.5;
    kFriction = 0.1;
    maxParticleLife = 10.0;
    emitRate = 100;
}


void SceneSnowball::paint(const Camera& camera) {

    QOpenGLFunctions_3_3_Core* glFuncs = nullptr;
    glFuncs = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();

    // camera matrices
    QMatrix4x4 camProj = camera.getPerspectiveMatrix();
    QMatrix4x4 camView = camera.getViewMatrix();

    // lighting
    const int numLights = 1;
    const QVector3D lightPosWorld[numLights] = {QVector3D(100,500,100)};
    const QVector3D lightColor[numLights] = {QVector3D(1,1,1)};
    QVector3D lightPosCam[numLights];
    for (int i = 0; i < numLights; i++) {
        lightPosCam[i] = camView * lightPosWorld[i];
    }


    QMatrix4x4 modelMat;
    shader->bind();
    shader->setUniformValue("ProjMatrix", camProj);
    shader->setUniformValue("ViewMatrix", camView);


    shader->setUniformValue("numLights", numLights);
    shader->setUniformValueArray("lightPos", lightPosCam, numLights);
    shader->setUniformValueArray("lightColor", lightColor, numLights);

    // draw blackhole
    vaoSphereS->bind();
    modelMat = QMatrix4x4();
    modelMat.translate(fBlackhole->getPosition().x(),fBlackhole->getPosition().y(),fBlackhole->getPosition().z());
    modelMat.scale(1);
    shader->setUniformValue("ModelMatrix", modelMat);
    shader->setUniformValue("matdiff", GLfloat(0.f), GLfloat(0.f), GLfloat(0.f));
    shader->setUniformValue("matspec", 1.0f, 1.0f, 1.0f);
    shader->setUniformValue("matshin", 100.f);
    shader->setUniformValue("alpha", 1.0f);
    glFuncs->glDrawElements(GL_TRIANGLES, 3*numFacesSphereS, GL_UNSIGNED_INT, 0);


    // draw the different spheres
    vaoSphereS->bind();
    for (const Particle* particle : system.getParticles()) {
        Vec3   p = particle->pos;
        Vec3   c = particle->color;
        double r = particle->radius;

        modelMat = QMatrix4x4();
        modelMat.translate(p[0], p[1], p[2]);
        modelMat.scale(r);
        shader->setUniformValue("ModelMatrix", modelMat);

        shader->setUniformValue("matdiff", GLfloat(c[0]), GLfloat(c[1]), GLfloat(c[2]));
        shader->setUniformValue("matspec", 1.0f, 1.0f, 1.0f);
        shader->setUniformValue("matshin", 100.f);
        shader->setUniformValue("alpha", 1.0f);

        glFuncs->glDrawElements(GL_TRIANGLES, 3*numFacesSphereS, GL_UNSIGNED_INT, 0);
    }

    // draw big sphere
    vaoSphereBigS->bind();
    modelMat = QMatrix4x4();
    modelMat.translate(colliderSnowball.sphereC[0],colliderSnowball.sphereC[1],colliderSnowball.sphereC[2]);
    modelMat.scale(colliderSnowball.sphereR);
    shader->setUniformValue("ModelMatrix", modelMat);
    shader->setUniformValue("matdiff", GLfloat(0.f), GLfloat(0.f), GLfloat(0.f));
    shader->setUniformValue("matspec", 1.0f, 1.0f, 1.0f);
    shader->setUniformValue("matshin", 100.f);
    shader->setUniformValue("alpha", 0.1f);
    glFuncs->glDrawElements(GL_TRIANGLES, 3*numFacesSphereBigS, GL_UNSIGNED_INT, 0);

    shader->release();
}


void SceneSnowball::update() {
    double dt = timeStep;

    //hash->create(system.getParticles());

    // integration step
    Vecd ppos = system.getPositions();
    integrator.step(system, dt);
    system.setPreviousPositions(ppos);

    // collisions
    for (Particle* pi : system.getParticles()) {
        float particleMinDist = 2.0 * pi->radius;
        // Sphere collider
        if (colliderSnowball.testCollision(pi)) {
            colliderSnowball.resolveCollision(pi, kBounce, kFriction, dt);
        }
        // Spatial Hashing collider
        /*hash->query(system.getParticles(),pi->id,2.0 * pi->radius);

        for(unsigned int nr=0; nr<hash->querySize;nr++){
            Particle *pj = system.getParticles()[hash->queryIds[nr]];
            Vecd normal = pi->pos - pj->pos;
            double d = (normal).norm();
            double d2 = d*d;

            if(d2 > 0.f && d2 < particleMinDist*particleMinDist) {
                normal = normal/d;

                double corr = (particleMinDist - d) * 0.5;

                pi->pos += normal*corr;
                pj->pos -= normal*corr;

                double vi = pi->vel.dot(normal);
                double vj = pj->vel.dot(normal);

                pi->vel += normal*(vj-vi);
                pj->vel += normal*(vi-vj);

            }
        }*/
    }
}

void SceneSnowball::mousePressed(const QMouseEvent* e, const Camera&)
{
    mouseX = e->pos().x();
    mouseY = e->pos().y();
}

void SceneSnowball::mouseMoved(const QMouseEvent* e, const Camera& cam)
{
    int dx = e->pos().x() - mouseX;
    int dy = e->pos().y() - mouseY;
    mouseX = e->pos().x();
    mouseY = e->pos().y();

    Vec3 disp = cam.worldSpaceDisplacement(dx, -dy, cam.getEyeDistance());

    // example
    if (e->modifiers() & Qt::ControlModifier) {
        // move fountain
        //fountainPos += disp;
    }
    else {
        // do something else: e.g. move colliders
        switch(widget->getMovableObjectId()) {
        case 0:
            colliderSnowball.sphereC += disp;
            for (Particle* pi : system.getParticles()) {
                pi->prevPos = pi->pos;
                pi->pos += disp;
                pi->vel += disp;
            }
            break;
        case 1:
            fBlackhole->position += disp;
            break;
        }
    }
}
