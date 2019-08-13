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

    bool isValid()
    {
        return (sinkName.isEmpty() == false);
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

private:
    int counter;
    bool abortFlag;
    QProgressBar *progress;
    QDialogButtonBox *buttonBox;
    QStringList filenames;
    QString sourceImageDirectory, sinkName;
    LAUFindGridGLWidget *widget;

signals:
    void emitObject(LAUMemoryObject obj);
};

#endif // LAUFINDGRIDBATCHWIDGET_H
