#include "scenesph_watercube.h"
#include "glutils.h"
#include "model.h"
#include <QOpenGLFunctions_3_3_Core>


SceneSPHWaterCube::SceneSPHWaterCube() {
    widget = new WidgetOP();
    connect(widget, SIGNAL(updatedParameters()), this, SLOT(updateSimParams()));
}


SceneSPHWaterCube::~SceneSPHWaterCube() {
    if (widget)     delete widget;
    if (shader)     delete shader;
    if (vaoFloor)   delete vaoFloor;
    if (vaoSphereS) delete vaoSphereS;
    if (vaoSphereBigS) delete vaoSphereBigS;
    if (fGravity)   delete fGravity;
    if (fBlackhole) delete fBlackhole;
    if (fSPHSystem.size()) fSPHSystem.clear();
}


void SceneSPHWaterCube::initialize(double dt, double bo, double fr, unsigned int dragt) {

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

    // Adding wind force
    fWind = new ForceConstAcceleration(Vec3(0.f, 0.f, 0.f));
    system.addForce(fWind);

    // scene
    colliderFloor.setPlane(Vec3(0, 1, 0), 50);
    colliderSphere.setSphere(Vec3(100, 100, 100), 20);
    colliderCube.setAABB(Vec3(0, 5, 0),Vec3(18, 24, 18));
    // create pool particles
    for(unsigned int i=0;i<poolSize.y();i++)
        for(unsigned int k=0;k<poolSize.z();k++)
            for(unsigned int j=0;j<poolSize.x();j++){
                Vec3 pos=Vec3(
                            colliderCube.pos.x()-colliderCube.scale.x(),
                            colliderCube.pos.y()-colliderCube.scale.y(),
                            colliderCube.pos.z()-colliderCube.scale.z()
                            ) +
                        Vec3(
                            j*2,
                            i*2,
                            k*2
                            ) +
                        Vec3(
                            1.f,
                            1.f,
                            1.f
                            );
                Particle *p =new Particle(pos);
                p->color = Vec3(153/255.0, 217/255.0, 234/255.0);
                p->mass = 0.01;
                poolParticles.push_back(p);
                system.addParticle(p);

                // don't forget to add particle to forces that affect it
                fGravity->addInfluencedParticle(p);
                fBlackhole->addInfluencedParticle(p);
                // Adding SPH forces
                ForceConstAcceleration* fSPH = new ForceConstAcceleration();
                fSPH->addInfluencedParticle(p);
                fSPHSystem.push_back(fSPH);
                system.addForce(fSPH);
            }

    // create drop particles
    Vec3 dropCenter = Vec3(
                colliderCube.pos.x(),
                colliderCube.pos.y()+colliderCube.scale.y()-dropSize.y()/2.f,
                colliderCube.pos.z()
                );
    for(int i=-dropSize.y()/2;i<dropSize.y()/2;i++)
        for(int k=-dropSize.z()/2;k<dropSize.z()/2;k++)
            for(int j=-dropSize.x()/2;j<dropSize.x()/2;j++){
                Vec3 pos=dropCenter +
                        Vec3(
                            j*2,
                            i*2,
                            k*2
                            );
                if(Vec3(j*2,i*2,k*2).norm()*2.f<=dropSize.x()){
                    Particle *p =new Particle(pos);
                    p->color = Vec3(153/255.0, 217/255.0, 234/255.0);
                    p->mass = 0.01;
                    dropParticles.push_back(p);
                    system.addParticle(p);
                    // don't forget to add particle to forces that affect it
                    fGravity->addInfluencedParticle(p);
                    fBlackhole->addInfluencedParticle(p);
                    // Adding SPH forces
                    ForceConstAcceleration* fSPH = new ForceConstAcceleration();
                    fSPH->addInfluencedParticle(p);
                    fSPHSystem.push_back(fSPH);
                    system.addForce(fSPH);
                }
            }

    // create spatial hashing
    hash = new Hash(2.f,system.getNumParticles());

}


