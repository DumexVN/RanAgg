#include <iostream>

#include <QCoreApplication>
#include <QtGlobal>
#include <QtDebug>
#include <QFile>
#include <QTextStream>

#include <stdio.h>
#include <stdlib.h>
#include "graph.h"

/** Override Debug Message Handler
 * @brief myMessageOutput
 * @param type
 * @param context
 * @param msg
 */
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt =  QString("%1 ").arg(msg).append('\n');
        break;
    case QtFatalMsg:
        txt =  QString("Fatal: %1 ").arg(msg).append('\n');
        abort();
    }
    QFile outFile("C:/Users/Dumex/Desktop/SocialNetworksCollection/SNAP_DumexTemplate/log.txt");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
    outFile.close();
}


int main(int argc, char *argv[])
{
    Graph G;
    QString dirPath = "C:/Users/Dumex/Desktop/SocialNetworksCollection/SNAP_DumexTemplate/DBLP";
    QDir dir(dirPath);
    if (!dir.exists())
    {
        qDebug() << "Dir Not Found!";
        return 0;
    }
    QFile file("C:/Users/Dumex/Desktop/SocialNetworksCollection/SNAP_DumexTemplate/log.txt");
    if (file.exists())
    {
        if(file.remove())
            qDebug() << "Duplicate Log File Succesfully Removed";
        else
            qDebug() << "Problem Removing Exsisting Log File";
    }
    G.read_DUMEX_input(dirPath);
    qInstallMessageHandler(myMessageOutput);
    qDebug() << "Finished Loading! Algorithm Starting ...";
    G.LARGE_rerun();
    qDebug() << "Finished! Check Log File";
    return 0;
}
