#include "motionthread.h"
#include <QDateTime>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>


#define SENDING_DELAY   300000
#define RESENDING_DELAY 600000


motionThread::motionThread(QObject *parent)
    : QObject{parent}
    , sEmail(QString("gabriele.salvato.55@gmail.com"))
    , sSubject(QString("Allarme"))
    , sBody(QString("\"Motion Detected\""))
    , sVideoDir(QString("/var/lib/motion"))
//    , sVideoDir(QString("/home/rov"))
    , sFilters(QStringList()
               //<< "*.jpg"  << "*.JPG"
               //<< "*.jpeg" << "*.JPEG"
               << "*.mkv"  << "*.MKV")
    , isTooEarly(false)
{
    // Prepare message logging
    sLogFileName = QString("motionConsoleLog.txt");
    sLogDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if(!sLogDir.endsWith(QString("/"))) sLogDir+= QString("/");
    sLogFileName = sLogDir+sLogFileName;
    prepareLogFile();

    videoDir.setPath(sVideoDir);
    videoDir.setNameFilters(sFilters);
    videoDir.setFilter(QDir::Files);
    videoDir.setSorting(QDir::Time|QDir::Reversed);
    if(!videoDir.exists()) {
        qDebug() << "Error: directory"
                 << sVideoDir
                 << "not present !";
        exit(EXIT_FAILURE);
    }
    oldFilesPresent = videoDir.entryList();

    connect(&watcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(onNewFileCreated(QString)));
    connect(&sendingTimer, SIGNAL(timeout()),
            this, SLOT(onTimeToSend()));
    connect(&restingTimer, SIGNAL(timeout()),
            this, SLOT(onTimeToRestartSending()));

    watcher.addPath(sVideoDir);
    sMessage = QTime::currentTime().toString() + " Program Started...";
    qDebug() << sMessage;
    logMessage(sMessage);
}


motionThread::~motionThread() {
    if(pLogFile) {
        if(pLogFile->isOpen()) {
            pLogFile->flush();
        }
        pLogFile->deleteLater();
        pLogFile = nullptr;
    }
}


bool
motionThread::prepareLogFile() {
    // Rotate 5 previous logs, removing the oldest, to avoid data loss
    QFileInfo checkFile(sLogFileName);
    if(checkFile.exists() && checkFile.isFile()) {
        QDir renamed;
        renamed.remove(sLogFileName+QString("_4.txt"));
        for(int i=4; i>0; i--) {
            renamed.rename(sLogFileName+QString("_%1.txt").arg(i-1),
                           sLogFileName+QString("_%1.txt").arg(i));
        }
        renamed.rename(sLogFileName,
                       sLogFileName+QString("_0.txt"));
    }
    // Open the new log file
    pLogFile = new QFile(sLogFileName);
    if (!pLogFile->open(QIODevice::WriteOnly)) {
        qDebug() <<  QString("Unable to open file %1: %2.")
                     .arg(sLogFileName, pLogFile->errorString());
        delete pLogFile;
        pLogFile = Q_NULLPTR;
    }
    return true;
}


void
motionThread::logMessage(QString sMessage) {
    QDateTime dateTime;
    QString sDebugMessage = dateTime.currentDateTime().toString() +
            QString(" - ") +
            sMessage;
    if(pLogFile) {
        if(pLogFile->isOpen()) {
            pLogFile->write(sDebugMessage.toUtf8().data());
            pLogFile->write("\n");
            pLogFile->flush();
        }
        else
            qDebug() << sDebugMessage;
    }
    else
        qDebug() << sDebugMessage;
}


void
motionThread::onNewFileCreated(QString sPath) {
    videoDir.refresh();
    newFilesPresent = videoDir.entryList();
    for(int i=0; i<newFilesPresent.count(); i++) {
        if(!oldFilesPresent.contains(newFilesPresent.at(i))) {
            sFileToSend = sPath + "/" + newFilesPresent.at(i);
            qDebug() << QTime::currentTime().toString()
                     << "New File Created:"
                     << sFileToSend;
            if(!isTooEarly) {
                isTooEarly = true;
                QString sMessage = QString("Will Send an email in %1 sec")
                                   .arg(SENDING_DELAY/1000);
                logMessage(sMessage);
                sendingTimer.start(SENDING_DELAY);
            }
        }
    }
    oldFilesPresent = newFilesPresent;
}


void
motionThread::onTimeToSend() {
    sendingTimer.stop();
    sCommand = QString("echo \"%1\" | mutt -a \"%2\" -s \"%3\" -- %4")
                   .arg(sBody, sFileToSend, sSubject, sEmail);
    sMessage = QString("Time Elapsed: Sending Motion Image\n") + sCommand;
    logMessage(sMessage);
    int iError = system(sCommand.toLocal8Bit());
    if(iError) {
        sMessage = QString("Error sending email: %1").arg(errno);
    }
    else {
        sMessage = QString("Motion Image Sent");
    }
    logMessage(sMessage);
    sMessage = QString("Avoid Sending emails for %1 sec")
               .arg(RESENDING_DELAY/1000);
    restingTimer.start(RESENDING_DELAY);
}


void
motionThread::onTimeToRestartSending() {
    isTooEarly = false;
}