void SceneSPHWaterCube::reset(double dt, double bo, double fr, unsigned int dragt)
{
    timeStep = dt;
    bouncing = bo;
    friction = fr;
    dragType = dragt;

    colliderFloor.setPlane(Vec3(0, 1, 0), 50);
    colliderSphere.setSphere(Vec3(100, 100, 100), 20);
    colliderCube.setAABB(Vec3(0, 5, 0),Vec3(18, 24, 18));

    // update values from UI
    updateSimParams();

    // reset random seed
    Random::seed(1337);

    // erase all particles
    fGravity->clearInfluencedParticles();
    fBlackhole->clearInfluencedParticles();
    fWind->clearInfluencedParticles();
    for(unsigned int i=0;i<fSPHSystem.size();i++)
        fSPHSystem[i]->clearInfluencedParticles();
    fSPHSystem.clear();

    system.deleteParticles();
    system.clearForces();
    deadParticles.clear();
    poolParticles.clear();
    dropParticles.clear();

    // create pool particles
    for(unsigned int i=0;i<poolSize.y();i++)
        for(unsigned int k=0;k<poolSize.z();k++)
            for(unsigned int j=0;j<poolSize.x();j++){
                Vec3 pos=Vec3(
                            colliderCube.pos.x()-colliderCube.scale.x(),
                            colliderCube.pos.y()-colliderCube.scale.y(),
                            colliderCube.pos.z()-colliderCube.scale.z()
                            ) +
                        Vec3(
                            j*2,
                            i*2,
                            k*2
                            ) +
                        Vec3(
                            1.f,
                            1.f,
                            1.f
                            );
                Particle *p =new Particle(pos);
                p->color = Vec3(153/255.0, 217/255.0, 234/255.0);
                p->mass = 0.01;
                poolParticles.push_back(p);
                system.addParticle(p);

                // don't forget to add particle to forces that affect it
                fGravity->addInfluencedParticle(p);
                fBlackhole->addInfluencedParticle(p);
                // Adding SPH forces
                ForceConstAcceleration* fSPH = new ForceConstAcceleration();
                fSPH->addInfluencedParticle(p);
                fSPHSystem.push_back(fSPH);
                system.addForce(fSPH);
            }

    // create drop particles
    Vec3 dropCenter = Vec3(
                colliderCube.pos.x(),
                colliderCube.pos.y()+colliderCube.scale.y()-dropSize.y()/2.f,
                colliderCube.pos.z()
                );
    for(int i=-dropSize.y()/2;i<dropSize.y()/2;i++)
        for(int k=-dropSize.z()/2;k<dropSize.z()/2;k++)
            for(int j=-dropSize.x()/2;j<dropSize.x()/2;j++){
                Vec3 pos=dropCenter +
                        Vec3(
                            j*2,
                            i*2,
                            k*2
                            );
                if(Vec3(j*2,i*2,k*2).norm()*2.f<=dropSize.x()){
                    Particle *p =new Particle(pos);
                    p->color = Vec3(153/255.0, 217/255.0, 234/255.0);
                    p->mass = 0.01;
                    dropParticles.push_back(p);
                    system.addParticle(p);

                    // don't forget to add particle to forces that affect it
                    fGravity->addInfluencedParticle(p);
                    fBlackhole->addInfluencedParticle(p);
                    // Adding SPH forces
                    ForceConstAcceleration* fSPH = new ForceConstAcceleration();
                    fSPH->addInfluencedParticle(p);
                    fSPHSystem.push_back(fSPH);
                    system.addForce(fSPH);
                }
            }

    hash->create(system.getParticles());
    system.addForce(fGravity);
    system.addForce(fBlackhole);
    system.addForce(fWind);

    updateSimParams();
}


void SceneSPHWaterCube::updateSimParams()
{
    // get gravity from UI and update force
    double g = widget->getGravity();
    fGravity->setAcceleration(Vec3(0, -g, 0));
    fBlackhole->setIntensity(widget->getBlackholeIntensity());

    // get other relevant UI values and update simulation params
    maxParticleLife = 20.0;
    emitRate = 50;
}


