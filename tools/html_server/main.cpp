#include <flame/foundation/system.h>
#include <flame/foundation/network.h>

#include <iostream>

using namespace flame;
using namespace flame::network;

Server* http_server = nullptr;
Server* rtp_server = nullptr;

void http_send_content(void* id, const std::string& content_type, const std::string& content)
{
	std::string reply;
	reply += "HTTP/1.1 200 OK\r\n";
	reply += "Connection: Keep-Alive\r\n";
	reply += std::format("Content-Type: {}\r\n", content_type);
	reply += std::format("Content-Length: {}\r\n", (int)content.size());
	reply += "\r\n";
	reply += content;
	reply += "\r\n";

	http_server->send(id, reply);
}

void http_send_file(void* id, const std::filesystem::path& path)
{
	std::string content_type = "text/html";
	auto ext = path.extension();
	if (ext == L".mp4")
		content_type = "video/mp4";
	http_send_content(id, content_type, get_file_content(path));
}

struct StreamData
{
	uint n = 0;
	std::string data[65536];
	std::mutex mtx;
};

StreamData stream_data;

int main(int argc, char **args)
{
	Path::set_root(L"data", L"F:\\data");

	http_server = Server::create(SocketTcpRaw, 80, nullptr, [](void* id) {
		http_server->set_client(id, [id](const std::string& msg) {
			auto lines = SUS::split(msg, '\n');
			auto sp = SUS::split(lines[0], ' ');
			auto req_str = s2w(sp[1]);
			req_str.erase(req_str.begin());
			if (req_str.empty())
				req_str = L"index.html";
			auto path = std::filesystem::current_path() / req_str; 
			if (!std::filesystem::exists(path))
				path = Path::get(req_str);
			if (std::filesystem::exists(path))
				http_send_file(id, path);
			else if (req_str == L"stream")
			{
				stream_data.mtx.lock();
				if (stream_data.n == countof(stream_data.data))
				{
					std::string data;
					for (auto& d : stream_data.data)
						data += d;
					http_send_content(id, "video/mp4", data);
				}
				stream_data.mtx.unlock();
			}
		}, []() {
			int cut = 1; // disconnected
		});
	});

	rtp_server = Server::create(SocketTcpRaw, 1234, [](void* id, const std::string& msg) {
		auto data = msg.data();
		auto data_end = msg.data() + msg.size();
		struct RTPHeader
		{
			uint version : 2;
			uint padding : 1;
			uint extension : 1;
			uint CSRC_count : 4;
			uint marker : 1;
			uint payload_type : 7;
			uint sequence_number : 16;
			uint timestamp : 32;
			uint SSRC : 32;
		};
		auto& header = *(RTPHeader*)data; 
		data += sizeof(RTPHeader);
		std::vector<uint> CSRCs(header.CSRC_count);
		memcpy(CSRCs.data(), data, CSRCs.size() * sizeof(uint)); 
		data += CSRCs.size() * sizeof(uint);
		if (header.extension)
		{
			struct RTPExtensionHeader
			{
				uint id : 16;
				uint length : 16;
			};
			auto& header = *(RTPExtensionHeader*)data;
			data += header.length * sizeof(uint);
		}
		//printf("timestamp: %d\n", header.timestamp);
		printf("sequence number: %d, SSRC: %d, data-size: %d\n", header.sequence_number, header.SSRC, (int)(data_end - data));

		auto idx = header.sequence_number;
		stream_data.mtx.lock();
		if (stream_data.n < countof(stream_data.data))
		{
			if (stream_data.data[idx].empty())
			{
				stream_data.data[idx] = std::string(data, data_end);
				stream_data.n++;
				printf("data-number: %d\n", stream_data.n);
			}
		}
		stream_data.mtx.unlock();
	}, nullptr);

	while (true)
	{
		sleep(60000);
	}

	return 0;
}
