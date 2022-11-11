#include "sceneop.h"
#include "glutils.h"
#include "model.h"
#include <QOpenGLFunctions_3_3_Core>


SceneOP::SceneOP() {
    widget = new WidgetOP();
    connect(widget, SIGNAL(updatedParameters()), this, SLOT(updateSimParams()));
    connect(widget, SIGNAL(releasedLockedParticles()), this, SLOT(releaseSimLockedParticles()));
}


SceneOP::~SceneOP() {
    if (widget)     delete widget;
    if (shader)     delete shader;
    if (vaoFloor)   delete vaoFloor;
    if (vaoSphereS) delete vaoSphereS;
    if (vaoSphereBigS) delete vaoSphereBigS;
    if (fGravity)   delete fGravity;
    if (fBlackhole) delete fBlackhole;
    if (cloth) delete cloth;
}


void SceneOP::initialize(double dt, double bo, double fr, unsigned int dragt) {

    timeStep = dt;
    bouncing = bo;
    friction = fr;
    dragType = dragt;

    // load shader
    shader = glutils::loadShaderProgram(":/shaders/phong.vert", ":/shaders/phong.frag");
    shaderCloth = glutils::loadShaderProgram(":/shaders/cloth.vert",":/shaders/cloth.geom", ":/shaders/cloth.frag");

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
    fWind = new ForceConstAcceleration(Vec3(0, 0, 0));
    system.addForce(fWind);

    // create cloth
    cloth = new Sail(numParticlesX,numParticlesY,Vec3(-numParticlesX/2,65,-numParticlesY/2));
    for(int i=0;i<cloth->particles.length();i++){
        system.addParticle(cloth->particles[i]);
        fGravity->addInfluencedParticle(cloth->particles[i]);
        fBlackhole->addInfluencedParticle(cloth->particles[i]);
        fWind ->addInfluencedParticle(cloth->particles[i]);
    }
    for(int i=0;i<cloth->force_springs.length();i++){
        system.addForce(cloth->force_springs[i]);
    }

    // create cloth mesh VAO
    vaoMesh = new QOpenGLVertexArrayObject();
    vaoMesh->create();
    vaoMesh->bind();
    vboMesh = new QOpenGLBuffer(QOpenGLBuffer::Type::VertexBuffer);
    vboMesh->create();
    vboMesh->bind();
    vboMesh->setUsagePattern(QOpenGLBuffer::UsagePattern::DynamicDraw);
    vboMesh->allocate(cloth->numParticles*3*3*sizeof(float)); // sync with widget max particles
    shaderCloth->setAttributeBuffer("vertex", GL_FLOAT, 0, 3, 0);
    shaderCloth->enableAttributeArray("vertex");
    iboMesh = new QOpenGLBuffer(QOpenGLBuffer::Type::IndexBuffer);
    iboMesh->create();
    iboMesh->bind();
    iboMesh->setUsagePattern(QOpenGLBuffer::UsagePattern::StaticDraw);
    iboMesh->allocate(cloth->numParticles*2*3*sizeof(unsigned int));
    vaoMesh->release();
    glutils::checkGLError();

    //update index buffer
    iboMesh->bind();
    numMeshIndices = (numParticlesX - 1)*(numParticlesY - 1)*2*3;
    int* indices= new int[numMeshIndices];
    int idx = 0;
    for(int i=0;i<numParticlesY-1;i++){
        for(int j=0;j<numParticlesX-1;j++){
            indices[idx  ] = i    *numParticlesX + j  ;
            indices[idx+1] = (i+1)*numParticlesX + j  ;
            indices[idx+2] = i    *numParticlesX + j+1;
            indices[idx+3] = i    *numParticlesX + j+1;
            indices[idx+4] = (i+1)*numParticlesX + j  ;
            indices[idx+5] = (i+1)*numParticlesX + j+1;
            idx += 6;
        }
    }
    void* bufptr = iboMesh->mapRange(0, numMeshIndices*sizeof(int),
                                     QOpenGLBuffer::RangeInvalidateBuffer | QOpenGLBuffer::RangeWrite);
    memcpy(bufptr, (void*)(indices), numMeshIndices*sizeof(int));
    iboMesh->unmap();
    iboMesh->release();
    delete[] indices;
    glutils::checkGLError();

    // scene
    fountainPos = Vec3(50, 10, 50);
    fountainPos2 = Vec3(-50, 1, 50);
    colliderFloor.setPlane(Vec3(0, 1, 0), 0);
    colliderSphere.setSphere(Vec3(80, 0, 20), 20);
    colliderBoat.setAABB(Vec3(0, 5, 0),Vec3(25, 10, 40));
    colliderMast.setAABB(Vec3(0, 37, -18),Vec3(2, 35, 2));
    colliderSailwood1.setAABB(Vec3(0, 65, -20),Vec3(30, 0.5, 0.5));
    colliderSailwood2.setAABB(Vec3(0, 25, -20),Vec3(30, 0.5, 0.5));

    // create spatial hashing
    hash = new Hash(1.f,cloth->numParticles);

    playlist = new QMediaPlaylist;
    player = new QMediaPlayer;

}


