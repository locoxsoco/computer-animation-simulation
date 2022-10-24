#ifndef WIDGETOP_H
#define WIDGETOP_H

#include <QWidget>

namespace Ui {
class WidgetOP;
}

class WidgetOP : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetOP(QWidget *parent = nullptr);
    ~WidgetOP();

    double getGravity()    const;
    int getBlackholeIntensity()    const;
    int getMovableObjectId() const;
    double getKe() const;
    double getKd() const;
    int getRelaxationSteps() const;
    bool getRenderParticles() const;
    bool getRenderCloth() const;
    bool getSelfCollisions() const;

signals:
    void updatedParameters();
    void releasedLockedParticles();

private:
    Ui::WidgetOP *ui;
};

#endif // WIDGETOP_H
