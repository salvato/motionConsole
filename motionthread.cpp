#include "motionthread.h"

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
               << "*.jpg"  << "*.JPG"
               //<< "*.mkv"  << "*.MKV"
               << "*.jpeg" << "*.JPEG")
    , isTooEarly(false)
{
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
    qDebug() << QTime::currentTime().toString()
             << "Program Started:";
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
                qDebug() << "Will Send an email in "
                         << SENDING_DELAY/1000
                         << "sec";
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
    qDebug() << "Time Elapsed: Sending Motion Image";
    qDebug() << sCommand;
    int iError = system(sCommand.toLocal8Bit());
    if(iError) {
        qDebug() << "Error sending email:" << errno;
    }
    else {
        qDebug() << "Motion Image Sent";
    }
    qDebug() << "Avoid Sending emails for "
             << RESENDING_DELAY/1000
             << "sec";
    restingTimer.start(RESENDING_DELAY);
}


void
motionThread::onTimeToRestartSending() {
    isTooEarly = false;
}
