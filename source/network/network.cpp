#include <flame/network/network.h>

#include <winsock2.h>
#include <assert.h>

namespace flame
{
	bool websocket_shakehank(int fd_c)
	{
		int res;
		fd_set rfds;
		timeval timeout = { 1, 0 };

		FD_ZERO(&rfds);
		FD_SET(fd_c, &rfds);
		res = select(-1, &rfds, nullptr, nullptr, &timeout);

		if (res <= 0)
		{
			closesocket(fd_c);
			return false;
		}

		uchar buf[1024 * 16];
		auto ret = recv(fd_c, (char*)buf, array_size(buf), 0);

		auto p = buf;

		if (ret <= 0 || !(ret > 3 && p[0] == 'G' && p[1] == 'E' && p[2] == 'T'))
		{
			closesocket(fd_c);
			return false;
		}

		p[ret] = 0;

		std::regex reg_key(R"(Sec-WebSocket-Key: (.*))");

		std::string req((char*)p);
		auto lines = SUS::split(req, '\n');
		for (auto& l : lines)
		{
			std::smatch res;
			if (std::regex_search(l, res, reg_key))
			{
				std::string key = res[1];
				SHA1 sha1;
				sha1.update(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
				key = base64_encode(sha1.final_bin());

				char reply[1024 * 16], time_str[128];
				auto time = std::time(nullptr);
				std::strftime(time_str, array_size(time_str), "%a, %d %b %Y %FLAME_HASH:%M:%S GMT", std::localtime(&time));
				sprintf(reply, "HTTP/1.1 101 Switching Protocols\r\n"
					"Content-Length: 0\r\n"
					"Upgrade: websocket\r\n"
					"Sec-Websocket-Accept: %s\r\n"
					"Server: flame\r\n"
					"Connection: Upgrade\r\n"
					"Data: %s\r\n"
					"\r\n", key.c_str(), time_str);
				auto res = ::send(fd_c, reply, strlen(reply), 0);
				assert(res > 0);
			}
		}

		return true;
	}

	bool websocket_send(int fd, uint size, void* data)
	{
		uchar buf[1024 * 64];
		auto p = buf;

		*p++ = 129;
		if (size <= 125)
			*p++ = size;
		else if (size <= 65535)
		{
			*p++ = 126;

			*p++ = size >> 8;
			*p++ = size & 0xff;
		}
		else
		{
			*p++ = 127;

			*p++ = size >> 56;
			*p++ = (size >> 48) & 0xff;
			*p++ = (size >> 40) & 0xff;
			*p++ = (size >> 32) & 0xff;

			*p++ = (size >> 24) >> 56;
			*p++ = (size >> 16) & 0xff;
			*p++ = (size >> 8) & 0xff;
			*p++ = size & 0xff;
		}

		memcpy(p, data, size);

		return ::send(fd, (char*)buf, int(p - buf) + size, 0) > 0;
	}

	std::vector<std::string> websocket_recv(int fd, bool& closed)
	{
		uchar buf[1024 * 64];

		auto bytes = recv(fd, (char*)buf, array_size(buf), 0);
		if (bytes <= 0)
		{
			closed = true;
			return std::vector<std::string>();
		}

		std::vector<std::string> ret;

		auto p = buf;
		while (bytes > 0)
		{
			if (bytes < 2)
			{
				auto new_bytes = recv(fd, (char*)p + bytes, 2 - bytes, 0);
				if (new_bytes <= 0)
					break;
				bytes += new_bytes;
			}

			uchar b1, b2;
			b1 = *p++;
			b2 = *p++;
			bytes -= 2;

			auto op = b1 & 0xf;
			auto mask = (b2 & 128) != 0;
			auto payload_length = b2 & 127;

			auto need_bytes = 0;
			if (payload_length == 126)
				need_bytes = 2;
			else if (payload_length == 127)
				need_bytes = 8;
			if (mask)
				need_bytes += 4;

			if (bytes < need_bytes)
			{
				auto new_bytes = recv(fd, (char*)p + bytes, need_bytes - bytes, 0);
				if (new_bytes <= 0)
					break;
				bytes += new_bytes;
			}

			ulonglong length;

			if (payload_length <= 125)
				length = payload_length;
			else if (payload_length == 126)
			{
				length = (*p++) << 8;
				length += *p++;
				
				bytes -= 2;
			}
			else if (payload_length == 127)
			{
				length = (*p++) << 56;
				length += (*p++) << 48;
				length += (*p++) << 40;
				length += (*p++) << 32;

				length += (*p++) << 24;
				length += (*p++) << 16;
				length += (*p++) << 8;
				length += *p++;

				bytes -= 8;
			}

			uint mask_key;
			if (mask)
			{
				mask_key = *(uint*)p;

				p += 4;
				bytes -= 4;
			}

			if (bytes < length)
			{
				auto new_bytes = recv(fd, (char*)p + bytes, length - bytes, 0);
				if (new_bytes <= 0)
					break;
				bytes += new_bytes;
			}

			for (auto i = 0; i < length; i++)
				p[i] ^= ((char*)&mask_key)[i % 4];

			if (op == 1)
				ret.push_back(std::string(p, p + length));

			p += length;
			bytes -= length;
		}

		return ret;
	}

	void init()
	{
		static auto inited = false;
		if (!inited)
		{
			WSADATA wsa_d = {};
			WSAStartup(MAKEWORD(1, 1), &wsa_d);
			inited = true;
		}
	}

	struct ClientPrivate : Client
	{
		SocketType type;

		int fd;

		std::unique_ptr<Closure<void(void* c, const char* msg)>> on_message;
		std::unique_ptr<Closure<void(void* c)>> on_close;

		~ClientPrivate()
		{
			closesocket(fd);
		}

		void close()
		{
			closesocket(fd);
			fd = 0;
			on_close->call();
		}
	};

	void Client::send(uint size, void* data)
	{
		::send(((ClientPrivate*)this)->fd, (char*)data, size, 0);
	}

	Client* Client::create(SocketType type, const char* ip, uint port, void on_message(void* c, const char* msg), void on_close(void* c), const Mail<>& capture)
	{
		init();

		int res;

		auto fd_c = socket(AF_INET, SOCK_STREAM, 0);
		assert(fd_c != INVALID_SOCKET);

		sockaddr_in address = {};
		address.sin_family = AF_INET;
		address.sin_addr.S_un.S_addr = inet_addr(ip);
		address.sin_port = htons(port);

		res = connect(fd_c, (sockaddr*)&address, sizeof(address));
		if (res == SOCKET_ERROR)
		{
			closesocket(fd_c);
			return nullptr;
		}

		auto c = new ClientPrivate;
		c->type = type;
		c->fd = fd_c;
		{
			auto _c = new Closure<void(void* c, const char* msg)>;
			_c->function = on_message;
			_c->capture = capture;
			c->on_message.reset(_c);
		}
		{
			auto _c = new Closure<void(void* c)>;
			_c->function = on_close;
			_c->capture = capture;
			c->on_close.reset(_c);
		}

		std::thread([=]() {
			while (true)
			{
				int size;
				auto ret = recv(fd_c, (char*)&size, sizeof(int), 0);
				if (ret < sizeof(int))
				{
					c->close();
					return;
				}
				char buf[1024 * 64];
				auto p = buf;
				while (size > 0)
				{
					auto ret = recv(fd_c, p, size, 0);
					if (ret <= 0)
					{
						c->close();
						return;
					}
					p += ret;
					size -= ret;
				}
				c->on_message->call(buf);
			}
		}).detach();

		return c;
	}

	void Client::destroy(Client* c)
	{

	}

	struct ServerPrivate : Server
	{
		struct Client
		{
			int fd;
			std::unique_ptr<Closure<void(void* c, const char* msg)>> on_message;
			std::unique_ptr<Closure<void(void* c)>> on_close;

			~Client()
			{
				closesocket(fd);
				fd = 0;
				on_close->call();
			}
		};

		SocketType type;

		int fd;

		std::vector<std::unique_ptr<Client>> cs;

		std::unique_ptr<Closure<void(void* c, void* id)>> on_connect;

		~ServerPrivate()
		{
			closesocket(fd);
		}

		void remove_client(Client* c)
		{
			for (auto it = cs.begin(); it != cs.end(); it++)
			{
				if (it->get() == c)
				{
					cs.erase(it);
					break;
				}
			}
		}
	};

	void Server::set_client(void* id, void on_message(void* c, const char* msg), void on_close(void* c), const Mail<>& capture)
	{
		{
			auto c = new Closure<void(void* c, const char* msg)>;
			c->function = on_message;
			c->capture = capture;
			((ServerPrivate::Client*)id)->on_message.reset(c);
		}
		{
			auto c = new Closure<void(void* c)>;
			c->function = on_close;
			c->capture = capture;
			((ServerPrivate::Client*)id)->on_close.reset(c);
		}
	}

	void Server::send(void* id, uint size, void* data)
	{
		::send(((ServerPrivate::Client*)id)->fd, (char*)data, size, 0) > 0;
	}

	Server* Server::create(SocketType type, uint port, void on_connect(void* c, void* id), const Mail<>& capture)
	{
		init();

		int res;

		auto fd_s = socket(AF_INET, SOCK_STREAM, 0);
		assert(fd_s != INVALID_SOCKET);

		sockaddr_in address = {};
		address.sin_family = AF_INET;
		address.sin_addr.S_un.S_addr = INADDR_ANY;
		address.sin_port = htons(port);
		res = bind(fd_s, (sockaddr*)&address, sizeof(address));
		assert(res == 0);
		res = listen(fd_s, 1);
		assert(res == 0);

		auto s = new ServerPrivate;
		s->type = type;
		s->fd = fd_s;
		{
			auto c = new Closure<void(void* c, void* id)>;
			c->function = on_connect;
			c->capture = capture;
			s->on_connect.reset(c);
		}

		std::thread([=]() {
			while (true)
			{
				auto fd = accept(fd_s, nullptr, nullptr);
				if (fd == INVALID_SOCKET)
					return;
				auto c = new ServerPrivate::Client;
				c->fd = fd;
				s->on_connect->call(c);
				if (c->on_message->function || c->on_close->function)
					s->cs.emplace_back(c);
				else
					delete c;

				std::thread([=]() {
					while (true)
					{
						int size;
						auto ret = recv(fd, (char*)&size, sizeof(int), 0);
						if (ret < sizeof(int))
						{
							s->remove_client(c);
							return;
						}
						char buf[1024 * 64];
						auto p = buf;
						while (size > 0)
						{
							auto ret = recv(fd, p, size, 0);
							if (ret <= 0)
							{
								s->remove_client(c);
								return;
							}
							p += ret;
							size -= ret;
						}
						c->on_message->call(buf);
					}
				}).detach();
			}
		}).detach();

		return s;
	}

	void Server::destroy(Server* s)
	{
		delete (ServerPrivate*)s;
	}

	struct OneClientServerPrivate : OneClientServer
	{
		SocketType type;

		int fd_c;

		bool send(uint size, void* data)
		{
			if (type == SocketWeb)
				return websocket_send(fd_c, size, data);
			else
				return ::send(fd_c, (char*)data, size, 0) > 0;
		}
	};

	bool OneClientServer::send(uint size, void* data)
	{
		return ((OneClientServerPrivate*)this)->send(size, data);
	}

	OneClientServer* OneClientServer::create(SocketType type, uint port, uint _timeout, void on_message(void* c, const char* msg), const Mail<>& capture)
	{
		init();

		int res;

		auto fd_s = socket(AF_INET, SOCK_STREAM, 0);
		assert(fd_s != INVALID_SOCKET);

		sockaddr_in address = {};
		address.sin_family = AF_INET;
		address.sin_addr.S_un.S_addr = INADDR_ANY;
		address.sin_port = htons(port);
		res = bind(fd_s, (sockaddr*)& address, sizeof(address));
		assert(res == 0);
		res = listen(fd_s, 1);
		assert(res == 0);

		timeval timeout = { _timeout, 0 };
		fd_set rfds;

		FD_ZERO(&rfds);
		FD_SET(fd_s, &rfds);
		res = select(-1, &rfds, nullptr, nullptr, &timeout);
		if (res <= 0)
		{
			closesocket(fd_s);
			return nullptr;
		}

		auto fd_c = accept(fd_s, nullptr, nullptr);
		closesocket(fd_s);

		if (type == SocketWeb)
		{
			if (!websocket_shakehank(fd_c))
				return nullptr;
		}

		auto s = new OneClientServerPrivate;
		s->type = type;
		s->fd_c = fd_c;
		s->ev_closed = CreateEvent(nullptr, true, false, nullptr);

		std::thread([=]() {
			while (true)
			{
				bool closed = false;
				auto reqs = websocket_recv(fd_c, closed);
				for (auto& r : reqs)
					on_message(capture.p, r.c_str());
				if (closed)
				{
					SetEvent(s->ev_closed);
					delete_mail(capture);
					return;
				}
			}
		}).detach();

		return s;
	}

	void OneClientServer::destroy(OneClientServer* s)
	{

	}

	struct FrameSyncServerPrivate : FrameSyncServer
	{
		SocketType type;

		int frame;
		int semaphore;

		std::vector<int> fd_cs;

		nlohmann::json frame_data;

		bool send(uint client_idx, int size, void* data)
		{
			if (type == SocketWeb)
				return websocket_send(fd_cs[client_idx], size, data);
			else
				return ::send(fd_cs[client_idx], (char*)data, size, 0) > 0;
		}
	};

	FrameSyncServer* FrameSyncServer::create(SocketType type, uint port, uint client_count)
	{
		int res;

		auto fd_s = socket(AF_INET, SOCK_STREAM, 0);
		assert(fd_s != INVALID_SOCKET);

		sockaddr_in address = {};
		address.sin_family = AF_INET;
		address.sin_addr.S_un.S_addr = INADDR_ANY;
		address.sin_port = htons(port);
		res = bind(fd_s, (sockaddr*)& address, sizeof(address));
		assert(res == 0);
		res = listen(fd_s, client_count);
		assert(res == 0);

		std::vector<int> fd_cs;
		while (fd_cs.size() < client_count)
		{
			auto fd_c = accept(fd_s, nullptr, nullptr);
			if (fd_c == INVALID_SOCKET)
				return nullptr;

			if (type == SocketWeb)
			{
				if (!websocket_shakehank(fd_c))
					continue;
			}

			fd_cs.push_back(fd_c);
		}

		closesocket(fd_s);

		auto s = new FrameSyncServerPrivate;
		s->type = type;
		s->frame = 0;
		s->semaphore = 0;
		s->fd_cs = fd_cs;
		s->ev_closed = CreateEvent(nullptr, true, false, nullptr);

		{
			srand(time(0));
			nlohmann::json json = {
				{"action", "start"},
				{"seed", ::rand()}
			};
			auto str = json.dump();
			for (auto i = 0; i < client_count; i++)
				s->send(i, str.size(), str.data());
		}

		std::thread([s]() {
			while (true)
			{
				fd_set rfds;
				FD_ZERO(&rfds);
				for (auto fd : s->fd_cs)
					FD_SET(fd, &rfds);
				if (select(-1, &rfds, nullptr, nullptr, nullptr) > 0)
				{
					auto client_idx = 0;
					for (auto fd : s->fd_cs)
					{
						if (FD_ISSET(fd, &rfds))
						{
							bool closed = false;
							auto reqs = websocket_recv(fd, closed);
							if (reqs.size() > 0)
							{
								auto req = nlohmann::json::parse(reqs[0]);
								auto n_frame = req.find("frame");
								if (n_frame != req.end())
								{
									auto frame = n_frame->get<int>();
									if (frame == s->frame)
									{
										s->semaphore++;
										auto dst = s->frame_data[std::to_string(client_idx + 1)];
										for (auto& i : req["data"].items())
											dst[i.key()] = i.value();

										if (s->semaphore >= s->fd_cs.size())
										{
											s->frame++;
											s->semaphore = 0;

											s->frame_data["action"] = "frame";
											auto str = s->frame_data.dump();
											for (auto i = 0; i < 2; i++)
												s->send(i, str.size(), str.data());
											s->frame_data.clear();
										}
									}
								}
							}
							if (closed)
							{
								// TODO
							}
						}
						client_idx++;
					}
				}
			}
		}).detach();

		return s;
	}

	bool FrameSyncServer::send(uint client_idx, uint size, void* data)
	{
		return ((FrameSyncServerPrivate*)this)->send(client_idx, size, data);
	}

	void FrameSyncServer::destroy(FrameSyncServer* s)
	{

	}
}
