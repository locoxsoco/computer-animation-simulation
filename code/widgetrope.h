#ifndef WIDGETROPE_H
#define WIDGETROPE_H

#include <QWidget>

namespace Ui {
class WidgetRope;
}

class WidgetRope : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetRope(QWidget *parent = nullptr);
    ~WidgetRope();

    double getGravity()    const;
    int getBlackholeIntensity()    const;
    int getMovableObjectId() const;

signals:
    void updatedParameters();

private:
    Ui::WidgetRope *ui;
};

#endif // WIDGETROPE_H
