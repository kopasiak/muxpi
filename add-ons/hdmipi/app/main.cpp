#include "rmon.h"
#include <QApplication>
#include <QtCore/QCoreApplication>
#include <QtWidgets/QApplication>

#include <VLCQtCore/Common.h>

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("RemoteMonitor");
    QCoreApplication::setAttribute(Qt::AA_X11InitThreads);

    QApplication app(argc, argv);
    VlcCommon::setPluginPath(app.applicationDirPath() + "/plugins");

    app.setOverrideCursor(QCursor(Qt::ArrowCursor));

    RMon w;
    w.show();

    return app.exec();
}
