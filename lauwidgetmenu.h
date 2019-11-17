#ifndef LAUWIDGETMENU_H
#define LAUWIDGETMENU_H

#include <QWidget>
#include <QPushButton>

#include "laumemoryobject.h"
#include "laudefaultdirectorieswidget.h"

class LAUWidgetMenu : public QWidget
{
    Q_OBJECT

public:
    explicit LAUWidgetMenu(QWidget *parent = nullptr);

public slots:
    void onButtonClicked();

private:
    QList<QPushButton *> buttons;
};

#endif // LAUWIDGETMENU_H
