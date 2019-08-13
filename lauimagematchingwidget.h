#ifndef LAUIMAGEMATCHINGWIDGET_H
#define LAUIMAGEMATCHINGWIDGET_H

#include "lauimageglwidget.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUImageMatchingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LAUImageMatchingWidget(LAUMemoryObject objA, LAUMemoryObject objB, QWidget *parent = nullptr);

    bool isValid()
    {
        return (objectA.isValid() && objectB.isValid());
    }

    LAUMemoryObject leftObject() const
    {
        return (objectA);
    }

    LAUMemoryObject rightObject() const
    {
        return (objectB);
    }

    static LAUMemoryObject match(LAUMemoryObject objA, LAUMemoryObject objB);
    static QMatrix3x3 homography(LAUMemoryObject objA, LAUMemoryObject objB);

private:
    LAUMemoryObject objectA, objectB;
    LAUImageGLWidget *lftWidget;
    LAUImageGLWidget *rghWidget;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUImageMatchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LAUImageMatchDialog(LAUMemoryObject objA = LAUMemoryObject(), LAUMemoryObject objB = LAUMemoryObject(), QWidget *parent = nullptr) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("Image Matching Dialog"));

        widget = new LAUImageMatchingWidget(objA, objB);
        widget->setMinimumSize(480, 320);
        this->layout()->addWidget(widget);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(buttonBox);

        QPushButton *button = new QPushButton("Batch Process");
        connect(button, SIGNAL(pressed()), this, SLOT(onBatchProcessImages()));
        buttonBox->addButton(button, QDialogButtonBox::ActionRole);
    }

    bool isValid()
    {
        if (widget) {
            return (widget->isValid());
        }
        return (false);
    }

    static LAUMemoryObject batchProcessImages(QStringList inputs, QStringList outputs);

public slots:
    void onBatchProcessImages();

protected:
    void accept()
    {
        LAUMemoryObject object = widget->match(widget->leftObject(), widget->rightObject());
        if (object.save()) {
            QDialog::accept();
        }
    }

private:
    LAUImageMatchingWidget *widget;
};

#endif // LAUIMAGEMATCHINGWIDGET_H
