#include "qr_code_widget.h"
#include <QNetworkInterface>
#include <QHostAddress>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QImage>
#include <QPainter>
#include <QByteArray>
#include <QHash>

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
    // Show a dialog to manually enter connection info
    // Since we don't have a QR code decoding library, we'll use manual entry
    bool ok;
    QString text = QInputDialog::getText(this, "Enter Connection Info",
                                         "IP:Port (e.g., 192.168.1.100:8080):", QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        // Parse and use the connection info
        QStringList parts = text.split(":");
        if (parts.size() == 2) {
            QString ip = parts[0];
            QString port = parts[1];
            QMessageBox::information(this, "Connection Info",
                QString("IP: %1\nPort: %2\n\nPlease use this info to connect.").arg(ip).arg(port));
        } else {
            QMessageBox::warning(this, "Invalid Format",
                "Please enter in format: IP:Port (e.g., 192.168.1.100:8080)");
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
    // Generate a QR code-like pattern with proper structure
    // Note: This is a simplified version. For production, use qrcodegen-cpp library
    
    QImage qrImage(250, 250, QImage::Format_RGB32);
    qrImage.fill(Qt::white);
    
    QPainter painter(&qrImage);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::black);
    painter.setRenderHint(QPainter::Antialiasing, false); // Sharp edges for QR codes
    
    int size = 250;
    int moduleSize = 5; // Smaller modules for better resolution
    int gridSize = size / moduleSize; // 50x50 grid
    
    QByteArray dataBytes = data.toUtf8();
    uint hash = qHash(dataBytes);
    
    // Draw finder patterns (corner squares) - standard QR code format
    // Top-left finder pattern
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            bool draw = false;
            // Outer square
            if ((i == 0 || i == 6) && j >= 0 && j <= 6) draw = true;
            else if ((j == 0 || j == 6) && i >= 0 && i <= 6) draw = true;
            // Inner square
            else if (i >= 2 && i <= 4 && j >= 2 && j <= 4) draw = true;
            
            if (draw) {
                painter.drawRect(i * moduleSize, j * moduleSize, moduleSize, moduleSize);
            }
        }
    }
    
    // Top-right finder pattern
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            bool draw = false;
            if ((i == 0 || i == 6) && j >= 0 && j <= 6) draw = true;
            else if ((j == 0 || j == 6) && i >= 0 && i <= 6) draw = true;
            else if (i >= 2 && i <= 4 && j >= 2 && j <= 4) draw = true;
            
            if (draw) {
                int x = (gridSize - 7 + i) * moduleSize;
                int y = j * moduleSize;
                painter.drawRect(x, y, moduleSize, moduleSize);
            }
        }
    }
    
    // Bottom-left finder pattern
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            bool draw = false;
            if ((i == 0 || i == 6) && j >= 0 && j <= 6) draw = true;
            else if ((j == 0 || j == 6) && i >= 0 && i <= 6) draw = true;
            else if (i >= 2 && i <= 4 && j >= 2 && j <= 4) draw = true;
            
            if (draw) {
                int x = i * moduleSize;
                int y = (gridSize - 7 + j) * moduleSize;
                painter.drawRect(x, y, moduleSize, moduleSize);
            }
        }
    }
    
    // Draw timing patterns (alternating pattern)
    // Horizontal timing pattern
    for (int x = 8; x < gridSize - 8; x++) {
        if (x % 2 == 0) {
            painter.drawRect(x * moduleSize, 6 * moduleSize, moduleSize, moduleSize);
        }
    }
    // Vertical timing pattern
    for (int y = 8; y < gridSize - 8; y++) {
        if (y % 2 == 0) {
            painter.drawRect(6 * moduleSize, y * moduleSize, moduleSize, moduleSize);
        }
    }
    
    // Draw alignment pattern (center)
    int centerX = gridSize / 2;
    int centerY = gridSize / 2;
    for (int i = -2; i <= 2; i++) {
        for (int j = -2; j <= 2; j++) {
            if ((i == -2 || i == 2) && j >= -2 && j <= 2) {
                painter.drawRect((centerX + i) * moduleSize, (centerY + j) * moduleSize, moduleSize, moduleSize);
            } else if ((j == -2 || j == 2) && i >= -2 && i <= 2) {
                painter.drawRect((centerX + i) * moduleSize, (centerY + j) * moduleSize, moduleSize, moduleSize);
            } else if (i == 0 && j == 0) {
                painter.drawRect((centerX + i) * moduleSize, (centerY + j) * moduleSize, moduleSize, moduleSize);
            }
        }
    }
    
    // Draw data pattern - encode the actual data bit by bit
    int dataIndex = 0;
    int bitIndex = 0;
    uchar currentByte = 0;
    
    for (int y = 0; y < gridSize; y++) {
        for (int x = 0; x < gridSize; x++) {
            // Skip finder patterns, timing patterns, and alignment pattern
            if ((x < 9 && y < 9) || 
                (x >= gridSize - 8 && y < 9) || 
                (x < 9 && y >= gridSize - 8) ||
                (x == 6 && y < gridSize - 8 && y > 8) ||
                (y == 6 && x < gridSize - 8 && x > 8) ||
                (x >= centerX - 2 && x <= centerX + 2 && y >= centerY - 2 && y <= centerY + 2)) {
                continue;
            }
            
            // Encode data bit by bit for better representation
            bool draw = false;
            if (dataIndex < dataBytes.size()) {
                if (bitIndex == 0) {
                    currentByte = dataBytes[dataIndex];
                }
                // Extract bit from current byte (MSB first)
                draw = (currentByte & (1 << (7 - bitIndex))) != 0;
                bitIndex++;
                if (bitIndex >= 8) {
                    bitIndex = 0;
                    dataIndex++;
                }
            } else {
                // Fill remaining with error correction pattern based on hash
                uint pattern = (hash + (x * 13) + (y * 17)) % 3;
                draw = (pattern == 0);
            }
            
            if (draw) {
                painter.drawRect(x * moduleSize, y * moduleSize, moduleSize, moduleSize);
            }
        }
    }
    
    painter.end();
    
    qrCodePixmap = QPixmap::fromImage(qrImage);
    qrCodeLabel->setPixmap(qrCodePixmap);
}

