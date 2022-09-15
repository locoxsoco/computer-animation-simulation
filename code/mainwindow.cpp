#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sceneprojectiles.h"
#include "scenefountain.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btnSimstep, SIGNAL(clicked()), ui->openGLWidget, SLOT(doSimStep()));
    connect(ui->btnSimloop, SIGNAL(clicked()), ui->openGLWidget, SLOT(doSimLoop()));
    connect(ui->btnPause,   SIGNAL(clicked()), ui->openGLWidget, SLOT(pauseSim()));
    connect(ui->btnReset,   SIGNAL(clicked()), ui->openGLWidget, SLOT(resetSim()));
    connect(ui->timestep,   SIGNAL(valueChanged(double)), ui->openGLWidget, SLOT(setTimeStep(double)));
    connect(ui->radio_no_drag,   SIGNAL(clicked()), ui->openGLWidget, SLOT(setNoDrag()));
    connect(ui->radio_linear_drag,   SIGNAL(clicked()), ui->openGLWidget, SLOT(setLinearDrag()));
    connect(ui->radio_quadratic_drag,   SIGNAL(clicked()), ui->openGLWidget, SLOT(setQuadraticDrag()));
    ui->openGLWidget->setTimeStep(ui->timestep->value());

    connect(ui->actionCameraReset, SIGNAL(triggered()), ui->openGLWidget, SLOT(resetCamera()));
    connect(ui->actionCameraX, SIGNAL(triggered()), ui->openGLWidget, SLOT(cameraViewX()));
    connect(ui->actionCameraY, SIGNAL(triggered()), ui->openGLWidget, SLOT(cameraViewY()));
    connect(ui->actionCameraZ, SIGNAL(triggered()), ui->openGLWidget, SLOT(cameraViewZ()));

    connect(ui->actionSceneProjectiles,&QAction::triggered, this, [=](void){ changeScene(new SceneProjectiles()); });
    connect(ui->actionSceneFountain,   &QAction::triggered, this, [=](void){ changeScene(new SceneFountain()); });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeScene(Scene* sc)
{
    // First set scene to glwidget. This will trigger deleting old scene and its associated widget.
    // Otherwise, qDeleteAll deletes the widget first, and then the scene destructor crashes
    ui->openGLWidget->setScene(sc);
    qDeleteAll(ui->controlsScene->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly));
    ui->controlsScene->layout()->addWidget(sc->sceneUI());
}
