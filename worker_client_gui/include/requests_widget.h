#ifndef REQUESTS_WIDGET_H
#define REQUESTS_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QTimer>

struct RecruitRequest {
    QString id;
    QString name;
    QString address;
    int port;
    long timestamp;
};

class RequestsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RequestsWidget(QWidget *parent = nullptr);
    ~RequestsWidget();

public slots:
    void refreshRequests();
    void acceptRequest(const QString& requestId);
    void rejectRequest(const QString& requestId);

private slots:
    void onRefreshClicked();
    void onAcceptClicked();
    void onRejectClicked();
    void updateRequestsList();

private:
    void setupUI();
    void fetchPendingRequests();
    void parseRequestsResponse(const QByteArray& response);
    
    QPushButton *refreshButton;
    QListWidget *requestsList;
    QLabel *statusLabel;
    QTimer *refreshTimer;
    
    QList<RecruitRequest> pendingRequests;
};

#endif // REQUESTS_WIDGET_H

