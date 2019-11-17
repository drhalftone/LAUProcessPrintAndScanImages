#include "laudefaultdirectorieswidget.h"

#include <QDir>
#include <QSettings>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDialogButtonBox>

QString LAUDefaultDirectoriesDialog::sourceImageDirectory;
QString LAUDefaultDirectoriesDialog::prestineSheetsDirectory;
QString LAUDefaultDirectoriesDialog::prestineThumbnailDirectory;
QString LAUDefaultDirectoriesDialog::printedSheetsDirectory;
QString LAUDefaultDirectoriesDialog::printedThumbnailsDirectory;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUDefaultDirectoriesDialog::LAUDefaultDirectoriesDialog(QWidget *parent) : QDialog(parent), widget(nullptr)
{
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(6, 6, 6, 6);
    this->setWindowTitle(QString("Default Directories"));

    QStringList keys;
    keys << QString("Source Image Directory");
    keys << QString("Prestine Sheets Directory");
    keys << QString("Prestine Thumbnail Directory");
    keys << QString("Printed Sheets Directory");
    keys << QString("Printed Thumbnails Directory");

    QStringList tags;
    tags << sourceImageDirectory;
    tags << prestineSheetsDirectory;
    tags << prestineThumbnailDirectory;
    tags << printedSheetsDirectory;
    tags << printedThumbnailsDirectory;

    widget = new LAUDefaultDirectoriesWidget(keys, tags);
    this->layout()->addWidget(widget);
    ((QVBoxLayout *)(this->layout()))->addStretch();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
    this->layout()->addWidget(buttonBox);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUDefaultDirectoriesDialog::isValid()
{
    if (QDir().exists(sourceImageDirectory)) {
        if (QDir().exists(prestineSheetsDirectory)) {
            if (QDir().exists(prestineThumbnailDirectory)) {
                if (QDir().exists(printedSheetsDirectory)) {
                    if (QDir().exists(printedThumbnailsDirectory)) {
                        return (true);
                    }
                }
            }
        }
    }
    return (false);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUDefaultDirectoriesDialog::load()
{
    QSettings settings;
    sourceImageDirectory = settings.value(QString("LAUDefaultDirectoriesDialog::sourceImageDirectory"), sourceImageDirectory).toString();
    if (QDir().exists(sourceImageDirectory) == false) {
        sourceImageDirectory = QString();
    }

    prestineSheetsDirectory = settings.value(QString("LAUDefaultDirectoriesDialog::prestineSheetsDirectory"), prestineSheetsDirectory).toString();
    if (QDir().exists(prestineSheetsDirectory) == false) {
        prestineSheetsDirectory = QString();
    }

    prestineThumbnailDirectory = settings.value(QString("LAUDefaultDirectoriesDialog::prestineThumbnailDirectory"), prestineThumbnailDirectory).toString();
    if (QDir().exists(prestineThumbnailDirectory) == false) {
        prestineThumbnailDirectory = QString();
    }

    printedSheetsDirectory = settings.value(QString("LAUDefaultDirectoriesDialog::printedSheetsDirectory"), printedSheetsDirectory).toString();
    if (QDir().exists(printedSheetsDirectory) == false) {
        printedSheetsDirectory = QString();
    }

    printedThumbnailsDirectory = settings.value(QString("LAUDefaultDirectoriesDialog::printedThumbnailsDirectory"), printedThumbnailsDirectory).toString();
    if (QDir().exists(printedThumbnailsDirectory) == false) {
        printedThumbnailsDirectory = QString();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUDefaultDirectoriesDialog::save()
{
    QSettings settings;
    settings.setValue(QString("LAUDefaultDirectoriesDialog::sourceImageDirectory"), sourceImageDirectory);
    settings.setValue(QString("LAUDefaultDirectoriesDialog::prestineSheetsDirectory"), prestineSheetsDirectory);
    settings.setValue(QString("LAUDefaultDirectoriesDialog::prestineThumbnailDirectory"), prestineThumbnailDirectory);
    settings.setValue(QString("LAUDefaultDirectoriesDialog::printedSheetsDirectory"), printedSheetsDirectory);
    settings.setValue(QString("LAUDefaultDirectoriesDialog::printedThumbnailsDirectory"), printedThumbnailsDirectory);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUDefaultDirectoriesDialog::accept()
{
    QStringList tags = widget->directories();
    sourceImageDirectory = tags.at(0);
    prestineSheetsDirectory = tags.at(1);
    prestineThumbnailDirectory = tags.at(2);
    printedSheetsDirectory = tags.at(3);
    printedThumbnailsDirectory = tags.at(4);
    save();

    QDialog::accept();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUDefaultDirectoriesWidget::LAUDefaultDirectoriesWidget(QStringList kys, QStringList tgs, QWidget *parent) : QWidget(parent), keys(kys), tags(tgs)
{
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(0, 0, 0, 0);
    this->layout()->setSpacing(0);

    for (int n = 0; n < kys.count(); n++) {
        QWidget *widget = new QWidget();
        widget->setLayout(new QHBoxLayout());
        widget->layout()->setContentsMargins(0, 0, 0, 0);

        QLabel *label = new QLabel(keys.at(n));
        widget->layout()->addWidget(label);
        labels << label;

        QPushButton *pushButton = new QPushButton(QString("SET:"));
        widget->layout()->addWidget(pushButton);
        connect(pushButton, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
        pushButtons << pushButton;

        QLineEdit *lineEdit = new QLineEdit();
        widget->layout()->addWidget(lineEdit);
        lineEdits << lineEdit;

        this->layout()->addWidget(widget);
    }

    // LOAD PREVIOUS TAGS, OVERWRITING WITH USER SUPPLIED TAGS, IF THEY EXIST
    for (int n = 0; n < keys.count(); n++) {
        if (n < tags.count()) {
            lineEdits.at(n)->setText(tags.at(n));
        } else {
            tags << lineEdits.at(n)->text();
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUDefaultDirectoriesWidget::onButtonClicked()
{
    for (int n = 0; n < pushButtons.count(); n++) {
        if (QObject::sender() == pushButtons.at(n)) {
            QSettings settings;
            QString directory = tags.at(n);
            if (QDir().exists(directory) == false) {
                directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
            }
            directory = QFileDialog::getExistingDirectory(this, QString("Set %1 directory...").arg(keys.at(n)), directory);
            if (directory.isEmpty() == false) {
                tags.replace(n, directory);
                lineEdits.at(n)->setText(directory);
            }
            return;
        }
    }
}
