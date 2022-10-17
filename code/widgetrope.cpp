#include "widgetrope.h"
#include "ui_widgetrope.h"

WidgetRope::WidgetRope(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetRope)
{
    ui->setupUi(this);

    connect(ui->btnUpdate, &QPushButton::clicked, this,
            [=] (void) { emit updatedParameters(); });
}

WidgetRope::~WidgetRope()
{
    delete ui;
}

double WidgetRope::getGravity() const {
    return ui->gravity->value();
}

int WidgetRope::getBlackholeIntensity() const {
    return ui->horizontalSlider->value();
}

int WidgetRope::getMovableObjectId() const {
    return ui->radioButton->isChecked()?0:ui->radioButton_2->isChecked()?1:2;
}
