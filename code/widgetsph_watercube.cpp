#include "widgetsph_watercube.h"
#include "ui_widgetsph_watercube.h"

WidgetSPHWaterCube::WidgetSPHWaterCube(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetSPHWaterCube)
{
    ui->setupUi(this);

    connect(ui->btnUpdate, &QPushButton::clicked, this,
            [=] (void) { emit updatedParameters(); });
    connect(ui->pushButton_release_lock, &QPushButton::clicked, this,
            [=] (void) { emit releasedLockedParticles(); });
}

WidgetSPHWaterCube::~WidgetSPHWaterCube()
{
    delete ui;
}

double WidgetSPHWaterCube::getGravity() const {
    return ui->gravity->value();
}

int WidgetSPHWaterCube::getBlackholeIntensity() const {
    return ui->horizontalSlider->value();
}

int WidgetSPHWaterCube::getMovableObjectId() const {
    return ui->radioButton->isChecked()?0:ui->radioButton_2->isChecked()?1:ui->radioButton_3->isChecked()?2:3;
}

double WidgetSPHWaterCube::getKe() const {
    return ui->spinBox_ke->value();
}

double WidgetSPHWaterCube::getKd() const {
    return ui->spinBox_kd->value();
}

int WidgetSPHWaterCube::getRelaxationSteps() const {
    return ui->spinBox_relaxation_steps->value();
}

bool WidgetSPHWaterCube::getRenderParticles() const {
    return ui->checkBox_particles->isChecked();
}

bool WidgetSPHWaterCube::getRenderCloth() const {
    return ui->checkBox_cloth->isChecked();
}

bool WidgetSPHWaterCube::getSelfCollisions() const {
    return ui->checkBox_selfcollisions->isChecked();
}
