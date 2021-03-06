#include "lausplitimagestotiffwidget.h"
#include "laudefaultdirectorieswidget.h"

#include <QProgressDialog>
#include <QStandardPaths>
#include <QFileDialog>
#include <QPainter>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUSplitImagesToTiffWidget::LAUSplitImagesToTiffWidget(QString filename, QWidget *parent) : QWidget(parent), fileString(filename)
{
    this->setLayout(new QHBoxLayout());
    this->layout()->setContentsMargins(6, 6, 6, 6);

    QWidget *widget = new QWidget();
    widget->setLayout(new QFormLayout());
    widget->layout()->setContentsMargins(6, 6, 6, 6);

    imageColsSpinBox = new QSpinBox();
    imageColsSpinBox->setFixedWidth(120);
    imageColsSpinBox->setMinimum(1);
    imageColsSpinBox->setMaximum(20);
    imageColsSpinBox->setAlignment(Qt::AlignRight);
    reinterpret_cast<QFormLayout *>(widget->layout())->addRow(QString("Number of image columns:"), imageColsSpinBox);

    imageRowsSpinBox = new QSpinBox();
    imageRowsSpinBox->setFixedWidth(120);
    imageRowsSpinBox->setMinimum(1);
    imageRowsSpinBox->setMaximum(20);
    imageRowsSpinBox->setAlignment(Qt::AlignRight);
    reinterpret_cast<QFormLayout *>(widget->layout())->addRow(QString("Number of image rows:"), imageRowsSpinBox);

    inputResolutionSpinBox = new QSpinBox();
    inputResolutionSpinBox->setFixedWidth(120);
    inputResolutionSpinBox->setMinimum(0);
    inputResolutionSpinBox->setMaximum(10000);
    inputResolutionSpinBox->setAlignment(Qt::AlignRight);
    reinterpret_cast<QFormLayout *>(widget->layout())->addRow(QString("Input image resolution (ppi):"), inputResolutionSpinBox);

    pageWidthSpinBox = new QDoubleSpinBox();
    pageWidthSpinBox->setFixedWidth(120);
    pageWidthSpinBox->setDecimals(4);
    pageWidthSpinBox->setSingleStep(1.0);
    pageWidthSpinBox->setAlignment(Qt::AlignRight);
    reinterpret_cast<QFormLayout *>(widget->layout())->addRow(QString("Page width (in):"), pageWidthSpinBox);

    pageHeightSpinBox = new QDoubleSpinBox();
    pageHeightSpinBox->setFixedWidth(120);
    pageHeightSpinBox->setDecimals(4);
    pageHeightSpinBox->setSingleStep(1.0);
    pageHeightSpinBox->setAlignment(Qt::AlignRight);
    reinterpret_cast<QFormLayout *>(widget->layout())->addRow(QString("Page height (in):"), pageHeightSpinBox);

    this->layout()->addWidget(widget);

    // GET A FILE TO OPEN FROM THE USER IF NOT ALREADY PROVIDED ONE
    if (fileString.isNull()) {
        QSettings settings;
        QString directory = settings.value("LAUSplitImagesToTiffWidget::lastUsedDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        if (QDir(directory).exists() == false) {
            directory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        }
        fileString = QFileDialog::getOpenFileName(nullptr, QString("Load scan from disk (*.tif)"), directory, QString("*.tif;*.tiff"));
        if (fileString.isEmpty() == false) {
            settings.setValue("LAUSplitImagesToTiffWidget::lastUsedDirectory", QFileInfo(fileString).absolutePath());
        } else {
            return;
        }
    }

    // LET THE USER SELECT A FILE FROM THE FILE DIALOG
    localObject = LAUMemoryObject(fileString);

    label = new LAUImageGLWidget(localObject);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->setMinimumSize(480, 480);

    widget = new QWidget();
    widget->setLayout(new QVBoxLayout());
    widget->layout()->setContentsMargins(6, 6, 6, 6);

    widget->layout()->addWidget(label);
    this->layout()->addWidget(widget);

    QSettings settings;
    imageColsSpinBox->setValue(settings.value("LAUSplitImagesToTiffWidget::imageColsSpinBox", 6).toInt());
    imageRowsSpinBox->setValue(settings.value("LAUSplitImagesToTiffWidget::imageRowsSpinBox", 6).toInt());

    // SET THE PARAMETERS FROM THE MEMORY OBJECT
    inputResolutionSpinBox->setValue((double)localObject.resolution());
    pageWidthSpinBox->setValue((double)localObject.width() / (double)localObject.resolution());
    pageWidthSpinBox->setMaximum(pageWidthSpinBox->value());
    pageHeightSpinBox->setValue((double)localObject.height() / (double)localObject.resolution());
    pageHeightSpinBox->setMaximum(pageHeightSpinBox->value());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUSplitImagesToTiffWidget::~LAUSplitImagesToTiffWidget()
{
    if (localObject.isValid()) {
        QSettings settings;
        settings.setValue("LAUSplitImagesToTiffWidget::imageColsSpinBox", imageColsSpinBox->value());
        settings.setValue("LAUSplitImagesToTiffWidget::imageRowsSpinBox", imageRowsSpinBox->value());
        settings.setValue("LAUSplitImagesToTiffWidget::inputResolutionSpinBox", inputResolutionSpinBox->value());
        settings.setValue("LAUSplitImagesToTiffWidget::pageWidthSpinBox", pageWidthSpinBox->value());
        settings.setValue("LAUSplitImagesToTiffWidget::pageHeightSpinBox", pageHeightSpinBox->value());
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUSplitImagesToTiffWidget::processThumbnails()
{
    // GET A LIST OF PRINTED SHEETS FROM THE INPUT DIRECTORY
    QStringList filters;
    filters << "*.TIFF";
    QStringList strings = QDir(LAUDefaultDirectoriesDialog::printedSheetsDirectory).entryList(filters, QDir::Files);

    return (true);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUSplitImagesToTiffDialog::onLoadImages()
{
    // GET A LIST OF IMAGES FROM THE INPUT DIRECTORY
    QStringList filters;
    filters << "*.tif" << "*.tiff" << "*.TIF" << "*.TIFF";
    imageStrings = QDir(LAUDefaultDirectoriesDialog::printedSheetsDirectory).entryList(filters, QDir::Files);

    // PREPEND THE DIRECTORY TO CREATE ABSOLUTE PATH NAMES
    for (int n = 0; n < imageStrings.count(); n++) {
        QString string = QString("%1/%2").arg(LAUDefaultDirectoriesDialog::printedSheetsDirectory).arg(imageStrings.at(n));
        imageStrings.replace(n, string);
    }

    // ENABLE THE OK BUTTON NOW THAT WE HAVE A VALID SOURCE IMAGE DIRECTORY
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}