void SceneOP::reset(double dt, double bo, double fr, unsigned int dragt)
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
    fWind->clearInfluencedParticles();
    for(int i=0;i<cloth->force_springs.length();i++){
        cloth->force_springs[i]->clearInfluencedParticles();
    }
    system.deleteParticles();
    system.clearForces();
    deadParticles.clear();

    hash->create(cloth->particles);
    system.addForce(fGravity);
    system.addForce(fBlackhole);
    system.addForce(fWind);

    delete(cloth);
    cloth = new Sail(numParticlesX,numParticlesY,Vec3(-numParticlesX/2,65,-numParticlesY/2));
    for(int i=0;i<cloth->particles.length();i++){
        system.addParticle(cloth->particles[i]);
        fGravity->addInfluencedParticle(cloth->particles[i]);
        fBlackhole->addInfluencedParticle(cloth->particles[i]);
        fWind->addInfluencedParticle(cloth->particles[i]);
    }
    for(int i=0;i<cloth->force_springs.length();i++){
        system.addForce(cloth->force_springs[i]);
    }

    //update index buffer
    iboMesh->bind();
    numMeshIndices = (numParticlesX - 1)*(numParticlesY - 1)*2*3;
    int* indices= new int[numMeshIndices];
    int idx = 0;
    for(int i=0;i<numParticlesY-1;i++){
        for(int j=0;j<numParticlesX-1;j++){
            indices[idx  ] = i    *numParticlesX + j  ;
            indices[idx+1] = (i+1)*numParticlesX + j  ;
            indices[idx+2] = i    *numParticlesX + j+1;
            indices[idx+3] = i    *numParticlesX + j+1;
            indices[idx+4] = (i+1)*numParticlesX + j  ;
            indices[idx+5] = (i+1)*numParticlesX + j+1;
            idx += 6;
        }
    }
    void* bufptr = iboMesh->mapRange(0, numMeshIndices*sizeof(int),
                                     QOpenGLBuffer::RangeInvalidateBuffer | QOpenGLBuffer::RangeWrite);
    memcpy(bufptr, (void*)(indices), numMeshIndices*sizeof(int));
    iboMesh->unmap();
    iboMesh->release();
    delete[] indices;
    glutils::checkGLError();

    //update cloth mesh VBO coords
    vboMesh->bind();
    float* pos = new float[3*numParticlesX*numParticlesY];
    for(int i = 0; i<numParticlesX*numParticlesY;i++){
        pos[3*i  ]=cloth->particles[i]->pos.x();
        pos[3*i+1]=cloth->particles[i]->pos.y();
        pos[3*i+2]=cloth->particles[i]->pos.z();
    }
    bufptr = vboMesh->mapRange(0, 3*numParticlesX*numParticlesY*sizeof(float),
                                     QOpenGLBuffer::RangeInvalidateBuffer | QOpenGLBuffer::RangeWrite);
    memcpy(bufptr, (void*)pos, 3*numParticlesX*numParticlesY*sizeof(float));
    vboMesh->unmap();
    vboMesh->release();
    delete[] pos;

    updateSimParams();

    colliderFloor.setPlane(Vec3(0, 1, 0), 0);
    colliderSphere.setSphere(Vec3(80, 0, 20), 20);
    colliderBoat.setAABB(Vec3(0, 5, 0),Vec3(25, 10, 40));
    colliderMast.setAABB(Vec3(0, 37, -18),Vec3(2, 35, 2));
    colliderSailwood1.setAABB(Vec3(0, 65, -20),Vec3(30, 0.5, 0.5));
    colliderSailwood2.setAABB(Vec3(0, 25, -20),Vec3(30, 0.5, 0.5));

    //playlist = new QMediaPlaylist;
    playlist->addMedia(QUrl::fromLocalFile("D:/MIRI/2022-2/SimulationsBase/computer-animation-simulation/sounds/overtaken.mp3"));
    playlist->setPlaybackMode(QMediaPlaylist::PlaybackMode::Loop);
    player->setPlaylist(playlist);
    player->setVolume(10);
    player->play();
}