void SceneSPHWaterCube::paint(const Camera& camera) {

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
    modelMat.translate(0.f,-colliderFloor.planeD,0.f);
    modelMat.scale(100, 1, 100);
    shader->setUniformValue("ModelMatrix", modelMat);
    shader->setUniformValue("matdiff", 0.8f, 0.8f, 0.8f);
    shader->setUniformValue("matspec", 0.297254f, 0.30829f, 0.306678f);
    shader->setUniformValue("matshin", 12.8f);
    shader->setUniformValue("alpha", 1.0f);
    glFuncs->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // draw sphere
    vaoSphereBigS->bind();
    modelMat = QMatrix4x4();
    modelMat.translate(colliderSphere.sphereC[0],colliderSphere.sphereC[1],colliderSphere.sphereC[2]);
    modelMat.scale(colliderSphere.sphereR*0.98);
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
        shader->setUniformValue("matspec", 0.297254f, 0.30829f, 0.306678f);
        shader->setUniformValue("matshin", 12.8f);
        shader->setUniformValue("alpha", 1.0f);

        glFuncs->glDrawElements(GL_TRIANGLES, 3*numFacesSphereS, GL_UNSIGNED_INT, 0);
    }

    // draw aabb
    vaoCube->bind();
    modelMat = QMatrix4x4();
    modelMat.translate(colliderCube.pos.x(),colliderCube.pos.y(),colliderCube.pos.z());
    modelMat.scale(colliderCube.scale.x(),colliderCube.scale.y(),colliderCube.scale.z());
    shader->setUniformValue("ModelMatrix", modelMat);
    shader->setUniformValue("matdiff", 0.55f, 0.55f, 0.55f);
    shader->setUniformValue("matspec", 0.7f, 0.7f, 0.7f);
    shader->setUniformValue("matshin", 32.f);
    shader->setUniformValue("alpha", 0.1f);
    glFuncs->glDrawElements(GL_TRIANGLES, 3*numFacesCube, GL_UNSIGNED_INT, 0);

    shader->release();
}

float kernelFunction(float r, float h){
    if(0 <= r && r <= h){
        float h2_r2 = h*h - r*r;
        float h9 = h*h*h*h*h*h*h*h*h;
        return h2_r2*h2_r2*h2_r2*315.f/(64.f*3.14159192*h9);
    }
    return 0;
}

Vec3 kernelFunctionGradient(Vec3 dir, float r, float h){
    if(0 <= r && r <= h){
        float h2_r2 = h*h - r*r;
        float h9 = h*h*h*h*h*h*h*h*h;
        return dir*h2_r2*h2_r2*945.f/(32.f*3.14159192*h9);
    }
    return Vec3(0.f,0.f,0.f);
}

float pressureFunction(float pi, float p0){
    float c = 3.f;
    return c*c*(pi-p0);
}

