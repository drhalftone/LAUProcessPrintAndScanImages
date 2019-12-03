#ifndef LAUAPPLYDNNWIDGET_H
#define LAUAPPLYDNNWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QDialogButtonBox>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPixelTransferOptions>

#include "laumemoryobject.h"

#define DWTLEVELS    5     // NUMBER OF TIMES WE DECOMPOSE THE SIGNAL USING THE DWT
#define DWTSTEP     32     // 2^DWTLEVELS

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUApplyDNNGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    LAUApplyDNNGLWidget(LAUMemoryObject obj, QWidget *parent = nullptr);
    ~LAUApplyDNNGLWidget();

    virtual bool isValid() const
    {
        return (wasInitialized());
    }

    bool wasInitialized() const
    {
        return (vertexArrayObject.isCreated());
    }

    unsigned int rows() const
    {
        return (numRows);
    }

    unsigned int cols() const
    {
        return (numCols);
    }

    LAUMemoryObject result();

    void process(LAUMemoryObject obj);

    virtual void updateBuffer(LAUMemoryObject obj);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

public slots:
    void onUpdateBuffer(LAUMemoryObject obj)
    {
        updateBuffer(obj);
        emit emitBuffer(obj);
    }

private:
    LAUMemoryObject object;

    QOpenGLPixelTransferOptions options;
    QOpenGLBuffer quadVertexBuffer, quadIndexBuffer;
    QOpenGLVertexArrayObject vertexArrayObject;
    QOpenGLTexture *objectTexture;
    QList<QOpenGLTexture *> filterTextures;
    QOpenGLFramebufferObject *frameBufferObjectA, *frameBufferObjectB;
    QOpenGLFramebufferObject *frameBufferObjectC, *frameBufferObjectD;
    QOpenGLShaderProgram program, progLoD, progHiD, progRecon;

    bool initializedFlag;
    int localWidth, localHeight, numInds;

    unsigned int numRows, numCols;

signals:
    void emitBuffer(LAUMemoryObject obj);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUApplyDNNWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LAUApplyDNNWidget(LAUMemoryObject obj = LAUMemoryObject(), QWidget *parent = nullptr);

    bool isValid() const
    {
        return (object.isValid());
    }

    bool saveResult() const
    {
        if (widget) {
            return (widget->result().save());
        }
        return (false);
    }

signals:

public slots:

private:
    LAUMemoryObject object;
    LAUApplyDNNGLWidget *widget;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUApplyDNNDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LAUApplyDNNDialog(LAUMemoryObject obj = LAUMemoryObject(), QWidget *parent = nullptr) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("Deep Neural Network Dialog"));

        widget = new LAUApplyDNNWidget(obj);
        this->layout()->addWidget(widget);

        buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(buttonBox);
    }

    bool isValid() const
    {
        if (widget) {
            return (widget->isValid());
        }
        return (false);
    }

protected:
    void accept()
    {
        if (widget->saveResult()) {
            QDialog::accept();
        }
    }

public slots:

private:
    QDialogButtonBox *buttonBox;
    LAUApplyDNNWidget *widget;
};

#endif // LAUAPPLYDNNWIDGET_H
