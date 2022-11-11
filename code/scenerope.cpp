#include "scenerope.h"
#include "glutils.h"
#include "model.h"
#include <QOpenGLFunctions_3_3_Core>


SceneRope::SceneRope() {
    widget = new WidgetRope();
    connect(widget, SIGNAL(updatedParameters()), this, SLOT(updateSimParams()));
}


SceneRope::~SceneRope() {
    if (widget)     delete widget;
    if (shader)     delete shader;
    if (vaoFloor)   delete vaoFloor;
    if (vaoSphereS) delete vaoSphereS;
    if (vaoSphereBigS) delete vaoSphereBigS;
    if (fGravity)   delete fGravity;
    if (fBlackhole) delete fBlackhole;
    if (rope) delete rope;
}


void SceneRope::initialize(double dt, double bo, double fr, unsigned int dragt) {

    timeStep = dt;
    bouncing = bo;
    friction = fr;
    dragType = dragt;

    // load shader
    shader = glutils::loadShaderProgram(":/shaders/phong.vert", ":/shaders/phong.frag");

    // create floor VAO
    Model quad = Model::createQuad();
    vaoFloor = glutils::createVAO(shader, &quad);
    glutils::checkGLError();

    // create sphere VAOs
    Model sphere = Model::createIcosphere(1);
    vaoSphereS = glutils::createVAO(shader, &sphere);
    numFacesSphereS = sphere.numFaces();
    glutils::checkGLError();

    // create big sphere VAOs
    Model sphereBig = Model::createIcosphere(5);
    vaoSphereBigS = glutils::createVAO(shader, &sphereBig);
    numFacesSphereBigS = sphereBig.numFaces();
    glutils::checkGLError();

    // create AABB cube VAOs
    Model cube = Model::createCube();
    vaoCube = glutils::createVAO(shader, &cube);
    numFacesCube = cube.numFaces();
    glutils::checkGLError();

    // create gravity force
    fGravity = new ForceConstAcceleration();
    system.addForce(fGravity);

    // create blackhole force
    fBlackhole = new ForceBlackhole(Vec3(50, 50, 50), 100);
    system.addForce(fBlackhole);

    // create rope
    rope = new Rope(20,Vec3(-20,80,-20));
    for(int i=0;i<rope->particles.length();i++){
        system.addParticle(rope->particles[i]);
        fGravity->addInfluencedParticle(rope->particles[i]);
        fBlackhole->addInfluencedParticle(rope->particles[i]);
    }
    for(int i=0;i<rope->force_springs.length();i++){
        system.addForce(rope->force_springs[i]);
    }

    // scene
    fountainPos = Vec3(0, 10, 0);
    colliderFloor.setPlane(Vec3(0, 1, 0), 0);
    colliderSphere.setSphere(Vec3(20, 0, 20), 20);
    colliderAABB.setAABB(Vec3(0, 0, 0),Vec3(15, 15, 30));

    // create spatial hashing
    //hash = new Hash(2.0,2000);

}


void SceneRope::reset(double dt, double bo, double fr, unsigned int dragt)
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
    for(int i=0;i<rope->force_springs.length();i++){
        rope->force_springs[i]->clearInfluencedParticles();
    }
    system.deleteParticles();
    system.clearForces();
    deadParticles.clear();

    //hash->create(system.getParticles());
    system.addForce(fGravity);
    system.addForce(fBlackhole);

    rope = new Rope(20,Vec3(-20,80,-20));
    for(int i=0;i<rope->particles.length();i++){
        system.addParticle(rope->particles[i]);
        fGravity->addInfluencedParticle(rope->particles[i]);
        fBlackhole->addInfluencedParticle(rope->particles[i]);
    }
    for(int i=0;i<rope->force_springs.length();i++){
        system.addForce(rope->force_springs[i]);
    }

    updateSimParams();
}


void SceneRope::updateSimParams()
{
    // get gravity from UI and update force
    double g = widget->getGravity();
    fGravity->setAcceleration(Vec3(0, -g, 0));
    fBlackhole->setIntensity(widget->getBlackholeIntensity());

    for(ForceSpring* fs : rope->force_springs){
        fs->ke = widget->getKe();
        fs->kd = widget->getKd();
    }

    // get other relevant UI values and update simulation params
    maxParticleLife = 10.0;
    emitRate = 200;
}


