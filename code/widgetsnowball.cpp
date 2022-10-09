#include "widgetsnowball.h"
#include "ui_widgetsnowball.h"

WidgetSnowball::WidgetSnowball(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetSnowball)
{
    ui->setupUi(this);

    connect(ui->btnUpdate, &QPushButton::clicked, this,
            [=] (void) { emit updatedParameters(); });
}

WidgetSnowball::~WidgetSnowball()
{
    delete ui;
}

double WidgetSnowball::getGravity() const {
    return ui->gravity->value();
}

int WidgetSnowball::getBlackholeIntensity() const {
    return ui->horizontalSlider->value();
}

int WidgetSnowball::getMovableObjectId() const {
    return ui->radioButton->isChecked()?0:1;
}
