#include <QCoreApplication>

#include <chan>

#include <thread>
#include <QDebug>
#include <QThread>
#include <QElapsedTimer>

void non_infinite_wait() {
#ifdef OPTIONAL_INCLUDED
	chan<const char*> chan;

    chan.send("trash message");

    std::thread th([& chan](){
        QElapsedTimer tmr;
        tmr.start();
        qint64 t0 = tmr.elapsed();
        chan.clear();
        auto r =  chan.receive(3000);
        if(r)
        {
			qDebug() << r.value() << ": received after" << tmr.elapsed() - t0 << "milliseconds wait";
        }
        else {
            qDebug() << "none messages was received after deadline." << tmr.elapsed() - t0 << "milliseconds waited";
        }
    });


#define NONE 1
#if NONE
    // none messages was received after deadline. 3000 milliseconds waited
    std::this_thread::sleep_for(std::chrono::milliseconds(4000));
#else
    // my cool message with non infinite waiting from chan : received after 1793 ms wait
    std::this_thread::sleep_for(std::chrono::milliseconds(1793));
    chan.send("my cool message with non infinite waiting from chan");
#endif
    th.join();
#else
	qDebug() << "optional type was not included";
#endif
}


void infinite_wait() {
	chan<const char*> chan;

    chan.send("trash message");

    std::thread th([& chan](){
        QElapsedTimer tmr;
        tmr.start();
        qint64 t0 = tmr.elapsed();
        chan.clear();
        auto r =  chan.receive();
        qDebug() << r << ": received after" << tmr.elapsed() - t0 << "milliseconds wait";
    });

    //my cool message with infinite waiting message from chan : received after 1321 ms wait
    std::this_thread::sleep_for(std::chrono::milliseconds(1321));
    chan.send("my cool message with infinite waiting message from chan");
    th.join();
}


int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    non_infinite_wait();
    infinite_wait();
    return 0;
}
