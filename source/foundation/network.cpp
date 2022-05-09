#include "../base64.h"
#include "network_private.h"
#include "system.h"

#include <winsock2.h>

#ifdef USE_SHA1
#include <sha1.hpp>
#endif

namespace flame
{
	namespace network
	{
		static bool websocket_shakehand(SocketType type, int fd)
		{
			if (type == SocketWeb)
			{
#ifdef USE_SHA1
				int res;

				fd_set rfds;
				timeval timeout = { 1, 0 };
				FD_ZERO(&rfds);
				FD_SET(fd, &rfds);
				res = select(-1, &rfds, nullptr, nullptr, &timeout);

				if (res <= 0)
				{
					closesocket(fd);
					return false;
				}

				uchar buf[1024 * 16];
				auto ret = recv(fd, (char*)buf, _countof(buf), 0);

				auto p = buf;

				if (ret <= 0 || !(ret > 3 && p[0] == 'G' && p[1] == 'E' && p[2] == 'T'))
				{
					closesocket(fd);
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
						auto str1 = sha1.final();
						auto str2 = std::string();
						str2.resize(sizeof(uint) * (str1.size() / 8));
						{
							auto dst = str2.data();
							for (auto i = 0; i < str2.size(); i++)
							{
								uint v;
								std::stringstream ss;
								ss << std::hex << str2.substr(i * 8, 8);
								ss >> v;
								for (auto j = 0; j < 4; j++)
									dst[j] = ((char*)(&v))[4 - j - 1];
								dst += 4;
							}
						}
						key = base64::encode(str2.data(), str2.size());

						char reply[1024 * 16], time_str[128];
						auto time = std::time(nullptr);
						std::strftime(time_str, _countof(time_str), "%a, %d %b %Y %H:%M:%S GMT", std::localtime(&time));
						sprintf(reply, "HTTP/1.1 101 Switching Protocols\r\n"
							"Content-Length: 0\r\n"
							"Upgrade: websocket\r\n"
							"Sec-Websocket-Accept: %s\r\n"
							"Server: flame\r\n"
							"Connection: Upgrade\r\n"
							"Data: %s\r\n"
							"\r\n", key.c_str(), time_str);
						auto res = send(fd, reply, strlen(reply), 0);
						assert(res > 0);
					}
				}

				return true;
#else
				return false;
#endif
			}
			else
				return true;
		}

		bool native_send(SocketType type, int fd, std::string_view msg)
		{
			auto size = (int)msg.size();
			char buf[1024 * 64];

			if (type == SocketWeb)
			{
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

				memcpy(p, msg.data(), size);

				return send(fd, (char*)buf, int(p - buf) + size, 0) > 0;
			}
			else
			{
				memcpy(buf, &size, sizeof(uint));
				memcpy(buf + sizeof(uint), msg.data(), size);
				return send(fd, buf, sizeof(uint) + size, 0) > 0;
			}
		}

		bool native_recv(SocketType type, int fd, std::vector<std::string>& res)
		{
			uchar buf[1024 * 64];

			if (type == SocketWeb)
			{
				auto n = recv(fd, (char*)buf, _countof(buf), 0);
				if (n <= 0)
					return false;

				auto p = buf;
				while (n > 0)
				{
					if (n < 2)
					{
						auto ret = recv(fd, (char*)p + n, 2 - n, 0);
						if (ret <= 0)
							return false;
						n += ret;
					}

					uchar b1, b2;
					b1 = *p++;
					b2 = *p++;
					n -= 2;

					auto op = b1 & 0xf;
					auto mask = (b2 & 128) != 0;
					auto len = b2 & 127;

					{
						auto b = 0;
						if (len == 126)
							b = 2;
						else if (len == 127)
							b = 8;
						if (mask)
							b += 4;
						if (n < b)
						{
							auto ret = recv(fd, (char*)p + n, b - n, 0);
							if (ret <= 0)
								return false;
							n += ret;
						}
					}

					uint64 length = 0;

					if (len <= 125)
						length += len;
					else if (len == 126)
					{
						length += (*p++) << 8;
						length += *p++;

						n -= 2;
					}
					else if (len == 127)
					{
						length += (*p++) << 56;
						length += (*p++) << 48;
						length += (*p++) << 40;
						length += (*p++) << 32;

						length += (*p++) << 24;
						length += (*p++) << 16;
						length += (*p++) << 8;
						length += *p++;

						n -= 8;
					}

					uint mask_key;
					if (mask)
					{
						mask_key = *(uint*)p;

						p += 4;
						n -= 4;
					}

					if (n < length)
					{
						auto ret = recv(fd, (char*)p + n, length - n, 0);
						if (ret <= 0)
							return false;
						n += ret;
					}

					for (auto i = 0; i < length; i++)
						p[i] ^= ((char*)&mask_key)[i % 4];

					if (op == 1)
						res.push_back(std::string(p, p + length));

					p += length;
					n -= length;
				}

				return true;
			}
			else
			{
				auto n = recv(fd, (char*)buf, _countof(buf), 0);
				if (n <= 0)
					return false;
				auto p = buf;
				while (n > 0)
				{
					if (n < sizeof(uint))
					{
						auto ret = recv(fd, (char*)p + n, sizeof(uint) - n, 0);
						if (ret <= 0)
							return false;
						n += ret;
					}

					auto length = *(uint*)p;
					p += 4;
					n -= 4;

					if (n < length)
					{
						auto ret = recv(fd, (char*)p + n, length - n, 0);
						if (ret <= 0)
							return false;
						n += ret;
					}

					res.push_back(std::string(p, p + length));

					p += length;
					n -= length;
				}
			}
		}

