#include "connection_widget.h"
#include <QIntValidator>

ConnectionWidget::ConnectionWidget(QWidget *parent)
    : QWidget(parent)
    , isConnected(false)
{
    setupUI();
}

void ConnectionWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Server address group
    QGroupBox *addressGroup = new QGroupBox("Server Address", this);
    QVBoxLayout *addressLayout = new QVBoxLayout(addressGroup);
    
    QHBoxLayout *addressRow = new QHBoxLayout();
    addressEdit = new QLineEdit(this);
    addressEdit->setPlaceholderText("192.168.1.100");
    addressEdit->setText("localhost");
    addressRow->addWidget(new QLabel("IP:", this));
    addressRow->addWidget(addressEdit);
    
    QHBoxLayout *portRow = new QHBoxLayout();
    portEdit = new QLineEdit(this);
    portEdit->setPlaceholderText("8080");
    portEdit->setText("8080");
    portEdit->setValidator(new QIntValidator(1, 65535, this));
    portRow->addWidget(new QLabel("Port:", this));
    portRow->addWidget(portEdit);
    
    addressLayout->addLayout(addressRow);
    addressLayout->addLayout(portRow);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    connectButton = new QPushButton("Connect", this);
    disconnectButton = new QPushButton("Disconnect", this);
    disconnectButton->setEnabled(false);
    
    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(disconnectButton);
    buttonLayout->addStretch();
    
    // Server mode checkbox
    serverModeCheckbox = new QCheckBox("Enable Server Mode", this);
    serverModeCheckbox->setToolTip("Allow other devices to connect to this device");
    
    // Status label
    statusLabel = new QLabel("Not connected", this);
    statusLabel->setStyleSheet("color: gray;");
    
    mainLayout->addWidget(addressGroup);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(serverModeCheckbox);
    mainLayout->addWidget(statusLabel);
    mainLayout->addStretch();
    
    // Connect signals
    connect(connectButton, &QPushButton::clicked, this, &ConnectionWidget::onConnectClicked);
    connect(disconnectButton, &QPushButton::clicked, this, &ConnectionWidget::onDisconnectClicked);
    connect(serverModeCheckbox, &QCheckBox::toggled, this, &ConnectionWidget::onServerModeToggled);
}

void ConnectionWidget::onConnectClicked()
{
    QString address = addressEdit->text();
    int port = portEdit->text().toInt();
    
    if (address.isEmpty() || port == 0) {
        statusLabel->setText("Invalid address or port");
        statusLabel->setStyleSheet("color: red;");
        return;
    }
    
    emit connectRequested(address, port);
    
    isConnected = true;
    connectButton->setEnabled(false);
    disconnectButton->setEnabled(true);
    addressEdit->setEnabled(false);
    portEdit->setEnabled(false);
    statusLabel->setText(QString("Connected to %1:%2").arg(address).arg(port));
    statusLabel->setStyleSheet("color: green;");
}

void ConnectionWidget::onDisconnectClicked()
{
    emit disconnectRequested();
    
    isConnected = false;
    connectButton->setEnabled(true);
    disconnectButton->setEnabled(false);
    addressEdit->setEnabled(true);
    portEdit->setEnabled(true);
    statusLabel->setText("Disconnected");
    statusLabel->setStyleSheet("color: gray;");
}

void ConnectionWidget::onServerModeToggled(bool checked)
{
    emit serverModeToggled(checked);
}

