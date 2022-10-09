#include "widgetfountain.h"
#include "ui_widgetfountain.h"

WidgetFountain::WidgetFountain(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetFountain)
{
    ui->setupUi(this);

    connect(ui->btnUpdate, &QPushButton::clicked, this,
            [=] (void) { emit updatedParameters(); });
}

WidgetFountain::~WidgetFountain()
{
    delete ui;
}

double WidgetFountain::getGravity() const {
    return ui->gravity->value();
}

int WidgetFountain::getBlackholeIntensity() const {
    return ui->horizontalSlider->value();
}

int WidgetFountain::getMovableObjectId() const {
    return ui->radioButton->isChecked()?0:ui->radioButton_2->isChecked()?1:2;
}