void SceneOP::updateSimParams()
{
    // get gravity from UI and update force
    double g = widget->getGravity();
    fGravity->setAcceleration(Vec3(0, -g, 0));
    fBlackhole->setIntensity(widget->getBlackholeIntensity());

    for(ForceSpring* fs : cloth->force_springs){
        fs->ke = widget->getKe();
        fs->kd = widget->getKd();
    }
    relaxation_steps = widget->getRelaxationSteps();

    // get other relevant UI values and update simulation params
    maxParticleLife = 20.0;
    emitRate = 50;
}

void SceneOP::releaseSimLockedParticles()
{
    for(Particle* pi: cloth->particles){
        if(pi->lock){
            pi->lock = false;
            pi->color = Vec3(1.f,1.f,1.f);
        }
    }
}


void SceneOP::paint(const Camera& camera) {

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
    modelMat.translate(0.f,-1.f,0.f);
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

    // draw aabb
    vaoCube->bind();
    modelMat = QMatrix4x4();
    modelMat.translate(colliderBoat.pos.x(),colliderBoat.pos.y(),colliderBoat.pos.z());
    modelMat.scale(colliderBoat.scale.x()*0.98f,colliderBoat.scale.y()*0.98f,colliderBoat.scale.z()*0.98f);
    shader->setUniformValue("ModelMatrix", modelMat);
    shader->setUniformValue("matdiff", 0.4f, 0.2368f, 0.1036f);
    shader->setUniformValue("matspec", 0.774597f, 0.458561f, 0.200621f);
    shader->setUniformValue("matshin", 76.8f);
    shader->setUniformValue("alpha", 1.0f);
    glFuncs->glDrawElements(GL_TRIANGLES, 3*numFacesCube, GL_UNSIGNED_INT, 0);

    modelMat = QMatrix4x4();
    modelMat.translate(colliderMast.pos.x(),colliderMast.pos.y(),colliderMast.pos.z());
    modelMat.scale(colliderMast.scale.x()*0.98f,colliderMast.scale.y()*0.98f,colliderMast.scale.z()*0.98f);
    shader->setUniformValue("ModelMatrix", modelMat);
    glFuncs->glDrawElements(GL_TRIANGLES, 3*numFacesCube, GL_UNSIGNED_INT, 0);

    modelMat = QMatrix4x4();
    modelMat.translate(colliderSailwood1.pos.x(),colliderSailwood1.pos.y(),colliderSailwood1.pos.z());
    modelMat.scale(colliderSailwood1.scale.x()*0.98f,colliderSailwood1.scale.y()*0.98f,colliderSailwood1.scale.z()*0.98f);
    shader->setUniformValue("ModelMatrix", modelMat);
    glFuncs->glDrawElements(GL_TRIANGLES, 3*numFacesCube, GL_UNSIGNED_INT, 0);

    modelMat = QMatrix4x4();
    modelMat.translate(colliderSailwood2.pos.x(),colliderSailwood2.pos.y(),colliderSailwood2.pos.z());
    modelMat.scale(colliderSailwood2.scale.x()*0.98f,colliderSailwood2.scale.y()*0.98f,colliderSailwood2.scale.z()*0.98f);
    shader->setUniformValue("ModelMatrix", modelMat);
    glFuncs->glDrawElements(GL_TRIANGLES, 3*numFacesCube, GL_UNSIGNED_INT, 0);


    // draw the different spheres
    if(widget->getRenderParticles()){
        vaoSphereS->bind();
        for (const Particle* particle : cloth->particles) {
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
    }

    // draw the different spheres
    vaoSphereS->bind();
    for (const Particle* particle : fountainParticles) {
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

    // draw the different spheres
    vaoSphereS->bind();
    for (const Particle* particle : fountainParticles2) {
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
    shader->release();

    // draw cloth mesh
    if(widget->getRenderCloth()){
        shaderCloth->bind();

        // camera matrices
        shaderCloth->setUniformValue("ProjMatrix", camProj);
        shaderCloth->setUniformValue("ViewMatrix", camView);
        shaderCloth->setUniformValue("radius", 1.f);

        shaderCloth->setUniformValue("numLights", numLights);
        shaderCloth->setUniformValueArray("lightPos", lightPosCam, numLights);
        shaderCloth->setUniformValueArray("lightColor", lightColor, numLights);

        vaoMesh->bind();
        shaderCloth->setUniformValue("matambFront", 0.f, 0.f, 0.f);
        shaderCloth->setUniformValue("matdiffFront", 0.f, 0.f, 0.5f);
        shaderCloth->setUniformValue("matspecFront", 0.6f, 0.6f, 0.7f);
        shaderCloth->setUniformValue("matshinFront", 0.25f*128);
        shaderCloth->setUniformValue("matambBack", 0.f, 0.f, 0.f);
        shaderCloth->setUniformValue("matdiffBack", 0.5f, 0.f, 0.f);
        shaderCloth->setUniformValue("matspecBack", 0.7f, 0.6f, 0.6f);
        shaderCloth->setUniformValue("matshinBack", 0.25f*128);
        /*shaderCloth->setUniformValue("matambBack", 1.0f, 0.5f, 0.31f);
        shaderCloth->setUniformValue("matdiffBack", 1.0f, 0.5f, 0.31f);
        shaderCloth->setUniformValue("matspecBack", 0.7f, 0.04f, 0.04f);
        shaderCloth->setUniformValue("matshinBack", 32.0f);
        shaderCloth->setUniformValue("matambFront", 0.0215f, 0.1745f, 	0.0215f);
        shaderCloth->setUniformValue("matdiffFront", 0.07568f, 0.61424f, 0.07568f);
        shaderCloth->setUniformValue("matspecFront", 0.633f, 0.727811f, 0.633f);
        shaderCloth->setUniformValue("matshinFront", 0.6f*128);*/
        glFuncs->glDrawElements(GL_TRIANGLES, 3*(numParticlesX-1)*(numParticlesY-1)*2, GL_UNSIGNED_INT, 0);
        shaderCloth->release();
    }
}


void SceneOP::update() {
    double dt = timeStep;
    float maxVelocity = 0.2 * cloth->thickness / dt;

    // emit new particles, reuse dead ones if possible
    int emitParticles = std::max(1, int(std::round(emitRate * dt)));
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
            fountainParticles.push_back(p);

            // don't forget to add particle to forces that affect it
            fGravity->addInfluencedParticle(p);
            fBlackhole->addInfluencedParticle(p);
        }

        p->color = Vec3(153/255.0, 217/255.0, 234/255.0);
        p->radius = 1.0;
        p->life = maxParticleLife;

        int x = Random::get(-50, 50);
        int y = 0;
        int z = Random::get(-50, 50);
        p->pos = Vec3(Random::get(-50, 50)/20.f,y,Random::get(-50, 50)/20.f) + fountainPos;
        p->vel = Vec3(x/10.f,Random::get(28, 30),z/10.f);
        p->prevPos = p->pos - timeStep*p->vel;
    }
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
            fountainParticles2.push_back(p);

            // don't forget to add particle to forces that affect it
            fGravity->addInfluencedParticle(p);
            fBlackhole->addInfluencedParticle(p);
        }

        p->color = Vec3(153/255.0, 217/255.0, 234/255.0);
        p->radius = 1.0;
        p->life = maxParticleLife;

        int x = Random::get(-50, 50);
        int y = 0;
        int z = Random::get(-50, 50);
        p->pos = Vec3(Random::get(-50, 50)/20.f,y,Random::get(-50, 50)/20.f) + fountainPos2;
        p->vel = Vec3(x/10.f,Random::get(28, 30),z/10.f);
        p->prevPos = p->pos - timeStep*p->vel;
    }

    hash->create(cloth->particles);
    float maxTravelDist = maxVelocity * dt;
    hash->queryAll(cloth->particles,maxTravelDist);

    int n_substeps=10;
    for(int i_substeps=0;i_substeps<n_substeps;i_substeps++){
        // integration step
        Vecd ppos = system.getPositions();
        integrator.step(system, dt/n_substeps);
        system.setPreviousPositions(ppos);
        int counter_rs = 0;
        while(counter_rs < relaxation_steps){
            for (ForceSpring* fs : cloth->force_springs) {
                if(fs->type==0 || fs->type==1){
                    std::vector<Particle*> fs_ps = fs->getInfluencedParticles();
                    Vec3 dir = fs_ps[1]->pos - fs_ps[0]->pos;
                    float dirPrev = fs->L;
                    if(dir.norm() > 1.1*dirPrev){
                        if (fs_ps[0]->lock) {
                            fs_ps[1]->pos = fs_ps[0]->pos + 1.1*dirPrev*dir.normalized();
                        }
                        else if (fs_ps[1]->lock) {
                            fs_ps[0]->pos = fs_ps[1]->pos - 1.1*dirPrev*dir.normalized();
                        }
                        else {Vec3 center = (fs_ps[1]->pos + fs_ps[0]->pos)/2.f;
                            fs_ps[0]->pos = center - 0.55*dirPrev*dir.normalized();
                            fs_ps[1]->pos = center + 0.55*dirPrev*dir.normalized();
                        }
                    }
                    else if(dir.norm() < 0.9*dirPrev){
                        if (fs_ps[0]->lock) {
                            fs_ps[1]->pos = fs_ps[0]->pos + 0.9*dirPrev*dir.normalized();
                        }
                        else if (fs_ps[1]->lock) {
                            fs_ps[0]->pos = fs_ps[1]->pos - 0.9*dirPrev*dir.normalized();
                        }
                        else {Vec3 center = (fs_ps[1]->pos + fs_ps[0]->pos)/2.f;
                            fs_ps[0]->pos = center - 0.45*dirPrev*dir.normalized();
                            fs_ps[1]->pos = center + 0.45*dirPrev*dir.normalized();
                        }
                    }
                }
            }
            counter_rs++;
        }


        // collisions
        float thickness2 = cloth->thickness * cloth->thickness;
        for (int i=0; i<system.getNumParticles();i++) {
            int id0 = i;
            Particle* p0 = system.getParticles()[id0];
            float particleMinDist = 2.0 * p0->radius;
            // Floor collider
            if (colliderFloor.testCollision(p0)) {
                colliderFloor.resolveCollision(p0, bouncing, friction, dt/n_substeps);
            }
            // Sphere collider
            if (colliderSphere.testCollision(p0)) {
                colliderSphere.resolveCollision(p0, bouncing, friction, dt/n_substeps);
            }
            // AABB collider
            if (colliderBoat.testCollision(p0)) {
                colliderBoat.resolveCollision(p0, bouncing, friction, dt/n_substeps);
            }
        }
        for (int i=0; i<cloth->numParticles;i++) {
            int id0 = i;
            Particle* p0 = cloth->particles[id0];
            // Spatial Hashing collider
            if(widget->getSelfCollisions()){
                if (1/p0->mass == 0.0)
                    continue;
                int first = hash->firstAdjId[i];
                int last = hash->firstAdjId[i + 1];

                for (int j = first; j < last; j++) {

                    int id1 = hash->adjIds[j];
                    Particle* p1 = cloth->particles[id1];
                    if (1/p1->mass == 0.0)
                        continue;

                    Vec3 vecs = p1->pos-p0->pos;
                    float dist2 = vecs.squaredNorm();
                    if (dist2 > thickness2 || dist2 == 0.0)
                        continue;
                    float restDist2 = (Vec3(p0->id_width,0.f,p0->id_height)-Vec3(p1->id_width,0.f,p1->id_height)).squaredNorm();

                    float minDist = cloth->thickness;
                    if (dist2 > restDist2)
                        continue;
                    if (restDist2 < thickness2)
                        minDist = sqrt(restDist2);

                    // position correction
                    float dist = sqrt(dist2);
                    vecs *= (minDist - dist) / dist;
                    p0->pos += -0.5*vecs;
                    p1->pos += 0.5*vecs;

                    // velocities
                    Vec3 vecs0 = p0->pos - p0->prevPos;
                    Vec3 vecs1 = p1->pos - p1->prevPos;

                    // average velocity
                    Vec3 vecs2 = (vecs0 + vecs1)*0.5;

                    // velocity corrections
                    vecs0 = vecs2 - vecs0;
                    vecs1 = vecs2 - vecs1;

                    // add corrections
                    float friction = 0.00;
                    p0->pos += friction*vecs0;
                    p1->pos += friction*vecs1;
                }
            }
        }
    }
    //update cloth mesh VBO coords
    vboMesh->bind();
    float* pos = new float[3*numParticlesX*numParticlesY];
    for(int i = 0; i<numParticlesX*numParticlesY;i++){
        pos[3*i  ]=cloth->particles[i]->pos.x();
        pos[3*i+1]=cloth->particles[i]->pos.y();
        pos[3*i+2]=cloth->particles[i]->pos.z();
    }
    void* bufptr = vboMesh->mapRange(0, 3*numParticlesX*numParticlesY*sizeof(float),
                                     QOpenGLBuffer::RangeInvalidateBuffer | QOpenGLBuffer::RangeWrite);
    memcpy(bufptr, (void*)pos, 3*numParticlesX*numParticlesY*sizeof(float));
    vboMesh->unmap();
    vboMesh->release();
    delete[] pos;

    // check dead particles
    for (Particle* p : fountainParticles) {
        if (p->life > 0) {
            p->life -= dt;
            if (p->life < 0) {
                deadParticles.push_back(p);
            }
        }
    }
    for (Particle* p : fountainParticles2) {
        if (p->life > 0) {
            p->life -= dt;
            if (p->life < 0) {
                deadParticles.push_back(p);
            }
        }
    }

    fWind->setAcceleration(Vec3(0.f,5,-5));
    curr_step++;
    if(curr_step%120==0){fountainPos=Vec3(Random::get(-80, 80),1.f,Random::get(-20, -80));}
    else fountainPos+=Vec3(0.f,0.f,0.5f);
    if((curr_step+50)%120==0){fountainPos2=Vec3(Random::get(-80, 80),1.f,Random::get(-20, -80));}
    else fountainPos2+=Vec3(0.f,0.f,0.5f);
}

