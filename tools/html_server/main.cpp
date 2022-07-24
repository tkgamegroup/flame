#include <flame/foundation/system.h>
#include <flame/foundation/network.h>

#include <iostream>

using namespace flame;
using namespace flame::network;

Server* http_server = nullptr;
Server* rtp_stream_receiver = nullptr;
Server* udp_stream_receiver = nullptr;

void http_send_content(void* id, const std::string& content_type, const std::string& content, const std::vector<std::pair<std::string, std::string>>& additional_header = {})
{
	std::string reply;
	reply += "HTTP/1.1 200 OK\r\n";
	reply += "Connection: Keep-Alive\r\n";
	reply += std::format("Content-Type: {}\r\n", content_type);
	reply += std::format("Content-Length: {}\r\n", (int)content.size());
	for (auto& i : additional_header)
		reply += std::format("X-{}: {}\r\n", i.first, i.second);
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

// streaming with ffmpeg:
// ffmpeg -re -f dshow -i video="screen-capture-recorder":audio="virtual-audio-capturer" -c:v libx264 -c:a aac -movflags empty_moov+omit_tfhd_offset+frag_keyframe+default_base_moof -f mpegts udp://127.0.0.1:2345

const auto StreamPacketCount = 1000;
uint stream_id = 1;
std::vector<std::string> stream_datas;
std::mutex stream_mtx;

void append_stream_data(const std::string& data)
{
	stream_mtx.lock();
	stream_datas.push_back(data);
	if (stream_datas.size() % (StreamPacketCount * 2) == 0)
	{
		stream_datas.erase(stream_datas.begin(), stream_datas.begin() + StreamPacketCount);
		stream_id++;

		static auto first = true;
		if (first)
		{
			first = false;
			std::ofstream file(L"d:\\data\\test.mp4", std::ios::binary);
			std::string send_data;
			for (auto i = 0; i < stream_datas.size(); i++)
				send_data += stream_datas[i];
			file.write(send_data.data(), send_data.size());
			file.close();
		}
	}
	stream_mtx.unlock();
}

int main(int argc, char **args)
{
	Path::set_root(L"data", L"D:\\data");

	http_server = Server::create(SocketTcpRaw, 80, nullptr, [](void* id) {
		http_server->set_client(id, [id](const std::string& msg) {
			auto lines = SUS::split(msg, '\n');
			auto sp = SUS::split(lines[0], ' ');
			if (sp[0] == "GET")
			{
				auto sp2 = SUS::split(sp[1], '?');
				auto req_str = s2w(sp2.front());
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
					auto req_id = sp2.size() > 1 ? s2t<int>(sp2[1]) : -1;
					if (req_id == -1 || req_id < stream_id)
						req_id = stream_id;
					auto ok = false;
					while (!ok)
					{
						stream_mtx.lock();
						if (stream_id * StreamPacketCount + stream_datas.size() > (req_id + 1) * StreamPacketCount)
						{
							std::string send_data;
							auto base = (req_id - stream_id) * StreamPacketCount;
							for (auto i = 0; i < StreamPacketCount; i++)
								send_data += stream_datas[base + i];
							http_send_content(id, "video/mp4", send_data, { {"stream_id", str(req_id + 1)} });
							ok = true;
						}
						stream_mtx.unlock();
						sleep(1);
					}
				}
			}
		}, []() {
			int cut = 1; // disconnected
		});
	});

	rtp_stream_receiver = Server::create(SocketTcpRaw, 1234, [](void* id, const std::string& msg) {
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
		//printf("sequence number: %d, SSRC: %d, data-size: %d\n", header.sequence_number, header.SSRC, (int)(data_end - data));

		if (data_end > data)
			append_stream_data(std::string(data, data_end));
	}, nullptr);

	udp_stream_receiver = Server::create(SocketTcpRaw, 2345, [](void* id, const std::string& msg) {
		append_stream_data(msg);
	}, nullptr);

	while (true)
	{
		sleep(60000);
	}

	return 0;
}
