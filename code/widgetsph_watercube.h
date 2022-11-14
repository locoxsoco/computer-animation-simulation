#ifndef WIDGETPSHWATERCUBE_H
#define WIDGETPSHWATERCUBE_H

#include <QWidget>

namespace Ui {
class WidgetSPHWaterCube;
}

class WidgetSPHWaterCube : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetSPHWaterCube(QWidget *parent = nullptr);
    ~WidgetSPHWaterCube();

    double getGravity()    const;
    int getBlackholeIntensity()    const;
    int getMovableObjectId() const;
    int getSPHMethod() const;
    double getHReduction() const;
    double getRestDensity() const;
    double getC() const;
    double getK() const;
    double getKinematicViscosity() const;
    double getMu() const;
    bool getRenderParticles() const;
    bool getRenderCloth() const;
    bool getSelfCollisions() const;
    void setDefaultParameters();

signals:
    void updatedParameters();
    void releasedLockedParticles();

private:
    Ui::WidgetSPHWaterCube *ui;
};

#endif // WIDGETPSHWATERCUBE_H
