#include <QApplication>
#include <QSurfaceFormat>

#include "lauwidgetmenu.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<LAUMemoryObject>("LAUMemoryObject");

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QSurfaceFormat format;
    format.setDepthBufferSize(10);
    format.setMajorVersion(3);
    format.setMinorVersion(3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication a(argc, argv);
    a.setOrganizationName(QString("Lau Consulting Inc"));
    a.setOrganizationDomain(QString("drhalftone.com"));
    a.setApplicationName(QString("LAUCombineImagesToPDF"));
    a.setQuitOnLastWindowClosed(true);

    LAUWidgetMenu w;
    w.show();
    return (a.exec());
}
