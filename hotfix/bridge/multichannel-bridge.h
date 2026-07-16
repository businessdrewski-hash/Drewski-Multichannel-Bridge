#pragma once

#include <cstddef>
#include <cstdint>

class QWidget;
struct obs_source;
typedef struct obs_source obs_source_t;
struct obs_source_audio;

enum class MCBRole {
	Unconfigured = 0,
	Sender = 1,
	Receiver = 2,
};

struct MCBSenderStatus {
	bool enabled = false;
	bool active = false;
	uint32_t track_a = 5;
	uint32_t track_b = 6;
	uint64_t paired_blocks = 0;
	uint64_t discarded_blocks = 0;
	uint64_t silence_fallback_blocks = 0;
	int64_t last_timestamp_delta_ns = 0;
	uint32_t queue_depth_a = 0;
	uint32_t queue_depth_b = 0;
	float peak_a = 0.0f;
	float peak_b = 0.0f;
	uint64_t last_audio_monotonic_ns = 0;
};

MCBRole mcb_role();
bool mcb_is_sender();
bool mcb_is_receiver();
size_t mcb_sender_track_a_zero_based();
size_t mcb_sender_track_b_zero_based();

void mcb_register_sources();
void mcb_init(QWidget *main_window);
void mcb_shutdown();

MCBSenderStatus mcb_sender_status_snapshot();
void mcb_sender_status_started(size_t track_a_zero_based, size_t track_b_zero_based);
void mcb_sender_status_stopped();
void mcb_sender_status_paired(int64_t timestamp_delta_ns, size_t queue_a, size_t queue_b);
void mcb_sender_status_discarded(int64_t timestamp_delta_ns, size_t queue_a, size_t queue_b);
void mcb_sender_status_silence_fallback(size_t queue_a, size_t queue_b);
void mcb_sender_status_levels(float peak_a, float peak_b);
void mcb_sender_status_reset_counters();

// Called from DistroAV's raw NDI receive path before OBS remixes the source to
// the profile speaker layout. Returns true when the original 4-channel source
// should be suppressed because both split proxy outputs are ready.
bool mcb_receiver_route_audio(obs_source_t *origin, const obs_source_audio *audio, int channel_count);
