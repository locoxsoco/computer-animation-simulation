#include "widgetprojectiles.h"
#include "ui_widgetprojectiles.h"

WidgetProjectiles::WidgetProjectiles(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetProjectiles)
{
    ui->setupUi(this);
}

WidgetProjectiles::~WidgetProjectiles()
{
    delete ui;
}

double WidgetProjectiles::getHeight() const {
    return ui->height->value();
}

double WidgetProjectiles::getGravity() const {
    return ui->gravity->value();
}

double WidgetProjectiles::getAngle() const {
    return ui->angle->value();
}

double WidgetProjectiles::getSpeed() const {
    return ui->speed->value();
}

double WidgetProjectiles::getWindX() const {
    return ui->windX->value();
}

double WidgetProjectiles::getWindY() const {
    return ui->windY->value();
}

double WidgetProjectiles::getWindZ() const {
    return ui->windZ->value();
}

int WidgetProjectiles::getSolver1() const {
    return ui->solver1->currentIndex();
}

int WidgetProjectiles::getSolver2() const {
    return ui->solver2->currentIndex();
}

int WidgetProjectiles::getSolver3() const {
    return ui->solver3->currentIndex();
}

bool WidgetProjectiles::renderSameZ() const {
    return ui->sameZ->isChecked();
}

bool WidgetProjectiles::renderTrajectory() const {
    return ui->trajectory->isChecked();
}

void WidgetProjectiles::setSolverTypes(const std::vector<std::string>& solvers) {
    ui->solver1->clear();
    ui->solver2->clear();
    ui->solver3->clear();
    for (const std::string& s : solvers) {
        ui->solver1->addItem(QString::fromStdString(s));
        ui->solver2->addItem(QString::fromStdString(s));
        ui->solver3->addItem(QString::fromStdString(s));
    }
}

void WidgetProjectiles::setSolver1(int idx) {
    ui->solver1->setCurrentIndex(idx);
}

void WidgetProjectiles::setSolver2(int idx) {
    ui->solver2->setCurrentIndex(idx);
}

void WidgetProjectiles::setSolver3(int idx) {
    ui->solver3->setCurrentIndex(idx);
}

