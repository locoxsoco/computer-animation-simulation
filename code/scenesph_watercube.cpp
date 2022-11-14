#include "scenesph_watercube.h"
#include "glutils.h"
#include "model.h"
#include <QOpenGLFunctions_3_3_Core>


SceneSPHWaterCube::SceneSPHWaterCube() {
    widget = new WidgetSPHWaterCube();
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
    double p0 = widget->getRestDensity();

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
    colliderCube.setAABB(Vec3(0, 5, 0),boundarySize);

    // create pool particles
    for(unsigned int i=1;i<poolSize.y();i++)
        for(unsigned int k=1;k<poolSize.z();k++)
            for(unsigned int j=1;j<poolSize.x();j++){
                Vec3 pos=Vec3(
                            colliderCube.pos.x()-colliderCube.scale.x(),
                            colliderCube.pos.y()-colliderCube.scale.y(),
                            colliderCube.pos.z()-colliderCube.scale.z()
                            ) +
                        Vec3(
                            j*2,
                            i*2,
                            k*2
                            );
                Particle *p =new Particle(pos);
                p->color = Vec3(153/255.0, 217/255.0, 234/255.0);
                p->mass = 0.01;
                p->type = ParticleType::NotBoundary;
                p->density = p0;
                poolParticles.push_back(p);
                system.addParticle(p);

                // don't forget to add particle to forces that affect it
                fGravity->addInfluencedParticle(p);
                fBlackhole->addInfluencedParticle(p);
                // Adding SPH forces
                ForceSPH* fSPH = new ForceSPH();
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
                    p->type = ParticleType::NotBoundary;
                    p->density = p0;
                    dropParticles.push_back(p);
                    system.addParticle(p);
                    // don't forget to add particle to forces that affect it
                    fGravity->addInfluencedParticle(p);
                    fBlackhole->addInfluencedParticle(p);
                    // Adding SPH forces
                    ForceSPH* fSPH = new ForceSPH();
                    fSPH->addInfluencedParticle(p);
                    fSPHSystem.push_back(fSPH);
                    system.addForce(fSPH);
                }
            }

    // create boundary particles
    for(int i=0;i<=boundarySize.y();i++)
        for(int k=0;k<=boundarySize.z();k++){
            //plane -x
            Vec3 npos=Vec3(
                        colliderCube.pos.x()-colliderCube.scale.x(),
                        colliderCube.pos.y()-colliderCube.scale.y(),
                        colliderCube.pos.z()-colliderCube.scale.z()
                        ) +
                    Vec3(
                        0.f,
                        i*2,
                        k*2
                        );
            Particle *np =new Particle(npos);
            np->color = Vec3(1.f, 1.f, 1.f);
            np->mass = 0.01;
            np->type = ParticleType::Boundary;
            np->density = p0;
            boundaryParticles.push_back(np);
            system.addParticle(np);
            //plane +x
            Vec3 ppos=Vec3(
                        colliderCube.pos.x()+colliderCube.scale.x(),
                        colliderCube.pos.y()-colliderCube.scale.y(),
                        colliderCube.pos.z()-colliderCube.scale.z()
                        ) +
                    Vec3(
                        0.f,
                        i*2,
                        k*2
                        );
            Particle *pp =new Particle(ppos);
            pp->color = Vec3(1.f, 1.f, 1.f);
            pp->mass = 0.01;
            pp->type = ParticleType::Boundary;
            pp->density = p0;
            boundaryParticles.push_back(pp);
            system.addParticle(pp);
        }
    for(int j=0;j<=boundarySize.x();j++)
        for(int k=1;k<boundarySize.z();k++){
            //plane -y
            Vec3 npos=Vec3(
                        colliderCube.pos.x()-colliderCube.scale.x(),
                        colliderCube.pos.y()-colliderCube.scale.y(),
                        colliderCube.pos.z()-colliderCube.scale.z()
                        ) +
                    Vec3(
                        j*2,
                        0.f,
                        k*2
                        );
            Particle *np =new Particle(npos);
            np->color = Vec3(1.f, 1.f, 1.f);
            np->mass = 0.01;
            np->type = ParticleType::Boundary;
            np->density = p0;
            boundaryParticles.push_back(np);
            system.addParticle(np);
            //plane +y
            Vec3 ppos=Vec3(
                        colliderCube.pos.x()-colliderCube.scale.x(),
                        colliderCube.pos.y()+colliderCube.scale.y(),
                        colliderCube.pos.z()-colliderCube.scale.z()
                        ) +
                    Vec3(
                        j*2,
                        0.f,
                        k*2
                        );
            Particle *pp =new Particle(ppos);
            pp->color = Vec3(1.f, 1.f, 1.f);
            pp->mass = 0.01;
            pp->type = ParticleType::Boundary;
            pp->density = p0;
            boundaryParticles.push_back(pp);
            system.addParticle(pp);
        }
    for(int i=1;i<boundarySize.y();i++)
        for(int j=1;j<boundarySize.x();j++){
            //plane -z
            Vec3 npos=Vec3(
                        colliderCube.pos.x()-colliderCube.scale.x(),
                        colliderCube.pos.y()-colliderCube.scale.y(),
                        colliderCube.pos.z()-colliderCube.scale.z()
                        ) +
                    Vec3(
                        j*2,
                        i*2,
                        0.f
                        );
            Particle *np =new Particle(npos);
            np->color = Vec3(1.f, 1.f, 1.f);
            np->mass = 0.01;
            np->type = ParticleType::Boundary;
            np->density = p0;
            boundaryParticles.push_back(np);
            system.addParticle(np);
            //plane +z
            Vec3 ppos=Vec3(
                        colliderCube.pos.x()-colliderCube.scale.x(),
                        colliderCube.pos.y()-colliderCube.scale.y(),
                        colliderCube.pos.z()+colliderCube.scale.z()
                        ) +
                    Vec3(
                        j*2,
                        i*2,
                        0.f
                        );
            Particle *pp =new Particle(ppos);
            pp->color = Vec3(1.f, 1.f, 1.f);
            pp->mass = 0.01;
            pp->type = ParticleType::Boundary;
            pp->density = p0;
            boundaryParticles.push_back(pp);
            system.addParticle(pp);
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
    double p0 = widget->getRestDensity();

    colliderFloor.setPlane(Vec3(0, 1, 0), 50);
    colliderSphere.setSphere(Vec3(100, 100, 100), 20);
    colliderCube.setAABB(Vec3(0, 5, 0),boundarySize);

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
    boundaryParticles.clear();

    // create pool particles
    for(unsigned int i=1;i<poolSize.y();i++)
        for(unsigned int k=1;k<poolSize.z();k++)
            for(unsigned int j=1;j<poolSize.x();j++){
                Vec3 pos=Vec3(
                            colliderCube.pos.x()-colliderCube.scale.x(),
                            colliderCube.pos.y()-colliderCube.scale.y(),
                            colliderCube.pos.z()-colliderCube.scale.z()
                            ) +
                        Vec3(
                            j*2,
                            i*2,
                            k*2
                            );
                Particle *p =new Particle(pos);
                p->color = Vec3(153/255.0, 217/255.0, 234/255.0);
                p->mass = 0.01;
                p->type = ParticleType::NotBoundary;
                p->density = p0;
                poolParticles.push_back(p);
                system.addParticle(p);

                // don't forget to add particle to forces that affect it
                fGravity->addInfluencedParticle(p);
                fBlackhole->addInfluencedParticle(p);
                // Adding SPH forces
                ForceSPH* fSPH = new ForceSPH();
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
                    p->type = ParticleType::NotBoundary;
                    p->density = p0;
                    dropParticles.push_back(p);
                    system.addParticle(p);

                    // don't forget to add particle to forces that affect it
                    fGravity->addInfluencedParticle(p);
                    fBlackhole->addInfluencedParticle(p);
                    // Adding SPH forces
                    ForceSPH* fSPH = new ForceSPH();
                    fSPH->addInfluencedParticle(p);
                    fSPHSystem.push_back(fSPH);
                    system.addForce(fSPH);
                }
            }

    // create boundary particles
    for(int i=0;i<=boundarySize.y();i++)
        for(int k=0;k<=boundarySize.z();k++){
            //plane -x
            Vec3 npos=Vec3(
                        colliderCube.pos.x()-colliderCube.scale.x(),
                        colliderCube.pos.y()-colliderCube.scale.y(),
                        colliderCube.pos.z()-colliderCube.scale.z()
                        ) +
                    Vec3(
                        0.f,
                        i*2,
                        k*2
                        );
            Particle *np =new Particle(npos);
            np->color = Vec3(1.f, 1.f, 1.f);
            np->mass = 0.01;
            np->type = ParticleType::Boundary;
            np->density = p0;
            boundaryParticles.push_back(np);
            system.addParticle(np);
            //plane +x
            Vec3 ppos=Vec3(
                        colliderCube.pos.x()+colliderCube.scale.x(),
                        colliderCube.pos.y()-colliderCube.scale.y(),
                        colliderCube.pos.z()-colliderCube.scale.z()
                        ) +
                    Vec3(
                        0.f,
                        i*2,
                        k*2
                        );
            Particle *pp =new Particle(ppos);
            pp->color = Vec3(1.f, 1.f, 1.f);
            pp->mass = 0.01;
            pp->type = ParticleType::Boundary;
            pp->density = p0;
            boundaryParticles.push_back(pp);
            system.addParticle(pp);
        }
    for(int j=0;j<=boundarySize.x();j++)
        for(int k=1;k<boundarySize.z();k++){
            //plane -y
            Vec3 npos=Vec3(
                        colliderCube.pos.x()-colliderCube.scale.x(),
                        colliderCube.pos.y()-colliderCube.scale.y(),
                        colliderCube.pos.z()-colliderCube.scale.z()
                        ) +
                    Vec3(
                        j*2,
                        0.f,
                        k*2
                        );
            Particle *np =new Particle(npos);
            np->color = Vec3(1.f, 1.f, 1.f);
            np->mass = 0.01;
            np->type = ParticleType::Boundary;
            np->density = p0;
            boundaryParticles.push_back(np);
            system.addParticle(np);
            //plane +y
            Vec3 ppos=Vec3(
                        colliderCube.pos.x()-colliderCube.scale.x(),
                        colliderCube.pos.y()+colliderCube.scale.y(),
                        colliderCube.pos.z()-colliderCube.scale.z()
                        ) +
                    Vec3(
                        j*2,
                        0.f,
                        k*2
                        );
            Particle *pp =new Particle(ppos);
            pp->color = Vec3(1.f, 1.f, 1.f);
            pp->mass = 0.01;
            pp->type = ParticleType::Boundary;
            pp->density = p0;
            boundaryParticles.push_back(pp);
            system.addParticle(pp);
        }
    for(int i=1;i<boundarySize.y();i++)
        for(int j=1;j<boundarySize.x();j++){
            //plane -z
            Vec3 npos=Vec3(
                        colliderCube.pos.x()-colliderCube.scale.x(),
                        colliderCube.pos.y()-colliderCube.scale.y(),
                        colliderCube.pos.z()-colliderCube.scale.z()
                        ) +
                    Vec3(
                        j*2,
                        i*2,
                        0.f
                        );
            Particle *np =new Particle(npos);
            np->color = Vec3(1.f, 1.f, 1.f);
            np->mass = 0.01;
            np->type = ParticleType::Boundary;
            np->density = p0;
            boundaryParticles.push_back(np);
            system.addParticle(np);
            //plane +z
            Vec3 ppos=Vec3(
                        colliderCube.pos.x()-colliderCube.scale.x(),
                        colliderCube.pos.y()-colliderCube.scale.y(),
                        colliderCube.pos.z()+colliderCube.scale.z()
                        ) +
                    Vec3(
                        j*2,
                        i*2,
                        0.f
                        );
            Particle *pp =new Particle(ppos);
            pp->color = Vec3(1.f, 1.f, 1.f);
            pp->mass = 0.01;
            pp->type = ParticleType::Boundary;
            pp->density = p0;
            boundaryParticles.push_back(pp);
            system.addParticle(pp);
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

    // draw the different spheres
    vaoSphereS->bind();
    for (const Particle* particle : poolParticles) {
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
    for (const Particle* particle : dropParticles) {
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
    for (const Particle* particle : boundaryParticles) {
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
        shader->setUniformValue("alpha", 0.1f);

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

double getKernelFunctionPoly(Vec3 r, double h){
    double r_norm = r.norm();
    if(0.f <= r_norm && r_norm <= h){
        double h2_r2 = h*h - r_norm*r_norm;
        double h9 = h*h*h*h*h*h*h*h*h;
        return 315.f/(64.f*3.14159192*h9)*h2_r2*h2_r2*h2_r2;
    }
    return 0;
}

Vec3 getKernelFunctionGradientPoly(Vec3 r, double h){
    double r_norm = r.norm();
    if(0.f <= r_norm && r_norm <= h){
        double h2_r2 = h*h - r_norm*r_norm;
        double h9 = h*h*h*h*h*h*h*h*h;
        return r*945.f/(32.f*3.14159192*h9)*h2_r2*h2_r2;
    }
    return Vec3(0.f,0.f,0.f);
}

double getKernelFunctionSpiky(Vec3 r, double h){
    double r_norm = r.norm();
    if(0.f <= r_norm && r_norm <= h){
        double h_r = h - r_norm;
        double h6 = h*h*h*h*h*h;
        return 15.f/(3.14159192*h6)*h_r*h_r*h_r;
    }
    return 0;
}

Vec3 getKernelFunctionGradientSpiky(Vec3 r, double h){
    double r_norm = r.norm();
    if(0.f <= r_norm && r_norm <= h){
        double h_r = h - r_norm;
        double h6 = h*h*h*h*h*h;
        //Vec3 value =  -r*45.f/(3.14159192*h6*r_norm)*h_r*h_r;
        //if(value.dot(r)>=0) return value;
        return -r*45.f/(3.14159192*h6*r_norm)*h_r*h_r;
    }
    return Vec3(0.f,0.f,0.f);
}

double getKernelFunctionCubicSpline(Vec3 r, double h){
    double r_norm = r.norm();
    double q = r_norm/h;
    double sigma = 8/(3.141519192*h*h*h);
    if(0.f <= q && q <= 1/2.f){
        return sigma*6.f*(q*q*q-q*q)+1.f;
    }
    if(1.f/2.f <= q <= 1.f){
        return sigma*2.f*(1.f-q)*(1.f-q)*(1.f-q);
    }
    return 0;
}

Vec3 getKernelFunctionGradientCubicSpline(Vec3 r, double h){
    double r_norm = r.norm();
    double q = r_norm/h;
    double sigma = 8/(3.141519192*h*h*h);
    if(0.f <= q && q <= 1/2.f){
        return r.normalized()*sigma*(18*q*q-12*q);
    }
    if(1.f/2.f <= q <= 1.f){
        return -r.normalized()*sigma*6*(1-q)*(1-q);
    }
    return Vec3(0.f,0.f,0.f);
}

double getKernelFunctionLaplacianViscosity(Vec3 r, double h){
    double r_norm = r.norm();
    if(0.f <= r_norm && r_norm <= h){
        double h5 = h*h*h*h*h;
        //Vec3 value =  -r*45.f/(3.14159192*h6*r_norm)*h_r*h_r;
        //if(value.dot(r)>=0) return value;
        return 45.f/(3.14159192*h5)*(1.f-r_norm/h);
    }
    return 0.f;
}

double getKernelFunctionLaplacianViscosityImproved(Vec3 r, double h){
    double r_norm = r.norm();
    if(0.f <= r_norm && r_norm <= h){
        return 2*getKernelFunctionGradientCubicSpline(r,h).norm()/r.norm();
    }
    return 0.f;
}

double SceneSPHWaterCube::getPressureFunctionStateEquation(double pi, double p0){
    double pressure = k*(pi/p0-1.f);
    if(pressure>=0.f) return pressure;
    return 0.0;
}

double SceneSPHWaterCube::getPressureFunctionSound(double pi, double p0){
    double pressure = c*c*(pi-p0);
    if(pressure>=0.f) return pressure;
    return 0.f;
}

double getPijMeanDensitySquare(Particle* pi, Particle* pj){
    return pj->mass*(pi->pressure/(pi->density*pi->density) + pj->pressure/(pj->density*pj->density));
}

double getPijMeanDensitySquareBoundary(Particle* pi, Particle* pj){
    return -pj->mass*2*(pi->pressure/(pi->density*pi->density));
}

Vec3 getVijMeanDensitySquare(Particle* pi, Particle* pj){
    return -pj->mass/pj->density*(pj->vel-pi->vel)/pi->density;
}

void SceneSPHWaterCube::update() {
    double dt = timeStep;
    c = widget->getC()*dt; //20.04757082400839/s;
    k = widget->getK()*dt; //20.04757082400839/s;

    hash->create(system.getParticles());

    if(widget->getSPHMethod() == SPHMethod::FullyCompressible){
        double h = sqrt(water_radius*water_radius*4.f + water_radius*water_radius*4.f)*widget->getHReduction();
        double p0 = widget->getRestDensity();
        for(int i=0; i<system.getNumParticles();i++) {
            Particle *pi = system.getParticles()[i];
            if(pi->type == ParticleType::Boundary) continue; // not necessary to calculate density nor pressure for boundary particles
            // find neighbors
            hash->query(pi->pos,h);

            // calculate density
            pi->density = 0.f;
            for(unsigned int nr=0; nr<hash->querySize;nr++){
                Particle *pj = system.getParticles()[hash->queryIds[nr]];
                double k = getKernelFunctionPoly(pi->pos-pj->pos,h);
                if(k) pi->density += pj->mass*k;
            }

            // calculate pressure
            pi->pressure = getPressureFunctionSound(pi->density,p0);
        }

        for(int i=0; i<system.getNumParticles();i++) {
            Particle *pi = system.getParticles()[i];
            if(pi->type == ParticleType::Boundary) continue; // not necessary to calculate accel_pressure for boundary particles
            // find neighbors
            hash->query(pi->pos,h);

            Vec3 a_pressure = Vec3(0.f,0.f,0.f);
            for(unsigned int nr=0; nr<hash->querySize;nr++){
                Particle *pj = system.getParticles()[hash->queryIds[nr]];
                if(pi->id != pj->id){
                    double p_ij;
                    if(pj->type == ParticleType::Boundary){
                         p_ij = 0.f;//getPijMeanDensitySquareBoundary(pi,pj);
                    } else {
                        p_ij = getPijMeanDensitySquare(pi,pj);
                    }
                    Vec3 k = getKernelFunctionGradientPoly(pi->pos-pj->pos,h);
                    if(k != Vec3(0.f,0.f,0.f)) a_pressure += p_ij*k;
                }
            }
            fSPHSystem[i]->setForce(pi->mass*a_pressure);
        }

        // integration step
        Vecd ppos = system.getPositions();
        integrator.step(system, dt);
        system.setPreviousPositions(ppos);
    } else if (widget->getSPHMethod() == SPHMethod::WeaklyCompressible){

        // 1. for all particle i reconstruct density pi
        double h = sqrt(water_radius*water_radius*4.f + water_radius*water_radius*4.f)*widget->getHReduction();
        double v = widget->getKinematicViscosity();
        double p0 = widget->getRestDensity();
        for(int i=0; i<system.getNumParticles();i++) {
            Particle *pi = system.getParticles()[i];
            // find neighbors
            hash->query(pi->pos,h);

            // calculate density
            pi->density = 0.f;
            for(unsigned int nr=0; nr<hash->querySize;nr++){
                Particle *pj = system.getParticles()[hash->queryIds[nr]];
                double k = getKernelFunctionSpiky(pi->pos-pj->pos,h);
                if(k) pi->density += pj->mass*k;
            }
            pi->pressure = getPressureFunctionStateEquation(pi->density,p0);
        }

        // 2. for all particle i compute viscocity and predicted velocity
        for(int i=0; i<system.getNumParticles();i++) {
            Particle *pi = system.getParticles()[i];
            // find neighbors
            hash->query(pi->pos,h);

            Vec3 laplacian_velocity = Vec3(0.f,0.f,0.f);
            for(unsigned int nr=0; nr<hash->querySize;nr++){
                Particle *pj = system.getParticles()[hash->queryIds[nr]];
                if(pi->id != pj->id){
                    Vec3 v_ij = getVijMeanDensitySquare(pi,pj);
                    double k = getKernelFunctionLaplacianViscosity(pi->pos-pj->pos,h);
                    //double k = getKernelFunctionLaplacianViscosityImproved(pi->pos-pj->pos,h);
                    if(k) laplacian_velocity += v_ij*k;
                }
            }
            Vec3 fSPHViscosity = pi->mass*v*laplacian_velocity;
            pi->vel += dt/pi->mass*(fSPHViscosity+fGravity->getAcceleration()*pi->mass);
        }

        // 3. for all particle i compute pressure
        for(int i=0; i<system.getNumParticles();i++) {
            Particle *pi = system.getParticles()[i];
            // find neighbors
            hash->query(pi->pos,h);

            Vec3 a_pressure = Vec3(0.f,0.f,0.f);
            for(unsigned int nr=0; nr<hash->querySize;nr++){
                Particle *pj = system.getParticles()[hash->queryIds[nr]];
                if(pi->id != pj->id){
                    double p_ij;
                    if(pj->type == ParticleType::Boundary){
                         p_ij = getPijMeanDensitySquareBoundary(pi,pj);
                    } else {
                        p_ij = getPijMeanDensitySquare(pi,pj);
                    }
                    Vec3 k = getKernelFunctionGradientSpiky(pi->pos-pj->pos,h);
                    //Vec3 k = getKernelFunctionGradientCubicSpline(pi->pos-pj->pos,h);
                    if(k != Vec3(0.f,0.f,0.f)) a_pressure += p_ij*k;
                }
            }
            Vec3 fSPHPressure = pi->mass*a_pressure;
            if(pi->type == ParticleType::NotBoundary){
                pi->vel += dt/pi->mass*(fSPHPressure);
                pi->pos += dt*pi->vel;
            }
        }
    } else if (widget->getSPHMethod() == SPHMethod::IterativeWeaklyCompressible){

        // 1. for all particle i compute non-pressure accel
        double h = sqrt(water_radius*water_radius*4.f + water_radius*water_radius*4.f)*widget->getHReduction();
        double v = widget->getKinematicViscosity();
        double p0 = widget->getRestDensity();
        for(int i=0; i<system.getNumParticles();i++) {
            Particle *pi = system.getParticles()[i];
            // find neighbors
            hash->query(pi->pos,h);

            Vec3 laplacian_velocity = Vec3(0.f,0.f,0.f);
            for(unsigned int nr=0; nr<hash->querySize;nr++){
                Particle *pj = system.getParticles()[hash->queryIds[nr]];
                if(pi->id != pj->id){
                    Vec3 v_ij = getVijMeanDensitySquare(pi,pj);
                    double k = getKernelFunctionLaplacianViscosity(pi->pos-pj->pos,h);
                    //double k = getKernelFunctionLaplacianViscosityImproved(pi->pos-pj->pos,h);
                    if(k) laplacian_velocity += v_ij*k;
                }
            }
            Vec3 aSPHViscosity = v*laplacian_velocity;
            pi->vel += dt*(aSPHViscosity+fGravity->getAcceleration());
        }

        // 2. for all particle i iterate until density i  similar to density 0, or iter_max
        int iter_max = 0;
        while(iter_max<2){
            iter_max++;
            for(int i=0; i<system.getNumParticles();i++) {
                Particle *pi = system.getParticles()[i];
                // find neighbors
                hash->query(pi->pos,h);

                // calculate density
                pi->density = 0.f;
                for(unsigned int nr=0; nr<hash->querySize;nr++){
                    Particle *pj = system.getParticles()[hash->queryIds[nr]];
                    double k = getKernelFunctionSpiky(pi->pos-pj->pos,h);
                    if(k) pi->density += pj->mass*k;
                }
                pi->pressure = getPressureFunctionStateEquation(pi->density,p0);
            }

            for(int i=0; i<system.getNumParticles();i++) {
                Particle *pi = system.getParticles()[i];
                hash->query(pi->pos,h);

                Vec3 g_pressure = Vec3(0.f,0.f,0.f);
                for(unsigned int nr=0; nr<hash->querySize;nr++){
                    Particle *pj = system.getParticles()[hash->queryIds[nr]];
                    if(pi->id != pj->id){
                        double p_ij;
                        if(pj->type == ParticleType::Boundary){
                             p_ij = getPijMeanDensitySquareBoundary(pi,pj);
                        } else {
                            p_ij = getPijMeanDensitySquare(pi,pj);
                        }
                        Vec3 k = getKernelFunctionGradientSpiky(pi->pos-pj->pos,h);
                        //Vec3 k = getKernelFunctionGradientCubicSpline(pi->pos-pj->pos,h);
                        if(k != Vec3(0.f,0.f,0.f)) g_pressure += p_ij*k;
                    }
                }
                pi->vel -= dt*(g_pressure/pi->density);
            }
        }


        // 3. for all particle i update pos
        for(int i=0; i<system.getNumParticles();i++) {
            Particle *pi = system.getParticles()[i];
            if(pi->type == ParticleType::NotBoundary){
                pi->pos += dt*pi->vel;
            }
        }
    }




    // collisions
    for (int i=0; i<system.getNumParticles();i++) {
        Particle* pi = system.getParticles()[i];
        // Floor collider
        if (colliderFloor.testCollision(pi)) {
            colliderFloor.resolveCollision(pi, bouncing, friction, dt);
        }
        // Sphere collider
        if (colliderSphere.testCollision(pi)) {
            colliderSphere.resolveCollision(pi, bouncing, friction, dt);
        }

        // AABB collider: 3 times in case of corners
        if (colliderCube.testCollision(pi)) {
            colliderCube.resolveCollision(pi, bouncing, friction, dt);
        }
        if (colliderCube.testCollision(pi)) {
            colliderCube.resolveCollision(pi, bouncing, friction, dt);
        }
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
