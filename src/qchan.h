#ifndef QCHAN_H
#define QCHAN_H
#include <mutex>
#include <condition_variable>

#ifdef QOPTION_INCLUDED
#include <QOption>
#endif

#include <limits>
template <typename T>
class QChan
{
public:
    typedef std::queue<T> queue_t;
    class receiver {
        friend class QChan;
    public:
        receiver(queue_t * q, std::mutex * m, std::condition_variable * c)
            : tube(q), mtx(m), cv(c)
        {

        }

        receiver(const receiver & r)
            : tube(r.tube), mtx(r.mtx), cv(r.cv)
        {

        }

        T receive() {
            auto o = receive(std::numeric_limits<quint32>::max());
            if(o)
                return o.unwrap();

            throw;
        }

#ifdef QOPTION_INCLUDED
        QOption<T> receive(qint64 ms) {
            {
                std::unique_lock<std::mutex> lck(*mtx);
                if(tube->size())
                {
                    T tmp = tube->front();
                    tube->pop();
                    return std::move(tmp);
                }
            }

            std::unique_lock<std::mutex> lck(mcv);
            if(cv->wait_for(lck, std::chrono::milliseconds(ms)) == std::cv_status::no_timeout)
            {
                std::unique_lock<std::mutex> lck(*mtx);
                if(tube->size())
                {
                    T tmp = tube->front();
                    tube->pop();
                    return std::move(tmp);
                }
            }

            return None();
        }
#endif // QOPTION_INCLUDED

        receiver & operator>>(T & t)
        {
            t = receive();
            return *this;
        }

    private:
        queue_t * tube;
        std::mutex * mtx;
        std::condition_variable * cv;
        std::mutex mcv;
    };

    class sender {
        friend class QChan;
        sender(queue_t * q, std::mutex * m, std::condition_variable * c)
            : tube(q), mtx(m), cv(c)
        {

        }
    public:
        void send(const T& t)
        {
            std::lock_guard<std::mutex> lck(*mtx);
            tube->push(t);
            cv->notify_all();
        }


        void send(T&&t)
        {
            std::lock_guard<std::mutex> lck(*mtx);
            tube->push(std::forward<T>(t));
            cv->notify_all();
        }


        sender & operator<<(const T & t)
        {
            send(t);
            return *this;
        }

        sender & operator<<(T && t)
        {
            send(std::forward<T>(t));
            return *this;
        }

    private:
        queue_t * tube;
        std::mutex * mtx;
        std::condition_variable * cv;
    };

    receiver rx() {
        return receiver(&tube, &mtx, &cv);
    }

    sender tx() {
        return sender(&tube, &mtx, &cv);
    }

    std::pair<receiver, sender> pipe() {
        return std::make_pair<receiver, sender>(receiver(&tube, &mtx, &cv), sender(&tube, &mtx, &cv));
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    queue_t tube;
};
#endif
