#ifndef LAUCOMBINEIMAGESTOPDFWIDGET_H
#define LAUCOMBINEIMAGESTOPDFWIDGET_H

#include <QLabel>
#include <QDialog>
#include <QWidget>
#include <QSpinBox>
#include <QSettings>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>

#include "laumemoryobject.h"

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

    void setDirectoryString(QString directory)
    {
        directoryString = directory;
    }

    bool processThumbnails();

    bool isValid()
    {
        return (true);
    }

private:
    QString directoryString;
    QSpinBox *imageColsSpinBox;         // NUMBER OF IMAGES PER ROW
    QSpinBox *imageRowsSpinBox;         // NUMERS OF IMAGES PER COLUMN
    QSpinBox *outputResolutionSpinBox;  // RESOLUTION OF OUTPUT IN PIXELS PER INCH
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
    explicit LAUCombineImagesToPDFDialog(QWidget *parent = NULL) : QDialog(parent)
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

        // DISABLE THE OK BUTTON UNTIL AFTER THE USER SETS THE SOURCE IMAGE DIRECTORY
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

        QPushButton *button = new QPushButton("Set Source Directory");
        connect(button, SIGNAL(pressed()), this, SLOT(onLoadImages()));
        buttonBox->addButton(button, QDialogButtonBox::ActionRole);

        button = new QPushButton("Preprocess");
        connect(button, SIGNAL(pressed()), this, SLOT(onPreProcessImages()));
        buttonBox->addButton(button, QDialogButtonBox::ActionRole);
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
    void onLoadImages();
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
