#include "widgetcloth.h"
#include "ui_widgetcloth.h"

WidgetCloth::WidgetCloth(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetCloth)
{
    ui->setupUi(this);

    connect(ui->btnUpdate, &QPushButton::clicked, this,
            [=] (void) { emit updatedParameters(); });
    connect(ui->pushButton_release_lock, &QPushButton::clicked, this,
            [=] (void) { emit releasedLockedParticles(); });
}

WidgetCloth::~WidgetCloth()
{
    delete ui;
}

double WidgetCloth::getGravity() const {
    return ui->gravity->value();
}

int WidgetCloth::getBlackholeIntensity() const {
    return ui->horizontalSlider->value();
}

int WidgetCloth::getMovableObjectId() const {
    return ui->radioButton->isChecked()?0:ui->radioButton_2->isChecked()?1:ui->radioButton_3->isChecked()?2:3;
}

double WidgetCloth::getKe() const {
    return ui->spinBox_ke->value();
}

double WidgetCloth::getKd() const {
    return ui->spinBox_kd->value();
}

int WidgetCloth::getRelaxationSteps() const {
    return ui->spinBox_relaxation_steps->value();
}

bool WidgetCloth::getRenderParticles() const {
    return ui->checkBox_particles->isChecked();
}

bool WidgetCloth::getRenderCloth() const {
    return ui->checkBox_cloth->isChecked();
}

bool WidgetCloth::getSelfCollisions() const {
    return ui->checkBox_selfcollisions->isChecked();
}