		static bool initialized = false;
		void initialize()
		{
			if (initialized)
				return;
			WSADATA wsad = {};
			WSAStartup(MAKEWORD(1, 1), &wsad);
			initialized = true;
		}

		ClientPrivate::~ClientPrivate()
		{
			stop(false);
			wait_native_event(ev_ended, -1);
			destroy_native_event(ev_ended);
		}

		void ClientPrivate::send(std::string_view msg)
		{
			native_send(type, fd, msg);
		}

		void ClientPrivate::stop(bool passive)
		{
			std::lock_guard lock(mtx);
			if (fd)
			{
				closesocket(fd);
				fd = 0;
				if (passive)
					on_close();
			}
		}

		struct ClientCreate : Client::Create
		{
			ClientPtr operator()(SocketType type, const char* ip, uint port, const std::function<void(std::string_view msg)>& on_message, const std::function<void()>& on_close) override
			{
				initialize();

				int res;

				auto fd = socket(AF_INET, SOCK_STREAM, 0);
				assert(fd != INVALID_SOCKET);
				sockaddr_in address = {};
				address.sin_family = AF_INET;
				address.sin_addr.S_un.S_addr = inet_addr(ip);
				address.sin_port = htons(port);
				res = connect(fd, (sockaddr*)&address, sizeof(address));
				if (res == SOCKET_ERROR)
				{
					closesocket(fd);
					return nullptr;
				}

				auto c = new ClientPrivate;
				c->type = type;
				c->fd = fd;
				c->ev_ended = create_native_event(false, true);
				c->on_message = on_message;
				c->on_close = on_close;

				std::thread([=]() {
					while (true)
					{
						std::vector<std::string> res;
						if (!native_recv(type, c->fd, res))
						{
							c->stop(true);
							set_native_event(c->ev_ended);
							return;
						}
						std::lock_guard lock(c->mtx);
						for (auto& r : res)
							c->on_message(r);
					}
					}).detach();

					return c;
			}
		}Client_create;
		Client::Create& Client::create = Client_create;

		struct DgramAddress
		{
			int fd;
			sockaddr* paddr;
		};

		ServerPrivate::Client::~Client()
		{
			destroy_native_event(ev_ended);
		}

		void ServerPrivate::Client::stop(bool passive)
		{
			std::lock_guard lock(mtx);
			if (fd)
			{
				closesocket(fd);
				fd = 0;
				if (passive)
					on_close();
			}
		}

		ServerPrivate::~ServerPrivate()
		{
			stop();
			wait_native_event(ev_ended_d, -1);
			wait_native_event(ev_ended_s, -1);
			for (auto& c : cs)
				wait_native_event(c->ev_ended, -1);
			destroy_native_event(ev_ended_d);
			destroy_native_event(ev_ended_s);
		}

		void ServerPrivate::stop()
		{
			std::lock_guard lock(mtx);
			if (fd_d)
			{
				closesocket(fd_d);
				fd_d = 0;
			}
			if (fd_s)
			{
				closesocket(fd_s);
				fd_s = 0;
			}
			for (auto& c : cs)
				c->stop(false);
		}

