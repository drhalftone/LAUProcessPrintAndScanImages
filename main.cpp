#include <QApplication>
#include <QSurfaceFormat>

#include "lauwidgetmenu.h"
#include "lauimagematchingwidget.h"

int main(int argc, char *argv[])
{
    QSurfaceFormat format;
    format.setDepthBufferSize(10);
    format.setMajorVersion(4);
    format.setMinorVersion(1);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    QSurfaceFormat::setDefaultFormat(format);

#ifndef Q_OS_LINUX
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QApplication a(argc, argv);
    a.setOrganizationName(QString("Lau Consulting Inc"));
    a.setOrganizationDomain(QString("drhalftone.com"));
    a.setApplicationName(QString("LAUCombineImagesToPDF"));
    a.setQuitOnLastWindowClosed(true);

    qRegisterMetaType<LAUMemoryObject>("LAUMemoryObject");

    LAUDefaultDirectoriesDialog::load();

    //LAUImageMatchDialog k(LAUMemoryObject(QString("/Volumes/StickIT II/PrintedThumbnails/printedThumbnail00000.tif")), LAUMemoryObject(QString("/Volumes/StickIT II/PrestineThumbnails/prestineThumbnail00000.tif")));
    //return k.exec();

    LAUWidgetMenu w;
    w.show();
    return (a.exec());
}
