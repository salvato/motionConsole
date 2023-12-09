#pragma once
#include <QObject>
#include <QFileSystemWatcher>
#include <QFileInfoList>
#include <QDir>
#include <QTimer>


class motionThread : public QObject
{
    Q_OBJECT
public:
    explicit motionThread(QObject *parent = nullptr);

signals:

private slots:
    void onNewFileCreated(QString);
    void onTimeToSend();
    void onTimeToRestartSending();

private:
    QString sEmail, sSubject, sBody;
    QString sVideoDir;
    QStringList sFilters;
    QDir videoDir;
    QFileSystemWatcher watcher;
    QStringList oldFilesPresent;
    QStringList newFilesPresent;
    volatile bool isTooEarly;
    QString sCommand;
    QString sFileToSend;
    QTimer sendingTimer;
    QTimer restingTimer;
};
