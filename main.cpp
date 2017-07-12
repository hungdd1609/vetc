#include <QApplication>
#include "client.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Client client;
#ifdef Q_OS_SYMBIAN
    // Make application better looking and more usable on small screen
    client.showMaximized();
#else
    client.show();
#endif
    return client.exec();
}
