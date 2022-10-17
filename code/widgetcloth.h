#ifndef WIDGETCLOTH_H
#define WIDGETCLOTH_H

#include <QWidget>

namespace Ui {
class WidgetCloth;
}

class WidgetCloth : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetCloth(QWidget *parent = nullptr);
    ~WidgetCloth();

    double getGravity()    const;
    int getBlackholeIntensity()    const;
    int getMovableObjectId() const;

signals:
    void updatedParameters();

private:
    Ui::WidgetCloth *ui;
};

#endif // WIDGETCLOTH_H
