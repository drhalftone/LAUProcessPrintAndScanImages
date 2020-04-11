#ifndef LAUCOMBINEIMAGESTOPDFWIDGET_H
#define LAUCOMBINEIMAGESTOPDFWIDGET_H

#include <QLabel>
#include <QDialog>
#include <QWidget>
#include <QSpinBox>
#include <QSettings>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>

#include "laumemoryobject.h"
#include "lautiff2pdfobject.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUCombineImagesToPDFLabel : public QLabel
{
    Q_OBJECT

public:
    LAUCombineImagesToPDFLabel(QLabel *parent = nullptr) : QLabel(parent) { ; }

protected:
    void paintEvent(QPaintEvent *);

public slots:
    void onSetRows(int val)
    {
        rows = val;
        update();
    }

    void onSetCols(int val)
    {
        cols = val;
        update();
    }

    void onSetPageWidth(double val)
    {
        pageWidth = val;
        update();
    }

    void onSetPageHeight(double val)
    {
        pageHeight = val;
        update();
    }

    void onSetLeftMargin(double val)
    {
        leftMargin = val;
        update();
    }

    void onSetTopMargin(double val)
    {
        topMargin = val;
        update();
    }

    void onSetGap(double val)
    {
        gap = val;
        update();
    }

private:
    int cols, rows;
    double pageWidth, pageHeight, leftMargin, topMargin, gap;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUCombineImagesToPDFWidget : public QWidget
{
    Q_OBJECT

public:
    LAUCombineImagesToPDFWidget(QWidget *parent = nullptr);
    ~LAUCombineImagesToPDFWidget();

    bool processThumbnails();

    bool isValid()
    {
        return (true);
    }

    LAUMemoryObject simulateNoise(QImage image, QRect position, float resolution);

private:
    LAUMemoryObject noiseObject;

    QSpinBox *imageColsSpinBox;         // NUMBER OF IMAGES PER ROW
    QSpinBox *imageRowsSpinBox;         // NUMERS OF IMAGES PER COLUMN
    QSpinBox *outputResolutionSpinBox;  // RESOLUTION OF OUTPUT IN PIXELS PER INCH
    QComboBox *simulateNoiseCheckBox;   // CHECK BOX TO LET USER SIMULATE PRINT ARTIFACTS
    QDoubleSpinBox *pageWidthSpinBox;   // PAGE WIDTH
    QDoubleSpinBox *pageHeightSpinBox;  // PAGE HEIGHT
    QDoubleSpinBox *leftMarginSpinBox;  // LEFT/RIGHT MARGIN
    QDoubleSpinBox *topMarginSpinBox;   // TOP/BOTTOM MARGIN
    QDoubleSpinBox *spaceBetweenImages; // SPACE BETWEEN IMAGES

    LAUCombineImagesToPDFLabel *label;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUCombineImagesToPDFDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LAUCombineImagesToPDFDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("Thumbnail Layout Dialog"));

        widget = new LAUCombineImagesToPDFWidget();
        this->layout()->addWidget(widget);

        buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(buttonBox);
    }

    bool isValid()
    {
        return (widget->isValid());
    }

    static void preProcessDataSet();

protected:
    void accept()
    {
        if (sourceImageDirectory.isEmpty()) {
            if (widget->processThumbnails()) {
                QDialog::accept();
            }
        }
    }

public slots:
    void onPreProcessImages()
    {
        preProcessDataSet();
    }

private:
    QDialogButtonBox *buttonBox;
    QString sourceImageDirectory;
    LAUCombineImagesToPDFWidget *widget;
};

#endif // LAUCOMBINEIMAGESTOPDFWIDGET_H
