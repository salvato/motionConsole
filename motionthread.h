#pragma once
#include <QObject>
#include <QFileSystemWatcher>
#include <QFileInfoList>
#include <QDir>
#include <QTimer>


QT_FORWARD_DECLARE_CLASS(QFile)


class motionThread : public QObject
{
    Q_OBJECT
public:
    explicit motionThread(QObject *parent = nullptr);
    ~motionThread();

signals:

private slots:
    void onNewFileCreated(QString);
    void onTimeToSend();
    void onTimeToRestartSending();

private:
    bool prepareLogFile();
    void logMessage(QString sMessage);

private:
    QFile* pLogFile;
    QString sLogFileName;
    QString sLogDir;
    QString sMessage;
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
