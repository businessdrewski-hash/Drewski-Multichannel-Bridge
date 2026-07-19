#include "deep-timing-recorder.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {
void require(bool condition, const char *message)
{
	if (!condition) {
		std::cerr << "FAILED: " << message << '\n';
		std::exit(1);
	}
}
} // namespace

int main()
{
	mcb::DeepTimingRecorder recorder;
	require(!recorder.enabled(), "recorder must default off");
	recorder.set_enabled(true, 1000000000ULL);
	require(recorder.enabled(), "recorder did not enable");

	recorder.observe_raw_audio(1000, 2000, 3000000000ULL, 4000000000ULL, 1024, 48000, 4);
	recorder.observe_raw_audio(214333, 215333, 3021333333ULL, 4021333333ULL, 1024, 48000, 4);
	recorder.observe_raw_video(1100, 2100, 3000000000ULL, 4000000000ULL, 1920, 1080);
	recorder.observe_selected_video(3033333333ULL, 4033333333ULL);
	recorder.observe_audio_input(0, 3021333333ULL, 4021333333ULL, 1024, 48000);
	recorder.observe_audio_output(0, 3021333333ULL, 4021333333ULL, 1024, 1023, 48000,
		3042666666ULL, 3042645833ULL, -1, -25.0);

	mcb::DeepTimingControlSnapshot control;
	control.governor_enabled = true;
	control.baseline_valid = true;
	control.measurement_fresh = true;
	control.phase = 2;
	control.raw_deviation_ns = 1250000;
	control.correction_ppm = -25.0;
	control.ndi_sync_mode = 2;
	control.ndi_audio_enabled = true;
	mcb::DeepTimingAudioCursors cursors;
	cursors.program_proxy_ns = 3021333333ULL;
	recorder.sample(4250000000ULL, control, cursors);
	require(recorder.sample_count() == 1, "sample was not retained");

	const std::string csv = recorder.csv();
	require(csv.find("raw_audio_ndi_timestamp_100ns") != std::string::npos,
		"raw NDI field missing");
	require(csv.find("program_audio_cursor_ns") != std::string::npos,
		"OBS cursor field missing");
	require(csv.find("program_cursor_minus_program_output_ns") != std::string::npos,
		"OBS ingest-boundary relation missing");
	require(csv.find("logging_enabled") != std::string::npos,
		"enable event missing");
	require(csv.find(",-25.0000,") != std::string::npos,
		"correction command missing");

	recorder.set_enabled(false, 5000000000ULL);
	require(!recorder.enabled(), "recorder did not stop");
	require(recorder.sample_count() == 1, "stopping discarded the export buffer");
	recorder.sample(5250000000ULL, control, cursors);
	require(recorder.sample_count() == 1, "disabled recorder accepted a sample");

	recorder.set_enabled(true, 6000000000ULL);
	require(recorder.sample_count() == 0, "new diagnostic session did not start fresh");
	std::cout << "Deep timing recorder tests passed\n";
	return 0;
}