void SceneOP::mousePressed(const QMouseEvent* e, const Camera&)
{
    mouseX = e->pos().x();
    mouseY = e->pos().y();
}

void SceneOP::mouseReleased(const QMouseEvent* e, const Camera& cam)
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

void SceneOP::mouseMoved(const QMouseEvent* e, const Camera& cam)
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
        fountainPos2 += disp;
    }
    else {
        // do something else: e.g. move colliders
        switch(widget->getMovableObjectId()) {
        case 0:
            colliderSphere.sphereC += disp;
            break;
        case 1:
            colliderBoat.pos += disp;
            colliderMast.pos += disp;
            colliderSailwood1.pos += disp;
            colliderSailwood2.pos += disp;
            for(Particle* pi: cloth->particles){
                pi->pos += disp;
            }
            break;
        case 2:
            fBlackhole->position += disp;
            break;
        case 3:
            Vec3 rayDir = cam.getRayDir(mouseX,mouseY);
            if(select_pi_status==2){
                break;
            } else if(select_pi_status==1){
                selectedPi->pos += disp;
                break;
            } else if(select_pi_status==0){
                float i = 0.f;
                while(!selectedPi && i<200){
                    Vec3 rayPos = cam.getPos() + i*rayDir;
                    hash->query(rayPos,0.5f);
                    if(hash->querySize){
                        for(int j=0;j<hash->querySize;j++){
                            Vec3 sphereC = cloth->particles[hash->queryIds[j]]->pos;
                            float sphereR = 1.f;//cloth->particles[hash->queryIds[j]]->radius;
                            Vec3 pointDiff = rayPos-sphereC;
                            if(pointDiff.dot(pointDiff.transpose())<=sphereR*sphereR){
                                selectedPi = cloth->particles[hash->queryIds[j]];
                                select_pi_status = 1;
                                selectedPi->color = Vec3(1.f,0.f,0.f);
                                selectedPi->pos += disp;
                                break;
                            }
                        }
                    }
                    i+=2.f;
                }
                if(select_pi_status!=1)select_pi_status = 2;
            }
            break;
        }
    }
}

