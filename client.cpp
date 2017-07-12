#include <QtGui>
#include <QtNetwork>

#include "client.h"

Client::Client(QWidget *parent)
    :   QDialog(parent)
{
    hostLabel = new QLabel(tr("&Server name:"));
    portLabel = new QLabel(tr("S&erver port:"));

    // find out which IP to connect to
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

    hostLineEdit = new QLineEdit(ipAddress);
    portLineEdit = new QLineEdit;
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    hostLabel->setBuddy(hostLineEdit);
    portLabel->setBuddy(portLineEdit);

    statusLabel = new QLabel(tr("This examples requires that you run the "
                                "Fortune Server example as well."));

    connectButton = new QPushButton(tr("Connect"));
    connectButton->setDefault(true);
    connectButton->setEnabled(false);

    getFortuneButton = new QPushButton(tr("Get Fortune"));
    getFortuneButton->setDefault(true);
    getFortuneButton->setEnabled(false);

    quitButton = new QPushButton(tr("Quit"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(connectButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(getFortuneButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    connect(hostLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableGetFortuneButton()));
    connect(portLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableGetFortuneButton()));    
    connect(connectButton, SIGNAL(clicked()),
            this, SLOT(clickConnect()));
    connect(getFortuneButton, SIGNAL(clicked()),
            this, SLOT(clickConnect()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));


    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostLineEdit, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(statusLabel, 2, 0, 1, 2);
    mainLayout->addWidget(buttonBox, 3, 0, 1, 2);
    setLayout(mainLayout);

    setWindowTitle(tr("Fortune Client"));
    portLineEdit->setFocus();


}

void Client::clickConnect(){
     vectfe = new VectFontEnd(hostLineEdit->text(), portLineEdit->text().toInt(), "hungdd", "123456", 1);

     connect(vectfe, SIGNAL(exportFortune(QString)), this, SLOT(readFortune(QString)));
     connect(vectfe, SIGNAL(exportError(QString)), this, SLOT(displayError(QString)));

     //addnew
     connect(vectfe, SIGNAL(exportRequestNewFortune()), this, SLOT(requestNewFortune()));
     connect(vectfe, SIGNAL(exportSessionOpened()), this, SLOT(sessionOpened()));
}

void Client::requestNewFortune()
{
    statusLabel->setText(tr("Opening network session."));   

    //getFortuneButton->setEnabled(false);
    /*
    blockSize = 0;
    tcpSocket->abort();
    tcpSocket->connectToHost(hostLineEdit->text(),
                             portLineEdit->text().toInt());*/
}

void Client::readFortune(QString currentFortune)
{
    statusLabel->setText(currentFortune);
    getFortuneButton->setEnabled(true);
}

void Client::displayError(QString errMsg)
{

    QMessageBox::information(this, tr("Fortune Client"), errMsg);


    getFortuneButton->setEnabled(true);
}

void Client::enableGetFortuneButton()
{
    //    getFortuneButton->setEnabled((!networkSession || networkSession->isOpen()) &&
    //                                 !hostLineEdit->text().isEmpty() &&
    //                                 !portLineEdit->text().isEmpty());

    getFortuneButton->setEnabled(!hostLineEdit->text().isEmpty() && !portLineEdit->text().isEmpty());
    connectButton->setEnabled(!hostLineEdit->text().isEmpty() && !portLineEdit->text().isEmpty());

}

void Client::sessionOpened()
{
    // Save the used configuration
//    QNetworkConfiguration config = networkSession->configuration();
//    QString id;
//    if (config.type() == QNetworkConfiguration::UserChoice)
//        id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
//    else
//        id = config.identifier();

//    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
//    settings.beginGroup(QLatin1String("QtNetwork"));
//    settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
//    settings.endGroup();

    statusLabel->setText(tr("This examples requires that you run the "
                            "Fortune Server example as well."));

    enableGetFortuneButton();
}
