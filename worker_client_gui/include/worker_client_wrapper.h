#ifndef WORKER_CLIENT_WRAPPER_H
#define WORKER_CLIENT_WRAPPER_H

#include <QObject>
#include <QString>
#include <QThread>
#include <memory>

class WorkerClientWrapper : public QObject
{
    Q_OBJECT

public:
    explicit WorkerClientWrapper(QObject *parent = nullptr);
    ~WorkerClientWrapper();

public slots:
    void connectToServer(const QString& address, int port, const QString& workerType);
    void disconnectFromServer();
    void startWorker();
    void stopWorker();

signals:
    void connected();
    void disconnected();
    void taskReceived(const QString& taskId);
    void taskCompleted(const QString& taskId, const QString& result);
    void errorOccurred(const QString& error);

private:
    class WorkerThread;
    std::unique_ptr<WorkerThread> workerThread;
    bool isConnected;
};

#endif // WORKER_CLIENT_WRAPPER_H

