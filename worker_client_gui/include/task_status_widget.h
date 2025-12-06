#ifndef TASK_STATUS_WIDGET_H
#define TASK_STATUS_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QGroupBox>

class TaskStatusWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskStatusWidget(QWidget *parent = nullptr);

public slots:
    void onTaskQueued(const QString& taskId);
    void onTaskCompleted(const QString& taskId, const QString& result);
    void onTaskFailed(const QString& taskId, const QString& error);
    void updateStatistics(int queued, int completed, int failed);

private:
    void setupUI();
    
    QLabel *queuedLabel;
    QLabel *completedLabel;
    QLabel *failedLabel;
    QTableWidget *taskTable;
    QPushButton *clearButton;
    
    int queuedCount;
    int completedCount;
    int failedCount;
};

#endif // TASK_STATUS_WIDGET_H

