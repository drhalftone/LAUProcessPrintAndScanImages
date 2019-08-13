#include <QApplication>
#include <QSurfaceFormat>

#include "laucombineimagestopdfwidget.h"
#include "lausplitimagestotiffwidget.h"
#include "laufindgridbatchwidget.h"
#include "lauimagematchingwidget.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<LAUMemoryObject>("LAUMemoryObject");

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QApplication a(argc, argv);
    a.setOrganizationName(QString("Lau Consulting Inc"));
    a.setOrganizationDomain(QString("drhalftone.com"));
    a.setApplicationName(QString("LAUCombineImagesToPDF"));
    a.setQuitOnLastWindowClosed(true);

    QSurfaceFormat format;
    format.setDepthBufferSize(10);
    format.setMajorVersion(3);
    format.setMinorVersion(3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    QSurfaceFormat::setDefaultFormat(format);

    //LAUCombineImagesToPDFDialog w;
    //LAUSplitImagesToTiffDialog w;
    LAUImageMatchDialog w;
    //LAUFindGridBatchDialog w((QStringList()));
    if (w.isValid()) {
        return (w.exec());
    }
    return (0);
}
