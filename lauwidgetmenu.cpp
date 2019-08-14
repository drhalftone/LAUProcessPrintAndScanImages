#include "lauwidgetmenu.h"

#include "laucombineimagestopdfwidget.h"
#include "lausplitimagestotiffwidget.h"
#include "laufindgridbatchwidget.h"
#include "lauimagematchingwidget.h"

LAUWidgetMenu::LAUWidgetMenu(QWidget *parent) : QWidget(parent)
{
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(6, 6, 6, 6);
    this->layout()->setSpacing(6);

    QPushButton *button = new QPushButton(QString("Create Targets"));
    connect(button, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
    this->layout()->addWidget(button);

    button = new QPushButton(QString("Parse Scans"));
    connect(button, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
    this->layout()->addWidget(button);

    button = new QPushButton(QString("Match Images"));
    connect(button, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
    this->layout()->addWidget(button);
}

void LAUWidgetMenu::onButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button != nullptr) {
        QString string = button->text();
        if (string == QString("Create Targets")) {
            LAUCombineImagesToPDFDialog dialog;
            if (dialog.isValid()) {
                dialog.exec();
            }
        } else if (string == QString("Parse Scans")) {
            LAUSplitImagesToTiffDialog dialog;
            if (dialog.isValid()) {
                dialog.exec();
            }
        } else if (string == QString("Match Images")) {
            LAUImageMatchDialog dialog;
            if (dialog.isValid()) {
                dialog.exec();
            }
        }
    }
}
