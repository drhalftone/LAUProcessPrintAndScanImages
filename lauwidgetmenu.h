#ifndef LAUWIDGETMENU_H
#define LAUWIDGETMENU_H

#include <QWidget>

#include "laumemoryobject.h"

class LAUWidgetMenu : public QWidget
{
    Q_OBJECT

public:
    explicit LAUWidgetMenu(QWidget *parent = nullptr);

public slots:
    void onButtonClicked();
};

#endif // LAUWIDGETMENU_H
