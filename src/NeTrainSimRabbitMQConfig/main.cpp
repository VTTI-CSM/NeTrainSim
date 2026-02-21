#include "RabbitMQConfigDialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("NeTrainSim");
    app.setOrganizationName("NeTrainSim");

    RabbitMQConfigDialog dialog;
    dialog.show();

    return app.exec();
}
