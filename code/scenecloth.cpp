#include "scenecloth.h"
#include "glutils.h"
#include "model.h"
#include <QOpenGLFunctions_3_3_Core>


SceneCloth::SceneCloth() {
    widget = new WidgetCloth();
    connect(widget, SIGNAL(updatedParameters()), this, SLOT(updateSimParams()));
    connect(widget, SIGNAL(releasedLockedParticles()), this, SLOT(releaseSimLockedParticles()));
}


SceneCloth::~SceneCloth() {
    if (widget)     delete widget;
    if (shader)     delete shader;
    if (vaoFloor)   delete vaoFloor;
    if (vaoSphereS) delete vaoSphereS;
    if (vaoSphereBigS) delete vaoSphereBigS;
    if (fGravity)   delete fGravity;
    if (fBlackhole) delete fBlackhole;
    if (cloth) delete cloth;
}


void SceneCloth::initialize(double dt, double bo, double fr, unsigned int dragt) {

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

    // create cloth
    cloth = new Cloth(numParticlesX,numParticlesY,Vec3(-numParticlesX/2,50,-numParticlesY/2));
    for(int i=0;i<cloth->particles.length();i++){
        system.addParticle(cloth->particles[i]);
        fGravity->addInfluencedParticle(cloth->particles[i]);
        fBlackhole->addInfluencedParticle(cloth->particles[i]);
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
    fountainPos = Vec3(0, 10, 0);
    colliderFloor.setPlane(Vec3(0, 1, 0), 0);
    colliderSphere.setSphere(Vec3(20, 0, 20), 20);
    colliderAABB.setAABB(Vec3(0, 0, 0),Vec3(15, 15, 30));

    // create spatial hashing
    hash = new Hash(1.0,cloth->numParticles);

}


void SceneCloth::reset(double dt, double bo, double fr, unsigned int dragt)
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
    for(int i=0;i<cloth->force_springs.length();i++){
        cloth->force_springs[i]->clearInfluencedParticles();
    }
    system.deleteParticles();
    system.clearForces();
    deadParticles.clear();

    hash->create(system.getParticles());
    system.addForce(fGravity);
    system.addForce(fBlackhole);

    cloth = new Cloth(numParticlesX,numParticlesY,Vec3(-numParticlesX/2,50,-numParticlesY/2));
    for(int i=0;i<cloth->particles.length();i++){
        system.addParticle(cloth->particles[i]);
        fGravity->addInfluencedParticle(cloth->particles[i]);
        fBlackhole->addInfluencedParticle(cloth->particles[i]);
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
        pos[3*i  ]=system.getParticle(i)->pos.x();
        pos[3*i+1]=system.getParticle(i)->pos.y();
        pos[3*i+2]=system.getParticle(i)->pos.z();
    }
    bufptr = vboMesh->mapRange(0, 3*numParticlesX*numParticlesY*sizeof(float),
                                     QOpenGLBuffer::RangeInvalidateBuffer | QOpenGLBuffer::RangeWrite);
    memcpy(bufptr, (void*)pos, 3*numParticlesX*numParticlesY*sizeof(float));
    vboMesh->unmap();
    vboMesh->release();
    delete[] pos;

    updateSimParams();
}


void SceneCloth::updateSimParams()
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
    maxParticleLife = 10.0;
    emitRate = 200;
}

void SceneCloth::releaseSimLockedParticles()
{
    for(Particle* pi: cloth->particles){
        if(pi->lock) pi->lock = false;
    }
}


