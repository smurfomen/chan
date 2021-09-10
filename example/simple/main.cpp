#include <QCoreApplication>

#include <QChan>

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
        auto receiver = ch_ping.pipe().first;
        auto sender = ch_pong.pipe().second;
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
