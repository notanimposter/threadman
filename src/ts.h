#ifndef ts_h
#define ts_h

#include <queue>
#include <mutex>
#include <condition_variable>

/**
 * A namespace of explicitly thread-safe stuff
 */
namespace ts {
	/**
	 * Thread-safe printf wrapper
	 */
	void printf (const char* format, ...);
	/**
	 * Thread-safe queue with wait-on-non-empty, suitable for a work queue
	 */
	template <typename T>
	class queue {
		private:
			std::queue<T>* internal;
			std::mutex putLock;
			std::mutex getLock;
			std::mutex waitLock;
			std::condition_variable notEmpty;
		public:
			queue () {
				internal = new std::queue<T>;
			}
			~queue () { delete internal; };
			void put (T thing) {
				putLock.lock ();
				internal->push (thing);
				putLock.unlock ();
				notEmpty.notify_one ();
			}
			T get () {
				getLock.lock ();
				std::unique_lock<std::mutex> lk (waitLock);
				notEmpty.wait (lk, [this]() {
					return !internal->empty ();
				});
				T ret = internal->front ();
				internal->pop ();
				getLock.unlock ();
				return ret;
			}
	};
};
#endif
