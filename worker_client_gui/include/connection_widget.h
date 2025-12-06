#ifndef CONNECTION_WIDGET_H
#define CONNECTION_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>

class ConnectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionWidget(QWidget *parent = nullptr);

signals:
    void connectRequested(const QString& address, int port);
    void disconnectRequested();
    void serverModeToggled(bool enabled);

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onServerModeToggled(bool checked);

private:
    void setupUI();
    
    QLineEdit *addressEdit;
    QLineEdit *portEdit;
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    QLabel *statusLabel;
    QCheckBox *serverModeCheckbox;
    bool isConnected;
};

#endif // CONNECTION_WIDGET_H