void SceneOP::keyPressed(const QKeyEvent * e, const Camera& cam)
{
    //update();
    keysPressed.insert(e->key());
    if(keysPressed.contains(Qt::Key_W)){
        Vec3 disp = Vec3(0.f,0.f,-0.4f);
        colliderBoat.pos += disp;
        colliderMast.pos += disp;
        colliderSailwood1.pos += disp;
        colliderSailwood2.pos += disp;
        for(Particle* pi: cloth->particles){
            pi->pos += disp;
        }
    }
    if(keysPressed.contains(Qt::Key_S)){
        Vec3 disp = Vec3(0.f,0.f, 0.4f);
        colliderBoat.pos += disp;
        colliderMast.pos += disp;
        colliderSailwood1.pos += disp;
        colliderSailwood2.pos += disp;
        for(Particle* pi: cloth->particles){
            pi->pos += disp;
        }
    }
    if(keysPressed.contains(Qt::Key_A)){
        Vec3 disp = Vec3(0.3f,0.f, 0.f);
        for(Particle* pi: fountainParticles){
            pi->pos += disp;
        }
        for(Particle* pi: fountainParticles2){
            pi->pos += disp;
        }
        fountainPos += disp;
        fountainPos2 += disp;
    }
    if(keysPressed.contains(Qt::Key_D)){
        Vec3 disp = Vec3(-0.3f,0.f, 0.f);
        for(Particle* pi: fountainParticles){
            pi->pos += disp;
        }
        for(Particle* pi: fountainParticles2){
            pi->pos += disp;
        }
        fountainPos += disp;
        fountainPos2 += disp;
    }

}

void SceneOP::keyReleased(const QKeyEvent * e, const Camera& cam){
    keysPressed.remove(e->key());
}
