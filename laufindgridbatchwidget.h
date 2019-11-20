#ifndef LAUFINDGRIDBATCHWIDGET_H
#define LAUFINDGRIDBATCHWIDGET_H

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
#include <QProgressDialog>
#include <QGuiApplication>
#include <QDialogButtonBox>
#include <QCoreApplication>
#include <QOpenGLFunctions>
#include <QProgressBar>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPixelTransferOptions>

#include "laufindgridglwidget.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUFindGridBatchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LAUFindGridBatchDialog(QStringList strings, QWidget *parent = nullptr);

    ~LAUFindGridBatchDialog()
    {
        if (progress) {
            delete progress;
        }
    }

    bool isValid()
    {
        return (sinkName.isEmpty() == false);
    }

    void setPrintedWidth(double val)
    {
        if (widget) {
            widget->setPrintedWidth(val);
        }
    }

    void setPrintedHeight(double val)
    {
        if (widget) {
            widget->setPrintedHeight(val);
        }
    }

    void setPrintedResolution(double val)
    {
        if (widget) {
            widget->setPrintedResolution(val);
        }
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

public slots:
    void onAbort()
    {
        abortFlag = true;
    }

    void onUpdateObject(LAUMemoryObject obj = LAUMemoryObject());

protected:
    void showEvent(QShowEvent *);

private:
    int counter;
    bool abortFlag;
    QProgressDialog *progress;
    QDialogButtonBox *buttonBox;
    QStringList filenames;
    QString sourceImageDirectory, sinkName;
    LAUFindGridGLWidget *widget;

signals:
    void emitObject(LAUMemoryObject obj);
};

#endif // LAUFINDGRIDBATCHWIDGET_H
