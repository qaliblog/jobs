#ifndef QR_CODE_WIDGET_H
#define QR_CODE_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>

class QRCodeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QRCodeWidget(QWidget *parent = nullptr);
    ~QRCodeWidget();

private slots:
    void onShowQRCodeClicked();
    void onScanQRCodeClicked();

private:
    void setupUI();
    void generateQRCode(const QString& data);
    QString getLocalConnectionInfo();
    
    QPushButton *showQRButton;
    QPushButton *scanQRButton;
    QLabel *qrCodeLabel;
    QLabel *infoLabel;
    QPixmap qrCodePixmap;
};

#endif // QR_CODE_WIDGET_H

