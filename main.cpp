#include <QApplication>
#include <QSurfaceFormat>

#include "lauwidgetmenu.h"
#include "lauimagematchingwidget.h"

int main(int argc, char *argv[])
{
    QSurfaceFormat format;
    format.setDepthBufferSize(10);
    format.setMajorVersion(3);
    format.setMinorVersion(3);
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

    LAUWidgetMenu w;
    w.show();
    return (a.exec());
}
