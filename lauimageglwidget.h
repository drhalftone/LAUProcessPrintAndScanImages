#ifndef LAUIMAGEGLWIDGET_H
#define LAUIMAGEGLWIDGET_H

#include <QMenu>
#include <QDialog>
#include <QScreen>
#include <QWidget>
#include <QObject>
#include <QSettings>
#include <QMatrix4x4>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPushButton>
#include <QApplication>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QDialogButtonBox>
#include <QCoreApplication>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPixelTransferOptions>

#include "laumemoryobject.h"

#ifndef PI
#define PI 3.14159265359f
#endif

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUImageGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    LAUImageGLWidget(LAUMemoryObject obj, QWidget *parent = nullptr);
    ~LAUImageGLWidget();

    virtual bool isValid() const
    {
        return (wasInitialized());
    }

    bool wasInitialized() const
    {
        return (vertexArrayObject.isCreated());
    }

    LAUMemoryObject result();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    bool initializedFlag;
    int localWidth, localHeight;
    LAUMemoryObject object;

    QOpenGLPixelTransferOptions options;
    QOpenGLBuffer quadVertexBuffer, quadIndexBuffer;
    QOpenGLVertexArrayObject vertexArrayObject;
    QOpenGLShaderProgram program;
    QOpenGLTexture *texture;

signals:
    void emitBuffer(LAUMemoryObject object);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUImageGLDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LAUImageGLDialog(LAUMemoryObject obj, QWidget *parent = nullptr) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("Thumbnail Layout Dialog"));

        LAUImageGLWidget *widget = new LAUImageGLWidget(obj);
        widget->setMinimumSize(480, 320);
        this->layout()->addWidget(widget);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(buttonBox);
    }
};

#endif // LAUIMAGEGLWIDGET_H
