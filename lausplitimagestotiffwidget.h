#ifndef LAUSPLITIMAGESTOTIFFWIDGET_H
#define LAUSPLITIMAGESTOTIFFWIDGET_H

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

#include "lauimageglwidget.h"
#include "laufindgridbatchwidget.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUSplitImagesToTiffWidget : public QWidget
{
    Q_OBJECT

public:
    LAUSplitImagesToTiffWidget(QString filename = QString(), QWidget *parent = nullptr);
    ~LAUSplitImagesToTiffWidget();

    void setDirectoryString(QString directory)
    {
        directoryString = directory;
    }

    bool processThumbnails();

    bool isValid()
    {
        return (object().isValid());
    }

    QString filename() const
    {
        return (fileString);
    }

    LAUMemoryObject object() const
    {
        return (localObject);
    }

    int cols() const
    {
        return (imageColsSpinBox->value());
    }

    int rows() const
    {
        return (imageRowsSpinBox->value());
    }

private:
    QString fileString;
    LAUMemoryObject localObject;

    QString directoryString;
    QSpinBox *imageColsSpinBox;         // NUMBER OF IMAGES PER ROW
    QSpinBox *imageRowsSpinBox;         // NUMERS OF IMAGES PER COLUMN
    QSpinBox *inputResolutionSpinBox;   // RESOLUTION OF OUTPUT IN PIXELS PER INCH
    QDoubleSpinBox *pageWidthSpinBox;   // PAGE WIDTH
    QDoubleSpinBox *pageHeightSpinBox;  // PAGE HEIGHT

    LAUImageGLWidget *label;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUSplitImagesToTiffDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LAUSplitImagesToTiffDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("Scan Splicing Dialog"));

        buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(buttonBox);

        // ASK USER TO SPECIFY AN INPUT IMAGE DIRECTORY
        onLoadImages();

        // ADD OUR SPLIT IMAGE WIDGET
        if (imageStrings.count() > 1) {
            mode = ModeBatchProcess;
        } else {
            mode = ModeSingleProcess;
        }

        if (imageStrings.count() > 0) {
            widget = new LAUSplitImagesToTiffWidget(imageStrings.first());
        } else {
            widget = new LAUSplitImagesToTiffWidget();
        }

        reinterpret_cast<QVBoxLayout *>(this->layout())->insertWidget(0, widget);
    }

    bool isValid()
    {
        return (widget->isValid());
    }

protected:
    void accept()
    {
        if (mode == ModeBatchProcess) {
            LAUFindGridBatchDialog dialog(imageStrings, this);
            dialog.setCols(widget->cols());
            dialog.setRows(widget->rows());
            if (dialog.exec() == QDialog::Accepted) {
                QDialog::accept();
            }
        } else if (mode == ModeSingleProcess) {
            LAUFindGridDialog dialog(widget->object(), this);
            dialog.setFilename(widget->filename());
            dialog.setCols(widget->cols());
            dialog.setRows(widget->rows());
            if (dialog.exec() == QDialog::Accepted) {
                QDialog::accept();
            }
        }
    }

public slots:
    void onLoadImages();

private:
    enum Mode { ModeBatchProcess, ModeSingleProcess };

    Mode mode;
    QDialogButtonBox *buttonBox;
    QStringList imageStrings;
    LAUSplitImagesToTiffWidget *widget;
};

#endif // LAUSPLITIMAGESTOTIFFWIDGET_H
