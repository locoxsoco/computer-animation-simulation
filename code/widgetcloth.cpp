#include "widgetcloth.h"
#include "ui_widgetcloth.h"

WidgetCloth::WidgetCloth(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetCloth)
{
    ui->setupUi(this);

    connect(ui->btnUpdate, &QPushButton::clicked, this,
            [=] (void) { emit updatedParameters(); });
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
    return ui->radioButton->isChecked()?0:ui->radioButton_2->isChecked()?1:2;
}
