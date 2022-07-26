#include <flame/foundation/system.h>
#include <flame/foundation/network.h>

#include <iostream>

using namespace flame;
using namespace flame::network;

Server* http_server = nullptr;

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
// ffmpeg -re -f dshow -i video="screen-capture-recorder":audio="virtual-audio-capturer" -c:v libx264 -c:a aac -keyint_min 150 -g 150 -f hls -hls_time 2 -hls_flags split_by_time -t 10 D:\data\out.m3u8

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
			}
		}, []() {
			int cut = 1; // disconnected
		});
	});

	while (true)
	{
		sleep(60000);
	}

	return 0;
}
