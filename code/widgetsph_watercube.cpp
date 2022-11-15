#include "widgetsph_watercube.h"
#include "ui_widgetsph_watercube.h"

enum comboBoxSPHMethod {
    FullyCompressibleMethod = 0,
    WeaklyCompressibleMethod = 1,
    IterativeWeaklyCompressibleMethod = 2,
};

void WidgetSPHWaterCube::setDefaultParameters(){
    if(ui->comboBox->currentIndex() == comboBoxSPHMethod::FullyCompressibleMethod){
        ui->spinBox_h_reduction->setValue(0.95f);
        ui->spinBox_rest_density->setValue(0.002000f);
        ui->spinBox_c->setValue(300.f);
        ui->spinBox_k->setValue(20.f);
        ui->spinBox_kinematic_viscosity->setValue(0.f);
    }else if(ui->comboBox->currentIndex() == comboBoxSPHMethod::WeaklyCompressibleMethod){
        ui->spinBox_h_reduction->setValue(0.7f);
        ui->spinBox_rest_density->setValue(0.004000f);
        ui->spinBox_c->setValue(300.f);
        ui->spinBox_k->setValue(7.f);
        ui->spinBox_kinematic_viscosity->setValue(0.f);
    }else if(ui->comboBox->currentIndex() == comboBoxSPHMethod::IterativeWeaklyCompressibleMethod  ){
        ui->spinBox_h_reduction->setValue(0.7f);
        ui->spinBox_rest_density->setValue(0.004000f);
        ui->spinBox_c->setValue(300.f);
        ui->spinBox_k->setValue(2.f);
        ui->spinBox_kinematic_viscosity->setValue(0.f);
    }
}

WidgetSPHWaterCube::WidgetSPHWaterCube(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetSPHWaterCube)
{
    ui->setupUi(this);

    connect(ui->btnUpdate, &QPushButton::clicked, this,
            [=] (void) { emit updatedParameters();});

    connect(ui->btnDefaultParameters, &QPushButton::clicked, this,
            [=] (void) { setDefaultParameters();});
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
    return ui->radioButton->isChecked()?0:ui->radioButton_2->isChecked()?1:2;
}

double WidgetSPHWaterCube::getHReduction() const {
    return ui->spinBox_h_reduction->value();
}

double WidgetSPHWaterCube::getRestDensity() const {
    return ui->spinBox_rest_density->value();
}

double WidgetSPHWaterCube::getC() const {
    return ui->spinBox_c->value();
}

double WidgetSPHWaterCube::getK() const {
    return ui->spinBox_k->value();
}

double WidgetSPHWaterCube::getKinematicViscosity() const {
    return ui->spinBox_kinematic_viscosity->value();
}

int WidgetSPHWaterCube::getSPHMethod() const {
    return ui->comboBox->currentIndex();
}
