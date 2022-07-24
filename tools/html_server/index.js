var video_player_container = document.getElementById("video_player_container");
var video = document.createElement("video");
video.setAttribute("controls", "controls");
video_player_container.appendChild(video);
video.addEventListener("play", on_video_play, false);
video.addEventListener("pause", on_video_pause, false);

var media_source;
var source_buffer;

media_source = new MediaSource();
video.src = URL.createObjectURL(media_source);
media_source.addEventListener('sourceopen', function () {
	source_buffer = media_source.addSourceBuffer('video/mp4; codecs="avc1.42E01E, mp4a.40.2"');
	//make_request("data/test.mp4", xhr_ready);
}, false);

var xhr;

function on_video_play(e)
{
	//make_request("stream?-1", xhr_ready);
}

function on_video_pause(e)
{
	//if (!xhr)
	//	return;
 //   xhr.removeEventListener("load", xhr_ready, false);
 //   xhr.abort();
 //   init_media_source();
}

function make_request(url, callback)
{
	xhr = new XMLHttpRequest();
	xhr.responseType = "arraybuffer";
	xhr.addEventListener("load", callback, false);
	xhr.open("GET", url, true);
	xhr.send();
}

function xhr_ready(e)
{
	var src = e.target;
	if (src.status != 200)
		return;

	source_buffer.addEventListener("updateend", function () {
		media_source.endOfStream();
		video.play();
	}, false);
	source_buffer.appendBuffer(src.response);

	//xhr = make_request("stream?" + src.getResponseHeader('X-stream_id'), xhr_ready);
}
