#ifndef LAUIMAGEMATCHINGWIDGET_H
#define LAUIMAGEMATCHINGWIDGET_H

#include <QDialog>
#include <QFileDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>

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
        return (objectB);
    }

    LAUMemoryObject rightObject() const
    {
        return (objectA);
    }

    static LAUMemoryObject matchByReduction(LAUMemoryObject objA, LAUMemoryObject objB);
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
class LAUObjectMatchPreview : public QDialog
{
    Q_OBJECT

public:
    explicit LAUObjectMatchPreview(LAUMemoryObject objA, LAUMemoryObject objB, QWidget *parent = nullptr) : QDialog(parent)
    {
        flag = 0;
        pixmapA = QPixmap::fromImage(objA.preview());
        pixmapB = QPixmap::fromImage(objB.preview());

        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("Object Match Preview"));

        label = new QLabel();
        label->setMinimumSize(480, 320);
        label->setPixmap(pixmapB);
        this->layout()->addWidget(label);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(buttonBox);
    }

protected:
    void showEvent(QShowEvent *)
    {
        this->startTimer(1000);
    }

    void timerEvent(QTimerEvent *)
    {
        if (flag == 0) {
            flag = 1;
            label->setPixmap(pixmapA);
            qDebug() << "pixmapA";
        } else {
            flag = 0;
            label->setPixmap(pixmapB);
            qDebug() << "pixmapB";
        }
    }

private:
    int flag;
    QPixmap pixmapA, pixmapB;
    QLabel *label;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUImageMatchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LAUImageMatchDialog(LAUMemoryObject objA, LAUMemoryObject objB, QWidget *parent = nullptr) : QDialog(parent)
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

    LAUImageMatchDialog(QWidget *parent = nullptr);

    bool isValid()
    {
        if (widget) {
            return (widget->isValid());
        }
        return (false);
    }

public slots:
    void onBatchProcessImages();

protected:
    void accept()
    {
        LAUMemoryObject object = widget->matchByReduction(widget->leftObject(), widget->rightObject());
        if (LAUObjectMatchPreview(object, widget->leftObject()).exec() == QDialog::Accepted) {
            if (object.save()) {
                QDialog::accept();
            }
        }
    }

private:
    LAUImageMatchingWidget *widget;
    QFileInfoList prestineFiles;
    QFileInfoList printedFiles;
};

#endif // LAUIMAGEMATCHINGWIDGET_H
