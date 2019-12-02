#include "lauwidgetmenu.h"

#include "laucombineimagestopdfwidget.h"
#include "lausplitimagestotiffwidget.h"
#include "laufindgridbatchwidget.h"
#include "lauimagematchingwidget.h"
#include "lauapplydnnwidget.h"

LAUWidgetMenu::LAUWidgetMenu(QWidget *parent) : QWidget(parent)
{
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(6, 6, 6, 6);
    this->layout()->setSpacing(6);

    QPushButton *button = new QPushButton(QString("Set Directories"));
    connect(button, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
    this->layout()->addWidget(button);
    buttons << button;

    bool flag = LAUDefaultDirectoriesDialog::isValid();

    button = new QPushButton(QString("Create Targets"));
    button->setEnabled(flag);
    connect(button, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
    this->layout()->addWidget(button);
    buttons << button;

    button = new QPushButton(QString("Parse Scans"));
    button->setEnabled(flag);
    connect(button, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
    this->layout()->addWidget(button);
    buttons << button;

    button = new QPushButton(QString("Match Images"));
    button->setEnabled(flag);
    connect(button, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
    this->layout()->addWidget(button);
    buttons << button;

    button = new QPushButton(QString("Score Image"));
    connect(button, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
    this->layout()->addWidget(button);
    buttons << button;
}

void LAUWidgetMenu::onButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button != nullptr) {
        QString string = button->text();
        if (string == QString("Set Directories")) {
            LAUDefaultDirectoriesDialog dialog;
            if (dialog.exec()) {
                bool flag = LAUDefaultDirectoriesDialog::isValid();
                for (int n = 1; n < buttons.count() - 1; n++) {
                    buttons.at(n)->setEnabled(flag);
                }
            }
        } else if (string == QString("Create Targets")) {
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
        } else if (string == QString("Score Image")) {
            LAUApplyDNNDialog dialog;
            if (dialog.isValid()) {
                dialog.exec();
            }
        }
    }
}
