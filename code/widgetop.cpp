#include "widgetop.h"
#include "ui_widgetop.h"

WidgetOP::WidgetOP(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetOP)
{
    ui->setupUi(this);

    connect(ui->btnUpdate, &QPushButton::clicked, this,
            [=] (void) { emit updatedParameters(); });
    connect(ui->pushButton_release_lock, &QPushButton::clicked, this,
            [=] (void) { emit releasedLockedParticles(); });
}

WidgetOP::~WidgetOP()
{
    delete ui;
}

double WidgetOP::getGravity() const {
    return ui->gravity->value();
}

int WidgetOP::getBlackholeIntensity() const {
    return ui->horizontalSlider->value();
}

int WidgetOP::getMovableObjectId() const {
    return ui->radioButton->isChecked()?0:ui->radioButton_2->isChecked()?1:ui->radioButton_3->isChecked()?2:3;
}

double WidgetOP::getKe() const {
    return ui->spinBox_ke->value();
}

double WidgetOP::getKd() const {
    return ui->spinBox_kd->value();
}

int WidgetOP::getRelaxationSteps() const {
    return ui->spinBox_relaxation_steps->value();
}

bool WidgetOP::getRenderParticles() const {
    return ui->checkBox_particles->isChecked();
}

bool WidgetOP::getRenderCloth() const {
    return ui->checkBox_cloth->isChecked();
}

bool WidgetOP::getSelfCollisions() const {
    return ui->checkBox_selfcollisions->isChecked();
}
