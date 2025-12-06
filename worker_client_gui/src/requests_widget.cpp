#include "requests_widget.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QPushButton>
#include <QHBoxLayout>

RequestsWidget::RequestsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &RequestsWidget::refreshRequests);
    refreshTimer->start(5000); // Refresh every 5 seconds
    refreshRequests();
}

RequestsWidget::~RequestsWidget()
{
    if (refreshTimer) {
        refreshTimer->stop();
    }
}

void RequestsWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Header
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("📥 Recruit Requests", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    refreshButton = new QPushButton("Refresh", this);
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(refreshButton);
    
    // Status label
    statusLabel = new QLabel("No pending requests", this);
    
    // Requests list
    requestsList = new QListWidget(this);
    requestsList->setAlternatingRowColors(true);
    
    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(requestsList);
    
    // Connect signals
    connect(refreshButton, &QPushButton::clicked, this, &RequestsWidget::onRefreshClicked);
    connect(requestsList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        QString requestId = item->data(Qt::UserRole).toString();
        if (!requestId.isEmpty()) {
            acceptRequest(requestId);
        }
    });
}

void RequestsWidget::onRefreshClicked()
{
    refreshRequests();
}

void RequestsWidget::refreshRequests()
{
    fetchPendingRequests();
}

void RequestsWidget::fetchPendingRequests()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    
    QNetworkRequest request(QUrl("http://localhost:8080/api/requests"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = manager->get(request);
    
    connect(reply, &QNetworkReply::finished, [this, reply, manager]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            parseRequestsResponse(data);
        } else {
            statusLabel->setText("Error fetching requests");
        }
        reply->deleteLater();
        manager->deleteLater();
    });
}

void RequestsWidget::parseRequestsResponse(const QByteArray& response)
{
    pendingRequests.clear();
    requestsList->clear();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(response, &error);
    
    if (error.error != QJsonParseError::NoError) {
        statusLabel->setText("Error parsing response");
        return;
    }
    
    QJsonObject obj = doc.object();
    QJsonArray requestsArray = obj["requests"].toArray();
    
    for (const QJsonValue &value : requestsArray) {
        QJsonObject reqObj = value.toObject();
        
        RecruitRequest req;
        req.id = reqObj["id"].toString();
        req.name = reqObj["name"].toString();
        req.address = reqObj["address"].toString();
        req.port = reqObj["port"].toInt();
        req.timestamp = reqObj["timestamp"].toVariant().toLongLong();
        
        pendingRequests.append(req);
        
        // Create list item
        QString displayText = QString("%1 (%2) @ %3:%4")
                             .arg(req.name)
                             .arg(req.address)
                             .arg(req.port);
        
        QListWidgetItem *item = new QListWidgetItem(displayText, requestsList);
        item->setData(Qt::UserRole, req.id);
        
        // Add accept/reject buttons as widget
        QWidget *itemWidget = new QWidget();
        QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);
        itemLayout->setContentsMargins(5, 5, 5, 5);
        
        QLabel *label = new QLabel(displayText);
        QPushButton *acceptBtn = new QPushButton("Accept");
        QPushButton *rejectBtn = new QPushButton("Reject");
        
        connect(acceptBtn, &QPushButton::clicked, [this, req]() {
            acceptRequest(req.id);
        });
        connect(rejectBtn, &QPushButton::clicked, [this, req]() {
            rejectRequest(req.id);
        });
        
        itemLayout->addWidget(label);
        itemLayout->addStretch();
        itemLayout->addWidget(acceptBtn);
        itemLayout->addWidget(rejectBtn);
        
        item->setSizeHint(itemWidget->sizeHint());
        requestsList->setItemWidget(item, itemWidget);
    }
    
    statusLabel->setText(QString("Found %1 pending request(s)").arg(pendingRequests.size()));
}

void RequestsWidget::acceptRequest(const QString& requestId)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    
    QJsonObject json;
    json["requestId"] = requestId;
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();
    
    QString serverUrl = "http://localhost:8080/api/requests/accept";
    QNetworkRequest request(QUrl(serverUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = manager->post(request, data);
    
    connect(reply, &QNetworkReply::finished, [this, reply, manager, requestId]() {
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(this, "Success", "Request accepted");
            refreshRequests();
        } else {
            QMessageBox::warning(this, "Error", "Failed to accept request");
        }
        reply->deleteLater();
        manager->deleteLater();
    });
}

void RequestsWidget::rejectRequest(const QString& requestId)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    
    QJsonObject json;
    json["requestId"] = requestId;
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();
    
    QString serverUrl = "http://localhost:8080/api/requests/reject";
    QNetworkRequest request(QUrl(serverUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = manager->post(request, data);
    
    connect(reply, &QNetworkReply::finished, [this, reply, manager, requestId]() {
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(this, "Success", "Request rejected");
            refreshRequests();
        } else {
            QMessageBox::warning(this, "Error", "Failed to reject request");
        }
        reply->deleteLater();
        manager->deleteLater();
    });
}

void RequestsWidget::onAcceptClicked()
{
    QListWidgetItem *item = requestsList->currentItem();
    if (item) {
        QString requestId = item->data(Qt::UserRole).toString();
        acceptRequest(requestId);
    }
}

void RequestsWidget::onRejectClicked()
{
    QListWidgetItem *item = requestsList->currentItem();
    if (item) {
        QString requestId = item->data(Qt::UserRole).toString();
        rejectRequest(requestId);
    }
}

