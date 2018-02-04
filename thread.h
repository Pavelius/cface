#pragma once

struct thread {
	struct mutex {
		mutex();
		~mutex();
		void				lock();
		bool				try_lock();
		void				unlock();
	private:
		char				data[40];
	};
	struct sync {
		sync(mutex& e);
		~sync();
	private:
		mutex&				data;
	};
	typedef unsigned(*proc)(void* param);
	//
	int						id;
	//
	thread(proc p, void* param = 0);
	void					join();
	bool					joinable() const;
	static void				run(proc p, void* param = 0);
private:
	void*					handle;
};