void SceneCloth::paint(const Camera& camera) {

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
    shader->setUniformValue("matspec", 0.0f, 0.0f, 0.0f);
    shader->setUniformValue("matshin", 0.0f);
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
    glFuncs->glDrawElements(GL_TRIANGLES, 3*numFacesSphereS, GL_UNSIGNED_INT, 0);

    // draw aabb
    vaoCube->bind();
    modelMat = QMatrix4x4();
    modelMat.translate(colliderAABB.pos.x(),colliderAABB.pos.y(),colliderAABB.pos.z());
    modelMat.scale(colliderAABB.scale.x()*0.98f,colliderAABB.scale.y()*0.98f,colliderAABB.scale.z()*0.98f);
    shader->setUniformValue("ModelMatrix", modelMat);
    shader->setUniformValue("matdiff", 0.8f, 0.8f, 0.8f);
    shader->setUniformValue("matspec", 0.0f, 0.0f, 0.0f);
    shader->setUniformValue("matshin", 0.f);
    glFuncs->glDrawElements(GL_TRIANGLES, 3*numFacesCube, GL_UNSIGNED_INT, 0);


    // draw the different spheres
    if(widget->getRenderParticles()){
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

            glFuncs->glDrawElements(GL_TRIANGLES, 3*numFacesSphereS, GL_UNSIGNED_INT, 0);
        }
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
        shaderCloth->setUniformValue("matambBack", 0.f, 0.f, 0.f);
        shaderCloth->setUniformValue("matdiffBack", 0.f, 0.f, 0.5f);
        shaderCloth->setUniformValue("matspecBack", 0.6f, 0.6f, 0.7f);
        shaderCloth->setUniformValue("matshinBack", 0.25f*128);
        shaderCloth->setUniformValue("matambFront", 0.f, 0.f, 0.f);
        shaderCloth->setUniformValue("matdiffFront", 0.5f, 0.f, 0.f);
        shaderCloth->setUniformValue("matspecFront", 0.7f, 0.6f, 0.6f);
        shaderCloth->setUniformValue("matshinFront", 0.25f*128);
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


void SceneCloth::update() {
    double dt = timeStep;
    float maxVelocity = 0.2 * cloth->thickness / dt;

    hash->create(system.getParticles());
    float maxTravelDist = maxVelocity * dt;
    hash->queryAll(system.getParticles(),maxTravelDist);

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
        for (int i=0; i<cloth->numParticles;i++) {
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
            if (colliderAABB.testCollision(p0)) {
                colliderAABB.resolveCollision(p0, bouncing, friction, dt/n_substeps);
            }
            // Spatial Hashing collider
            if(widget->getSelfCollisions()){
                if (1/p0->mass == 0.0)
                    continue;
                int first = hash->firstAdjId[i];
                int last = hash->firstAdjId[i + 1];

                for (int j = first; j < last; j++) {

                    int id1 = hash->adjIds[j];
                    Particle* p1 = system.getParticles()[id1];
                    if (1/p1->mass == 0.0)
                        continue;

                    Vec3 vecs = p1->pos-p0->pos;
                    float dist2 = vecs.squaredNorm();
                    if (dist2 > thickness2 || dist2 == 0.0)
                        continue;
                    float restDist2 = 1.f;

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
                    Vec3 vecs1 = p0->pos - p0->prevPos;

                    // average velocity
                    Vec3 vecs2 = (vecs0 + vecs1)*0.5;

                    // velocity corrections
                    vecs0 = vecs2 - vecs0;
                    vecs1 = vecs2 - vecs1;

                    // add corrections
                    float friction = 0.0;
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
        pos[3*i  ]=system.getParticle(i)->pos.x();
        pos[3*i+1]=system.getParticle(i)->pos.y();
        pos[3*i+2]=system.getParticle(i)->pos.z();
    }
    void* bufptr = vboMesh->mapRange(0, 3*numParticlesX*numParticlesY*sizeof(float),
                                     QOpenGLBuffer::RangeInvalidateBuffer | QOpenGLBuffer::RangeWrite);
    memcpy(bufptr, (void*)pos, 3*numParticlesX*numParticlesY*sizeof(float));
    vboMesh->unmap();
    vboMesh->release();
    delete[] pos;
}

void SceneCloth::mousePressed(const QMouseEvent* e, const Camera&)
{
    mouseX = e->pos().x();
    mouseY = e->pos().y();
}

void SceneCloth::mouseReleased(const QMouseEvent* e, const Camera& cam)
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

void SceneCloth::mouseMoved(const QMouseEvent* e, const Camera& cam)
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
