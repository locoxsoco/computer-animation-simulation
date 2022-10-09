#ifndef WIDGETSNOWBALL_H
#define WIDGETSNOWBALL_H

#include <QWidget>

namespace Ui {
class WidgetSnowball;
}

class WidgetSnowball : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetSnowball(QWidget *parent = nullptr);
    ~WidgetSnowball();

    double getGravity()    const;
    int getBlackholeIntensity()    const;
    int getMovableObjectId() const;

signals:
    void updatedParameters();

private:
    Ui::WidgetSnowball *ui;
};

#endif // WIDGETSNOWBALL_H
