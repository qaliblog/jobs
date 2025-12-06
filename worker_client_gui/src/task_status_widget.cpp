#include "task_status_widget.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QDateTime>
#include <algorithm>

TaskStatusWidget::TaskStatusWidget(QWidget *parent)
    : QWidget(parent)
    , queuedCount(0)
    , completedCount(0)
    , failedCount(0)
{
    setupUI();
}

void TaskStatusWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Statistics
    QGroupBox *statsGroup = new QGroupBox("Statistics", this);
    QHBoxLayout *statsLayout = new QHBoxLayout(statsGroup);
    
    queuedLabel = new QLabel("Queued: 0", this);
    completedLabel = new QLabel("Completed: 0", this);
    failedLabel = new QLabel("Failed: 0", this);
    
    statsLayout->addWidget(queuedLabel);
    statsLayout->addWidget(completedLabel);
    statsLayout->addWidget(failedLabel);
    statsLayout->addStretch();
    
    // Task table
    taskTable = new QTableWidget(this);
    taskTable->setColumnCount(4);
    taskTable->setHorizontalHeaderLabels(QStringList() << "Task ID" << "Status" << "Result" << "Time");
    taskTable->horizontalHeader()->setStretchLastSection(true);
    taskTable->setAlternatingRowColors(true);
    taskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    // Clear button
    clearButton = new QPushButton("Clear Completed", this);
    
    mainLayout->addWidget(statsGroup);
    mainLayout->addWidget(taskTable);
    mainLayout->addWidget(clearButton);
    
    connect(clearButton, &QPushButton::clicked, [this]() {
        for (int i = taskTable->rowCount() - 1; i >= 0; --i) {
            QTableWidgetItem *statusItem = taskTable->item(i, 1);
            if (statusItem && (statusItem->text() == "Completed" || statusItem->text() == "Failed")) {
                taskTable->removeRow(i);
            }
        }
    });
}

void TaskStatusWidget::onTaskQueued(const QString& taskId)
{
    queuedCount++;
    updateStatistics(queuedCount, completedCount, failedCount);
    
    int row = taskTable->rowCount();
    taskTable->insertRow(row);
    taskTable->setItem(row, 0, new QTableWidgetItem(taskId));
    taskTable->setItem(row, 1, new QTableWidgetItem("Queued"));
    taskTable->setItem(row, 2, new QTableWidgetItem("-"));
    taskTable->setItem(row, 3, new QTableWidgetItem(QDateTime::currentDateTime().toString()));
}

void TaskStatusWidget::onTaskCompleted(const QString& taskId, const QString& result)
{
    queuedCount = std::max(0, queuedCount - 1);
    completedCount++;
    updateStatistics(queuedCount, completedCount, failedCount);
    
    // Find and update task
    for (int i = 0; i < taskTable->rowCount(); ++i) {
        QTableWidgetItem *idItem = taskTable->item(i, 0);
        if (idItem && idItem->text() == taskId) {
            taskTable->setItem(i, 1, new QTableWidgetItem("Completed"));
            taskTable->setItem(i, 2, new QTableWidgetItem(result));
            break;
        }
    }
}

void TaskStatusWidget::onTaskFailed(const QString& taskId, const QString& error)
{
    queuedCount = std::max(0, queuedCount - 1);
    failedCount++;
    updateStatistics(queuedCount, completedCount, failedCount);
    
    // Find and update task
    for (int i = 0; i < taskTable->rowCount(); ++i) {
        QTableWidgetItem *idItem = taskTable->item(i, 0);
        if (idItem && idItem->text() == taskId) {
            taskTable->setItem(i, 1, new QTableWidgetItem("Failed"));
            taskTable->setItem(i, 2, new QTableWidgetItem(error));
            break;
        }
    }
}

void TaskStatusWidget::updateStatistics(int queued, int completed, int failed)
{
    queuedLabel->setText(QString("Queued: %1").arg(queued));
    completedLabel->setText(QString("Completed: %1").arg(completed));
    failedLabel->setText(QString("Failed: %1").arg(failed));
}

