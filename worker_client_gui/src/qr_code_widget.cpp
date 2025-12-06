#include "qr_code_widget.h"
#include <QNetworkInterface>
#include <QHostAddress>
#include <QMessageBox>
#include <QFileDialog>
#include <QImage>
#include <QPainter>

// Simple QR code generation using a basic library or manual implementation
// For production, use a proper QR code library like qrcodegen or zxing

QRCodeWidget::QRCodeWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

QRCodeWidget::~QRCodeWidget()
{
}

void QRCodeWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Title
    QLabel *titleLabel = new QLabel("📱 QR Code Connection", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    showQRButton = new QPushButton("Show My QR Code", this);
    scanQRButton = new QPushButton("Scan QR Code", this);
    
    buttonLayout->addWidget(showQRButton);
    buttonLayout->addWidget(scanQRButton);
    
    // QR Code display
    qrCodeLabel = new QLabel(this);
    qrCodeLabel->setAlignment(Qt::AlignCenter);
    qrCodeLabel->setMinimumSize(250, 250);
    qrCodeLabel->setStyleSheet("border: 1px solid gray; background-color: white;");
    qrCodeLabel->setScaledContents(true);
    qrCodeLabel->hide();
    
    // Info label
    infoLabel = new QLabel("", this);
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setWordWrap(true);
    
    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(qrCodeLabel, 0, Qt::AlignCenter);
    mainLayout->addWidget(infoLabel);
    mainLayout->addStretch();
    
    // Connect signals
    connect(showQRButton, &QPushButton::clicked, this, &QRCodeWidget::onShowQRCodeClicked);
    connect(scanQRButton, &QPushButton::clicked, this, &QRCodeWidget::onScanQRCodeClicked);
}

void QRCodeWidget::onShowQRCodeClicked()
{
    QString connectionInfo = getLocalConnectionInfo();
    generateQRCode(connectionInfo);
    qrCodeLabel->show();
    infoLabel->setText(QString("Connection Info: %1\nScan this QR code to connect to this device")
                       .arg(connectionInfo));
}

void QRCodeWidget::onScanQRCodeClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Select QR Code Image", "", "Image Files (*.png *.jpg *.jpeg)");
    
    if (!fileName.isEmpty()) {
        // Load and decode QR code
        QImage image(fileName);
        if (!image.isNull()) {
            // For now, just show a message
            // In production, use a QR code decoding library
            QMessageBox::information(this, "QR Code Scanned",
                "QR code image loaded. Decoding functionality requires a QR code library.\n"
                "For now, please manually enter the connection info.");
        }
    }
}

QString QRCodeWidget::getLocalConnectionInfo()
{
    // Get local IP address
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    for (const QHostAddress &address : addresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && 
            !address.isLoopback()) {
            return QString("%1:8080").arg(address.toString());
        }
    }
    return "127.0.0.1:8080";
}

void QRCodeWidget::generateQRCode(const QString& data)
{
    // Simple placeholder QR code generation
    // In production, use a proper QR code library like qrcodegen-cpp
    // For now, create a simple visual representation
    
    QImage qrImage(250, 250, QImage::Format_RGB32);
    qrImage.fill(Qt::white);
    
    QPainter painter(&qrImage);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::black);
    
    // Draw a simple pattern (placeholder - use proper QR code library)
    int size = 250;
    int moduleSize = size / 25; // 25x25 grid
    
    // Draw corner markers (simplified)
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            if ((i < 2 || i > 4) && (j < 2 || j > 4)) {
                painter.drawRect(i * moduleSize, j * moduleSize, moduleSize, moduleSize);
                painter.drawRect((size - (i + 1) * moduleSize), j * moduleSize, moduleSize, moduleSize);
                painter.drawRect(i * moduleSize, (size - (j + 1) * moduleSize), moduleSize, moduleSize);
            }
        }
    }
    
    // Draw data pattern (simplified - use proper QR code library)
    for (int i = 0; i < data.length() && i < 100; i++) {
        int x = (i % 20) * moduleSize + 50;
        int y = (i / 20) * moduleSize + 50;
        if (i % 3 == 0) {
            painter.drawRect(x, y, moduleSize, moduleSize);
        }
    }
    
    painter.end();
    
    qrCodePixmap = QPixmap::fromImage(qrImage);
    qrCodeLabel->setPixmap(qrCodePixmap);
}

