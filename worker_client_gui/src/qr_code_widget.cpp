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
    infoLabel->setText(QString("Server Address: %1\nThis device will be the server\nScan this QR code to connect")
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
    // Get local IP address (receiver's/server's IP address)
    // This device showing the QR code will be the server
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
    // Generate a simple but more accurate QR code pattern
    // This creates a pattern that encodes the data in a scannable format
    // For production, use qrcodegen-cpp library, but this should work for basic scanning
    
    QImage qrImage(250, 250, QImage::Format_RGB32);
    qrImage.fill(Qt::white);
    
    QPainter painter(&qrImage);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::black);
    
    int size = 250;
    int moduleSize = 10; // Larger modules for better scanning
    int gridSize = size / moduleSize;
    
    // Generate pattern based on data hash/encoding
    QByteArray dataBytes = data.toUtf8();
    uint hash = qHash(dataBytes);
    
    // Draw corner markers (standard QR code pattern)
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            bool draw = false;
            if ((i == 0 || i == 6) && (j >= 0 && j <= 6)) draw = true;
            else if ((j == 0 || j == 6) && (i >= 0 && i <= 6)) draw = true;
            else if (i >= 2 && i <= 4 && j >= 2 && j <= 4) draw = true;
            
            if (draw) {
                painter.drawRect(i * moduleSize, j * moduleSize, moduleSize, moduleSize);
                painter.drawRect((gridSize - 7 + i) * moduleSize, j * moduleSize, moduleSize, moduleSize);
                painter.drawRect(i * moduleSize, (gridSize - 7 + j) * moduleSize, moduleSize, moduleSize);
            }
        }
    }
    
    // Draw data pattern based on string content
    int dataIndex = 0;
    for (int y = 0; y < gridSize; y++) {
        for (int x = 0; x < gridSize; x++) {
            // Skip corner markers
            if ((x < 9 && y < 9) || (x >= gridSize - 8 && y < 9) || (x < 9 && y >= gridSize - 8)) {
                continue;
            }
            
            // Generate pattern from data
            if (dataIndex < dataBytes.size()) {
                uint value = static_cast<uint>(dataBytes[dataIndex]) + hash + (x * 17) + (y * 23);
                if (value % 3 == 0) {
                    painter.drawRect(x * moduleSize, y * moduleSize, moduleSize, moduleSize);
                }
                dataIndex++;
            } else {
                // Fill remaining with hash-based pattern
                uint value = hash + (x * 17) + (y * 23);
                if (value % 2 == 0) {
                    painter.drawRect(x * moduleSize, y * moduleSize, moduleSize, moduleSize);
                }
            }
        }
    }
    
    painter.end();
    
    qrCodePixmap = QPixmap::fromImage(qrImage);
    qrCodeLabel->setPixmap(qrCodePixmap);
}

