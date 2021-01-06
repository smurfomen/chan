#include <QCoreApplication>

#include <qchan.h>

#include <thread>
#include <QDebug>
#include <QThread>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QChan<const char*> ch_ping;
    QChan<const char*> ch_pong;

    std::thread * ping = new std::thread([&](){
       while(true)
       {
           ch_ping.tx().send("ping");
           qDebug()<< ch_pong.rx().receive();
           QThread::msleep(qrand()%1000);
       }
    });


    std::thread * pong = new std::thread([&](){
        auto sender = std::get<1>(ch_pong.rxtx());
        auto receiver = std::get<0>(ch_ping.rxtx());
        while(true)
        {
            qDebug()<<receiver.receive();
            sender << "pong";
            QThread::msleep(qrand()%1000);
        }
    });

    ping->join();
    pong->join();

    return a.exec();
}
