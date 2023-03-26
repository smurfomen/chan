#ifndef CHAN_H
#define CHAN_H
#include <mutex>
#include <condition_variable>

#ifdef OPTIONAL_INCLUDED
#include <optional>
#endif

#include <limits>
#include <queue>
template <typename T>
class chan
{
public:
	typedef std::queue<T> queue_t;
	class receiver {
		friend class chan;
	public:
		receiver(queue_t * q, std::mutex * m, std::condition_variable * c)
			: tube(q), mtx(m), cv(c)
		{

		}

		receiver(const receiver & r)
			: tube(r.tube), mtx(r.mtx), cv(r.cv)
		{

		}

		/*! \brief  Blocking receive message with waiting in infinite time bounds from otherside chan. */
		T receive() {
			std::unique_lock<std::mutex> lck(*mtx);

			if(tube->size())
			{
				T tmp = tube->front();
				tube->pop();
				return std::move(tmp);
			}

			cv->wait(lck);

			if(tube->size())
			{
				T tmp = tube->front();
				tube->pop();
				return std::move(tmp);
			}

			throw std::runtime_error("chan::receiver::receive goes on unreachable code");
		}

#ifdef OPTIONAL_INCLUDED
		/*! \brief  Blocking receive message with waiting a message in bounds ms millisecconds time range from otherside chan. */
		optional<T> receive(int64_t ms) {
			std::unique_lock<std::mutex> lck(*mtx);
			if(tube->size())
			{
				T tmp = tube->front();
				tube->pop();
				return tmp;
			}

			if(cv->wait_for(lck, std::chrono::milliseconds(ms)) == std::cv_status::no_timeout)
			{
				if(tube->size())
				{
					T tmp = tube->front();
					tube->pop();
					return tmp;
				}
			}

			return nullopt;
		}
#endif // OPTIONAL_INCLUDED

		/*!
		 *  \brief  Blocking receive message with waiting in infinite time bounds from otherside chan.
		 *  \return Returns reference to self for composition.
		 */
		receiver & operator>>(T & t)
		{
			t = receive();
			return *this;
		}

	private:
		/* Queue messages. Shared between many receivers and transmitters. Owned by chan parent object. */
		queue_t * tube;

		/* Mutex for lock access to queue messages. Shared between many receivers and transmitters. Owned by chan parent object. */
		std::mutex * mtx;

		/* Condition variable for notify when new message is puted to queue messages. Shared between many receivers and transmitters. Owned by chan parent object. */
		std::condition_variable * cv;
	};

	class sender {
		friend class chan;
		sender(queue_t * q, std::mutex * m, std::condition_variable * c)
			: tube(q), mtx(m), cv(c)
		{

		}

	public:
		/*! \brief  Send message to otherside chan. */
		void send(const T& t)
		{
			{
				std::lock_guard<std::mutex> lck(*mtx);
				tube->push(t);
			}
			cv->notify_all();
		}

		/*! \brief  Send message to otherside chan. */
		void send(T&&t)
		{
			{
				std::lock_guard<std::mutex> lck(*mtx);
				tube->push(std::forward<T>(t));
			}
			cv->notify_all();
		}

		/*!
		 *  \brief  Send message to otherside chan.
		 *  \return Returns reference to self for composition.
		 */
		sender & operator<<(const T & t)
		{
			send(t);
			return *this;
		}

		/*!
		 *  \brief  Send message to otherside chan.
		 *  \return Returns reference to self for composition.
		 */
		sender & operator<<(T && t)
		{
			send(std::forward<T>(t));
			return *this;
		}

	private:
		/* Queue messages. Shared between many receivers and transmitters. Owned by chan parent object. */
		queue_t * tube;

		/* Mutex for lock access to queue messages. Shared between many receivers and transmitters. Owned by chan parent object. */
		std::mutex * mtx;

		/* Condition variable for notify when new message is puted to queue messages. Shared between many receivers and transmitters. Owned by chan parent object. */
		std::condition_variable * cv;
	};

	/*! \brief  Makes and returns new receiver object. */
	receiver rx()
	{ return receiver(&tube, &mtx, &cv); }

	/*! \brief  Makes and returns new transmitter object. */
	sender tx()
	{ return sender(&tube, &mtx, &cv); }

	/*! \brief  Makes and returns new pipe from receiver and sender objects. */
	std::pair<receiver, sender> pipe()
	{ return std::make_pair<receiver, sender>(receiver(&tube, &mtx, &cv), sender(&tube, &mtx, &cv)); }

	/*! \brief  Clear chan queue messages. */
	void clear() {
		std::unique_lock<std::mutex> lck(mtx);
		while(tube.size()) {
			tube.pop();
		}
	}

	/*! \brief  Send message to otherside chan. */
	void send(const T& t)
	{ tx() << t; }

	/*! \brief  Send message to otherside chan. */
	void send(T&&t)
	{ tx() << std::forward<T>(t); }

	/*! \brief  Blocking receive message with waiting in infinite time bounds from otherside chan. */
	T receive()
	{ return rx().receive(); }

#ifdef OPTIONAL_INCLUDED
	/*! \brief  Blocking receive message with waiting a message in bounds ms millisecconds time range from otherside chan. */
	optional<T> receive(int64_t ms)
	{ return rx().receive(ms); }
#endif


private:
	/* Mutex for waiting with confition variable. Owned by this receiver object. */
	std::mutex mtx;

	/* Condition variable for notify when new message is puted to queue messages. Shared between many receivers and transmitters. Owned by chan parent object. */
	std::condition_variable cv;

	/* Queue messages. Shared between many receivers and transmitters. Owned by chan parent object. */
	queue_t tube;
};
#endif