void SceneRope::paint(const Camera& camera) {

    QOpenGLFunctions_3_3_Core* glFuncs = nullptr;
    glFuncs = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();

    shader->bind();

    // camera matrices
    QMatrix4x4 camProj = camera.getPerspectiveMatrix();
    QMatrix4x4 camView = camera.getViewMatrix();
    shader->setUniformValue("ProjMatrix", camProj);
    shader->setUniformValue("ViewMatrix", camView);

    // lighting
    const int numLights = 1;
    const QVector3D lightPosWorld[numLights] = {QVector3D(100,500,100)};
    const QVector3D lightColor[numLights] = {QVector3D(1,1,1)};
    QVector3D lightPosCam[numLights];
    for (int i = 0; i < numLights; i++) {
        lightPosCam[i] = camView * lightPosWorld[i];
    }
    shader->setUniformValue("numLights", numLights);
    shader->setUniformValueArray("lightPos", lightPosCam, numLights);
    shader->setUniformValueArray("lightColor", lightColor, numLights);

    // draw floor
    vaoFloor->bind();
    QMatrix4x4 modelMat;
    modelMat.scale(100, 1, 100);
    shader->setUniformValue("ModelMatrix", modelMat);
    shader->setUniformValue("matdiff", 0.8f, 0.8f, 0.8f);
    shader->setUniformValue("matspec", 0.0f, 0.0f, 0.0f);
    shader->setUniformValue("matshin", 0.0f);
    shader->setUniformValue("alpha", 1.0f);
    glFuncs->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // draw sphere
    vaoSphereBigS->bind();
    modelMat = QMatrix4x4();
    modelMat.translate(colliderSphere.sphereC[0],colliderSphere.sphereC[1],colliderSphere.sphereC[2]);
    modelMat.scale(colliderSphere.sphereR);
    shader->setUniformValue("ModelMatrix", modelMat);
    shader->setUniformValue("matdiff", 0.8f, 0.8f, 0.8f);
    shader->setUniformValue("matspec", 0.0f, 0.0f, 0.0f);
    shader->setUniformValue("matshin", 0.f);
    shader->setUniformValue("alpha", 1.0f);
    glFuncs->glDrawElements(GL_TRIANGLES, 3*numFacesSphereBigS, GL_UNSIGNED_INT, 0);

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

    // draw aabb
    vaoCube->bind();
    modelMat = QMatrix4x4();
    modelMat.translate(colliderAABB.pos.x(),colliderAABB.pos.y(),colliderAABB.pos.z());
    modelMat.scale(colliderAABB.scale.x(),colliderAABB.scale.y(),colliderAABB.scale.z());
    shader->setUniformValue("ModelMatrix", modelMat);
    shader->setUniformValue("matdiff", 0.8f, 0.8f, 0.8f);
    shader->setUniformValue("matspec", 0.0f, 0.0f, 0.0f);
    shader->setUniformValue("matshin", 0.f);
    shader->setUniformValue("alpha", 1.0f);
    glFuncs->glDrawElements(GL_TRIANGLES, 3*numFacesCube, GL_UNSIGNED_INT, 0);


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

    shader->release();
}


void SceneRope::update() {
    double dt = timeStep;

    // emit new particles, reuse dead ones if possible
    /*int emitParticles = std::max(1, int(std::round(emitRate * dt)));
    for (int i = 0; i < emitParticles; i++) {
        Particle* p;
        if (!deadParticles.empty()) {
            // reuse one dead particle
            p = deadParticles.front();
            deadParticles.pop_front();
        }
        else {
            // create new particle
            p = new Particle();
            system.addParticle(p);

            // don't forget to add particle to forces that affect it
            fGravity->addInfluencedParticle(p);
            fBlackhole->addInfluencedParticle(p);
        }

        p->color = Vec3(153/255.0, 217/255.0, 234/255.0);
        p->radius = 1.0;
        p->life = maxParticleLife;

        double x = Random::get(-2, 2);
        double y = 0;
        double z = Random::get(-2, 2);
        p->pos = Vec3(0,y,0) + fountainPos;
        p->vel = Vec3(x,Random::get(28, 30),z);
        p->prevPos = p->pos - timeStep*p->vel;
    }*/

    //hash->create(system.getParticles());

    // integration step
    Vecd ppos = system.getPositions();
    integrator.step(system, dt);
    system.setPreviousPositions(ppos);

    // collisions
    for (Particle* pi : system.getParticles()) {
        float particleMinDist = 2.0 * pi->radius;
        // Floor collider
        if (colliderFloor.testCollision(pi)) {
            colliderFloor.resolveCollision(pi, bouncing, friction, dt);
        }
        // Sphere collider
        if (colliderSphere.testCollision(pi)) {
            colliderSphere.resolveCollision(pi, bouncing, friction, dt);
        }
        // AABB collider
        if (colliderAABB.testCollision(pi)) {
            colliderAABB.resolveCollision(pi, bouncing, friction, dt);
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

    // check dead particles
    /*for (Particle* p : system.getParticles()) {
        if (p->life > 0) {
            p->life -= dt;
            if (p->life < 0) {
                deadParticles.push_back(p);
            }
        }
    }*/
}

void SceneRope::mousePressed(const QMouseEvent* e, const Camera&)
{
    mouseX = e->pos().x();
    mouseY = e->pos().y();
}

void SceneRope::mouseMoved(const QMouseEvent* e, const Camera& cam)
{
    int dx = e->pos().x() - mouseX;
    int dy = e->pos().y() - mouseY;
    mouseX = e->pos().x();
    mouseY = e->pos().y();

    Vec3 disp = cam.worldSpaceDisplacement(dx, -dy, cam.getEyeDistance());

    // example
    if (e->modifiers() & Qt::ControlModifier) {
        // move fountain
        fountainPos += disp;
    }
    else {
        // do something else: e.g. move colliders
        switch(widget->getMovableObjectId()) {
        case 0:
            colliderSphere.sphereC += disp;
            break;
        case 1:
            colliderAABB.pos += disp;
            break;
        case 2:
            fBlackhole->position += disp;
            break;
        }
    }
}