		void ServerPrivate::set_client(void* id, const std::function<void(std::string_view msg)>& on_message, const std::function<void()>& on_close)
		{
			auto client = (Client*)id;
			client->on_message = on_message;
			client->on_close = on_close;
		}

		void ServerPrivate::send(void* id, std::string_view msg, bool dgram)
		{
			if (!dgram)
				native_send(type, ((Client*)id)->fd, msg);
			else
			{
				auto& da = *(DgramAddress*)id;
				sendto(da.fd, (char*)msg.data(), msg.size(), 0, da.paddr, sizeof(sockaddr_in));
			}
		}

		struct ServerCreate : Server::Create 
		{
			ServerPtr operator()(SocketType type, uint port, const std::function<void(void* id, std::string_view msg)>& on_dgram, const std::function<void(void* id)>& on_connect) override
			{
				initialize();

				int res;
				sockaddr_in address;

				SOCKET fd_d = 0;
				if (on_dgram)
				{
					fd_d = socket(AF_INET, SOCK_DGRAM, 0);
					assert(fd_d != INVALID_SOCKET);
					address = {};
					address.sin_family = AF_INET;
					address.sin_addr.S_un.S_addr = INADDR_ANY;
					address.sin_port = htons(port);
					res = bind(fd_d, (sockaddr*)&address, sizeof(address));
					assert(res == 0);
				}

				auto fd_s = socket(AF_INET, SOCK_STREAM, 0);
				assert(fd_s != INVALID_SOCKET);
				address = {};
				address.sin_family = AF_INET;
				address.sin_addr.S_un.S_addr = INADDR_ANY;
				address.sin_port = htons(port);
				res = bind(fd_s, (sockaddr*)&address, sizeof(address));
				assert(res == 0);
				res = listen(fd_s, 1);
				assert(res == 0);

				auto s = new ServerPrivate;
				s->type = type;
				s->fd_d = fd_d;
				s->fd_s = fd_s;
				s->ev_ended_d = create_native_event(false, true);
				s->ev_ended_s = create_native_event(false, true);
				s->on_dgram = on_dgram;
				s->on_connect = on_connect;

				if (on_dgram)
				{
					std::thread([=]() {
						while (true)
						{
							char buf[1024 * 64];
							sockaddr_in address;
							int address_size = sizeof(address);
							auto res = recvfrom(s->fd_d, buf, sizeof(buf), 0, (sockaddr*)&address, &address_size);
							std::lock_guard lock(s->mtx);
							if (res <= 0)
							{
								if (s->fd_d)
								{
									closesocket(s->fd_d);
									s->fd_d = 0;
								}
								set_native_event(s->ev_ended_d);
								return;
							}
							DgramAddress da;
							da.fd = s->fd_d;
							da.paddr = (sockaddr*)&address;
							s->on_dgram(&da, { buf, (size_t)res });
						}
						}).detach();
				}

				std::thread([=]() {
					while (true)
					{
						auto fd = accept(s->fd_s, nullptr, nullptr);
						std::lock_guard lock(s->mtx);
						if (fd == INVALID_SOCKET)
						{
							s->stop();
							set_native_event(s->ev_ended_s);
							return;
						}
						auto c = new ServerPrivate::Client;
						c->fd = fd;
						c->ev_ended = create_native_event(false, true);
						s->on_connect(c);
						if (c->on_message || c->on_close)
							s->cs.emplace_back(c);
						else
						{
							closesocket(fd);
							delete c;
							continue;
						}

						std::thread([=]() {
							while (true)
							{
								std::vector<std::string> res;
								if (!native_recv(type, c->fd, res))
								{
									c->stop(true);
									set_native_event(c->ev_ended);
									return;
								}
								std::lock_guard lock(c->mtx);
								for (auto& r : res)
									c->on_message(r);
							}
							}).detach();
					}
				}).detach();

				return s;
			}
		}Server_create;
		Server::Create& Server::create = Server_create;

		FrameSyncServerPrivate::~FrameSyncServerPrivate()
		{
			stop();
			wait_native_event(ev_ended, -1);
			destroy_native_event(ev_ended);
		}

		bool FrameSyncServerPrivate::send(uint idx, std::string_view msg)
		{
			return native_send(type, fd_cs[idx], msg);
		}

		void FrameSyncServerPrivate::stop()
		{
			for (auto& fd : fd_cs)
			{
				if (fd)
				{
					closesocket(fd);
					fd = 0;
				}
			}
		}

