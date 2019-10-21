#ifndef LAUFINDGRIDGLWIDGET_H
#define LAUFINDGRIDGLWIDGET_H

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
class LAUFindGridGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    LAUFindGridGLWidget(LAUMemoryObject obj, QWidget *parent = nullptr);
    ~LAUFindGridGLWidget();

    virtual bool isValid() const
    {
        return (wasInitialized());
    }

    bool wasInitialized() const
    {
        return (vertexArrayObject.isCreated());
    }

    void setRows(int val)
    {
        numRows = val;
    }

    void setCols(int val)
    {
        numCols = val;
    }

    unsigned int rows() const
    {
        return (numRows);
    }

    unsigned int cols() const
    {
        return (numCols);
    }

    QTransform mapping() const
    {
        return (QTransform(solution.x(), 1.0, solution.y(), 1.0, solution.z(), solution.w(), 1.0, 1.0, 1.0));
    }

    LAUMemoryObject result(int col = -1, int row = -1);

    void process(LAUMemoryObject obj);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

public slots:
    void onUpdateObject(QString string)
    {
        process(LAUMemoryObject(string));
        qApp->processEvents();
        emit emitObject(object);
    }

    void onUpdateObject(LAUMemoryObject obj)
    {
        process(obj);
        qApp->processEvents();
        emit emitObject(object);
    }

private:
    LAUMemoryObject object;

    QOpenGLPixelTransferOptions options;
    QOpenGLBuffer quadVertexBuffer, quadIndexBuffer;
    QOpenGLVertexArrayObject vertexArrayObject;
    QOpenGLFramebufferObject *frameBufferObjectC;
    QOpenGLShaderProgram programA, programB, programC, programD;
    QVector4D solution;

    bool initializedFlag;
    int localWidth, localHeight, numInds;
    double bestAngle;

    int numRows, numCols;

    double entropy(LAUMemoryObject object);
    QVector4D findGaps(LAUMemoryObject objX, LAUMemoryObject objY);

signals:
    void emitObject(LAUMemoryObject object);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUFindGridDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LAUFindGridDialog(LAUMemoryObject obj, QWidget *parent = nullptr) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("Thumbnail Layout Dialog"));

        widget = new LAUFindGridGLWidget(obj);
        widget->setMinimumSize(480, 320);
        this->layout()->addWidget(widget);

        buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(buttonBox);
    }

    void setRows(int val)
    {
        if (widget) {
            widget->setRows(val);
        }
    }

    void setCols(int val)
    {
        if (widget) {
            widget->setCols(val);
        }
    }

    QTransform mapping() const
    {
        if (widget) {
            return (widget->mapping());
        }
        return (QTransform());
    }

    LAUMemoryObject result(int col = -1, int row = -1)
    {
        return (widget->result(col, row));
    }

protected:
    void accept();

private:
    QDialogButtonBox *buttonBox;
    QString sourceImageDirectory;
    LAUFindGridGLWidget *widget;
};

#endif // LAUFINDGRIDGLWIDGET_H