void SceneSPHWaterCube::update() {
    double dt = timeStep;
    float maxVelocity = 1.f; //0.2 * cloth->thickness / dt;

    hash->create(system.getParticles());

    float h = sqrt(water_radius*water_radius*4.f + water_radius*water_radius*4.f);
    float p0 = 0.0012116683647036552;
    for(int i=0; i<system.getNumParticles();i++) {
        Particle *pi = system.getParticles()[i];
        // find neighbors
        hash->query(pi->pos,h);

        // calculate density
        pi->density = 0.f;
        for(unsigned int nr=0; nr<hash->querySize;nr++){
            Particle *pj = system.getParticles()[hash->queryIds[nr]];
            float k = kernelFunction((pj->pos-pi->pos).norm(),h);
            if(k) pi->density += pj->mass*k;
        }

    // calculate pressure
    pi->pressure = pressureFunction(pi->density,p0);
    }

    for(int i=0; i<system.getNumParticles();i++) {
        Particle *pi = system.getParticles()[i];
        // find neighbors
        hash->query(pi->pos,h);

        Vec3 a_pressure = Vec3(0.f,0.f,0.f);
        for(unsigned int nr=0; nr<hash->querySize;nr++){
            Particle *pj = system.getParticles()[hash->queryIds[nr]];

            float p_ij = -pj->mass*(pi->pressure/(pi->density*pi->density) + pj->pressure/(pj->density*pj->density));
            Vec3 k = kernelFunctionGradient(pj->pos-pi->pos,(pj->pos-pi->pos).norm(),h);
            if(k != Vec3(0.f,0.f,0.f)) a_pressure += p_ij*k;
        }
        fSPHSystem[i]->setAcceleration(a_pressure);
    }

    // integration step
    Vecd ppos = system.getPositions();
    integrator.step(system, dt);
    system.setPreviousPositions(ppos);


    // collisions
    for (int i=0; i<system.getNumParticles();i++) {
        Particle* pi = system.getParticles()[i];

        // AABB collider
        if (colliderCube.testCollision(pi)) {
            colliderCube.resolveCollision(pi, bouncing, friction, dt);
        }
    }
}

void SceneSPHWaterCube::mousePressed(const QMouseEvent* e, const Camera&)
{
    mouseX = e->pos().x();
    mouseY = e->pos().y();
}

void SceneSPHWaterCube::mouseReleased(const QMouseEvent* e, const Camera& cam)
{
    if(select_pi_status==1){
        selectedPi->color = Vec3(1.f,1.f,1.f);
    }
    if(e->modifiers()&Qt::ShiftModifier && select_pi_status==1){
        selectedPi->lock = true;
        selectedPi->color = Vec3(0.1f,0.1f,0.1f);
    }
    selectedPi = nullptr;
    select_pi_status=0;
}

void SceneSPHWaterCube::mouseMoved(const QMouseEvent* e, const Camera& cam)
{
    int dx = e->pos().x() - mouseX;
    int dy = e->pos().y() - mouseY;
    mouseX = e->pos().x();
    mouseY = e->pos().y();

    Vec3 disp = cam.worldSpaceDisplacement(dx, -dy, cam.getEyeDistance());

    // example
    if (e->modifiers() & Qt::ControlModifier) {
        // move fountain
    }
    else {
        // do something else: e.g. move colliders
        switch(widget->getMovableObjectId()) {
        case 0:
            colliderSphere.sphereC += disp;
            break;
        case 1:
            colliderCube.pos += disp;
            for(Particle* pi: system.getParticles()){
                pi->pos += disp;
            }
            break;
        case 2:
            fBlackhole->position += disp;
            break;
        }
    }
}

void SceneSPHWaterCube::keyPressed(const QKeyEvent * e, const Camera& cam)
{
    //update();
    keysPressed.insert(e->key());
    if(keysPressed.contains(Qt::Key_W)){
        Vec3 disp = Vec3(0.f,0.f,-0.4f);
        colliderCube.pos += disp;
        for(Particle* pi: system.getParticles()){
            pi->pos += disp;
        }
    }
    if(keysPressed.contains(Qt::Key_S)){
        Vec3 disp = Vec3(0.f,0.f, 0.4f);
        colliderCube.pos += disp;
        for(Particle* pi: system.getParticles()){
            pi->pos += disp;
        }
    }
    if(keysPressed.contains(Qt::Key_A)){
        Vec3 disp = Vec3(0.3f,0.f, 0.f);
        for(Particle* pi: system.getParticles()){
            pi->pos += disp;
        }
    }
    if(keysPressed.contains(Qt::Key_D)){
        Vec3 disp = Vec3(-0.3f,0.f, 0.f);
        for(Particle* pi: system.getParticles()){
            pi->pos += disp;
        }
    }

}

void SceneSPHWaterCube::keyReleased(const QKeyEvent * e, const Camera& cam){
    keysPressed.remove(e->key());
}
