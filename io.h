#pragma once

enum seek_flags { SeekSet, SeekCur, SeekEnd };
enum stream_flags {
	StreamRead = 1,
	StreamWrite = 2,
	StreamText = 4,
};
namespace io {
	// Network protocols
	enum protocols { NoProtocol, TCP, UDP };
	// Abstract bi-stream interface
	struct stream {
		stream&				operator<<(const char* t); // Post text string into stream data in correct coding.
		stream&				operator<<(const int n); // Post number as string into stream data in correct coding.
		unsigned			getsize();
		int					readtext(char* result, unsigned size);
		virtual int			read(void* result, int count) = 0;
		virtual void		read(int& value);
		template<class T> void read(T& value);
		virtual int			seek(int count, int rel = SeekCur) { return 0; };
		virtual int			write(const void* result, int count) = 0;
		virtual void		write(const char* value);
		virtual void		write(int value);
		virtual void		write(bool value);
		template<class T> void write(const T& value);
	};
	class file : public stream {
		int					handle;
	public:
		class find {
			class iter {
				find&		parent;
			public:
				constexpr iter(find& parent) : parent(parent) {}
				const char* operator*() const { return parent.name(); }
				bool operator!=(const iter& e) { return parent; }
				void operator++() { parent.next(); }
			};
			char			reserved[512];
			void*			handle;
		public:
			find(const char* url);
			~find();
			operator bool() const { return handle != 0; }
			iter			begin() { return iter(*this); }
			iter			end() { return iter(*this); }
			const char*		name();
			void			next();
		};
		file();
		file(const char* url, unsigned flags = StreamRead);
		~file();
		operator bool() const { return handle != 0; }
		void				close();
		bool				create(const char* url, unsigned flags);
		static bool			exist(const char* url);
		static char*		getdir(char* url, int size);
		static char*		getmodule(char* url, int size);
		static bool			makedir(const char* url);
		int					read(void* result, int count) override;
		static bool			remove(const char* url);
		int					seek(int count, int rel) override;
		static bool			setdir(const char* url);
		int					write(const void* result, int count) override;
	};
	struct memory : public stream {
		memory(void* data, int size);
		int					read(void* result, int count) override;
		int					seek(int count, int rel) override;
		int					write(const void* result, int count) override;
	private:
		unsigned char*		data;
		int					pos;
		int					size;
	};
	struct address {
		unsigned short		family;
		unsigned short		port;
		unsigned			ip;
		char    			reserved[8];
		//
		void				clear();
		bool				parse(const char* url, const char* service_name);
		bool				tostring(char* node, int node_len, char* service, int service_len);
	};
	struct socket : public stream, public address {
		socket();
		~socket();
		//
		void				accept(socket& client);
		bool				bind();
		bool				create(protocols type);
		bool				connect();
		void				listen(int backlog);
		int					read(void* result, int count) override;
		int					write(const void* result, int count) override;
	private:
		int					s;
	};
	bool					read(const char* url, const char* strategy_type, void* param);
	bool					write(const char* url, const char* strategy_type, void* param);
}