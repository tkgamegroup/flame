var video_player_container = document.getElementById("video_player_container");
var video = document.createElement("video");
video.setAttribute("controls", "controls");
video_player_container.appendChild(video);
video.addEventListener("play", on_video_play, false);
video.addEventListener("pause", on_video_pause, false);

var media_source;

function init_media_source()
{
	media_source = new window.MediaSource();
	if (video.src)
		window.URL.revokeObjectURL(video.src);
	video.src = window.URL.createObjectURL(media_source);
}

init_media_source();

var xhr;

function on_video_play(e)
{
	make_request("stream", xhr_ready);
}

function on_video_pause(e)
{
    xhr.removeEventListener("load", xhr_ready, false);
    xhr.abort();
    init_media_source();
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
	var src = e.targetp;
	if (src.status != 200)
		return;

	var source_buffer;
	if (media_source.sourceBuffers.length > 0)
		source_buffer = media_source.sourceBuffers[0];
	else
	{
		source_buffer = media_source.addSourceBuffer("video/mp4");
		/*
		source_buffer.addEventListener("updateend", function(){
			if (video.currentTime === 0 && video.readyState > 0)
				video.currentTime = source_buffer.bnffered.start(0);
		}, false)
		*/
	}
	source_buffer.appendBuffer(src.response);

	xhr = make_request("stream", xhr_ready);
}