		struct FrameSyncServerCreate : FrameSyncServer::Create
		{
			FrameSyncServerPtr operator()(SocketType type, uint port, uint num_clients) override
			{
				initialize();

				int res;

				auto fd_s = socket(AF_INET, SOCK_STREAM, 0);
				assert(fd_s != INVALID_SOCKET);
				sockaddr_in address = {};
				address.sin_family = AF_INET;
				address.sin_addr.S_un.S_addr = INADDR_ANY;
				address.sin_port = htons(port);
				res = bind(fd_s, (sockaddr*)&address, sizeof(address));
				assert(res == 0);
				res = listen(fd_s, num_clients);
				assert(res == 0);

				std::vector<int> fd_cs;
				while (fd_cs.size() < num_clients)
				{
					auto fd = accept(fd_s, nullptr, nullptr);
					if (fd == INVALID_SOCKET)
						return nullptr;

					if (!websocket_shakehand(type, fd))
						continue;

					fd_cs.push_back(fd);
				}

				closesocket(fd_s);

				{
					srand(time(0));
					nlohmann::json json = {
						{"action", "start"},
						{"seed", rand()}
					};
					auto str = json.dump();
					for (auto fd : fd_cs)
					{
						if (!native_send(type, fd, str))
							return nullptr;
					}
				}

				auto s = new FrameSyncServerPrivate;
				s->type = type;
				s->frame = 0;
				s->semaphore = 0;
				s->fd_cs = fd_cs;
				s->ev_ended = create_native_event(false, true);

				std::thread([type, s]() {
					while (true)
					{
						fd_set rfds;
						FD_ZERO(&rfds);
						for (auto fd : s->fd_cs)
							FD_SET(fd, &rfds);
						auto res = select(-1, &rfds, nullptr, nullptr, nullptr);
						if (res < 0)
						{
							s->stop();
							set_native_event(s->ev_ended);
							return;
						}
						for (auto i = 0; i < s->fd_cs.size(); i++)
						{
							auto fd = s->fd_cs[i];
							if (FD_ISSET(fd, &rfds))
							{
								std::vector<std::string> reqs;
								if (!native_recv(type, fd, reqs))
								{
									s->stop();
									set_native_event(s->ev_ended);
									return;
								}
								auto req = nlohmann::json::parse(reqs[0]);
								auto n_frame = req.find("frame");
								if (n_frame != req.end())
								{
									auto frame = n_frame->get<int>();
									if (frame == s->frame)
									{
										s->semaphore++;
										auto dst = s->frame_data[str(i)];
										for (auto& i : req["data"].items())
											dst[i.key()] = i.value();

										if (s->semaphore >= s->fd_cs.size())
										{
											s->frame++;
											s->semaphore = 0;

											s->frame_data["action"] = "frame";
											auto str = s->frame_data.dump();
											for (auto i = 0; i < 2; i++)
												s->send(i, str);
											s->frame_data.clear();
										}
									}
								}
							}
						}
					}
				}).detach();

				return s;
			}
		}FrameSyncServer_create;
		FrameSyncServer::Create& FrameSyncServer::create = FrameSyncServer_create;

		void board_cast(uint port, uint size, void* data, uint timeout, const std::function<void(const char* ip, std::string_view msg)>& on_message)
		{
			int res;

			auto fd = socket(AF_INET, SOCK_DGRAM, 0);
			assert(fd != INVALID_SOCKET);
			auto attr = 1;
			setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char*)&attr, sizeof(attr));
			sockaddr_in address;
			address.sin_family = AF_INET;
			address.sin_addr.S_un.S_addr = inet_addr("255.255.255.255");
			address.sin_port = htons(port);

			sendto(fd, (char*)data, size, 0, (sockaddr*)&address, sizeof(address));

			std::thread([=]() {
				int res;

				while (true)
				{
					timeval tv = { timeout, 0 };
					fd_set rfds;
					FD_ZERO(&rfds);
					FD_SET(fd, &rfds);
					res = select(-1, &rfds, nullptr, nullptr, &tv);
					if (res <= 0)
					{
						closesocket(fd);
						return;
					}

					char buf[1024 * 64];
					sockaddr_in address;
					int address_size = sizeof(address);
					res = recvfrom(fd, buf, sizeof(buf), 0, (sockaddr*)&address, &address_size);
					if (res <= 0)
					{
						closesocket(fd);
						return;
					}
					on_message(inet_ntoa(address.sin_addr), { buf, (size_t)res });
				}
			}).detach();
		}
	}
}
