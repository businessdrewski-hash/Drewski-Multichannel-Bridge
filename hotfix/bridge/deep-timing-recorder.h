#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace mcb {

enum class DeepTimingEvent : uint8_t {
	None = 0,
	LoggingEnabled,
	ReceiverAttached,
	ReceiverDetached,
	ReceiverReconnectFresh,
	ReceiverReconnectVerify,
	CountersReset,
};

struct DeepTimingStageSnapshot {
	int64_t ndi_timestamp_100ns = 0;
	int64_t ndi_timecode_100ns = 0;
	uint64_t timestamp_ns = 0;
	uint64_t wall_ns = 0;
	int64_t timestamp_delta_ns = 0;
	int64_t wall_delta_ns = 0;
	int64_t ndi_timestamp_delta_100ns = 0;
	int64_t ndi_timecode_delta_100ns = 0;
	uint32_t unit_a = 0;
	uint32_t unit_b = 0;
	uint32_t unit_c = 0;
	uint64_t observations = 0;
	uint64_t pacing_anomalies = 0;
};

struct DeepTimingControlSnapshot {
	bool governor_enabled = false;
	bool baseline_valid = false;
	bool measurement_fresh = false;
	uint8_t phase = 0;
	int64_t relation_ns = 0;
	int64_t baseline_ns = 0;
	int64_t raw_deviation_ns = 0;
	int64_t corrected_deviation_ns = 0;
	int64_t drift_ppm = 0;
	int64_t native_audio_error_ppm = 0;
	double correction_ppm = 0.0;
	double target_ppm = 0.0;
	uint32_t confidence = 0;
	uint32_t drift_samples = 0;
	uint64_t discontinuities = 0;
	uint64_t quarantined_samples = 0;
	uint64_t corrected_blocks = 0;
	int64_t net_frame_adjustment = 0;
	bool ndi_frame_sync = false;
	int32_t ndi_sync_mode = -1;
	bool ndi_audio_enabled = false;
	int32_t ndi_behavior = -1;
};

struct DeepTimingAudioCursors {
	uint64_t canonical_source_ns = 0;
	uint64_t program_proxy_ns = 0;
	uint64_t mic_proxy_ns = 0;
};

// Callback-facing methods only publish atomics. Ring allocation, sampling and
// CSV formatting happen on the Qt control thread after diagnostics are enabled.
class DeepTimingRecorder {
public:
	static constexpr size_t kCapacity = 43200; // three hours at the 250 ms controller cadence

	void set_enabled(bool enabled, uint64_t wall_ns);
	bool enabled() const noexcept { return enabled_.load(std::memory_order_acquire); }
	void mark_event(DeepTimingEvent event, uint64_t wall_ns) noexcept;

	void observe_raw_audio(int64_t ndi_timestamp_100ns, int64_t ndi_timecode_100ns,
		uint64_t timestamp_ns, uint64_t wall_ns, uint32_t frames,
		uint32_t sample_rate, uint32_t channels) noexcept;
	void observe_raw_video(int64_t ndi_timestamp_100ns, int64_t ndi_timecode_100ns,
		uint64_t timestamp_ns, uint64_t wall_ns, uint32_t width, uint32_t height) noexcept;
	void observe_selected_video(uint64_t timestamp_ns, uint64_t wall_ns) noexcept;
	void observe_audio_input(int pair, uint64_t timestamp_ns, uint64_t wall_ns,
		uint32_t frames, uint32_t sample_rate) noexcept;
	void observe_audio_output(int pair, uint64_t timestamp_ns, uint64_t wall_ns,
		uint32_t input_frames, uint32_t output_frames, uint32_t sample_rate,
		uint64_t expected_input_timestamp_ns, uint64_t next_output_timestamp_ns,
		int64_t net_frame_adjustment, double correction_ppm) noexcept;
	void note_route_contention(bool video) noexcept;

	void sample(uint64_t wall_ns, const DeepTimingControlSnapshot &control,
		const DeepTimingAudioCursors &cursors);
	std::string csv() const;
	size_t sample_count() const;
	uint64_t overwritten_samples() const;

private:
	struct StageAtomics {
		std::atomic<uint64_t> sequence{0};
		std::atomic<int64_t> ndi_timestamp_100ns{0};
		std::atomic<int64_t> ndi_timecode_100ns{0};
		std::atomic<uint64_t> timestamp_ns{0};
		std::atomic<uint64_t> wall_ns{0};
		std::atomic<int64_t> timestamp_delta_ns{0};
		std::atomic<int64_t> wall_delta_ns{0};
		std::atomic<int64_t> ndi_timestamp_delta_100ns{0};
		std::atomic<int64_t> ndi_timecode_delta_100ns{0};
		std::atomic<uint32_t> unit_a{0};
		std::atomic<uint32_t> unit_b{0};
		std::atomic<uint32_t> unit_c{0};
		std::atomic<uint64_t> observations{0};
		std::atomic<uint64_t> pacing_anomalies{0};
	};

	struct OutputExtras {
		std::atomic<uint64_t> expected_input_timestamp_ns{0};
		std::atomic<uint64_t> next_output_timestamp_ns{0};
		std::atomic<int64_t> net_frame_adjustment{0};
		std::atomic<double> correction_ppm{0.0};
	};

	struct Sample {
		uint64_t wall_ns = 0;
		uint64_t session = 0;
		DeepTimingEvent event = DeepTimingEvent::None;
		uint64_t event_generation = 0;
		uint64_t event_wall_ns = 0;
		uint64_t overwritten_at_capture = 0;
		uint64_t audio_route_contention = 0;
		uint64_t video_route_contention = 0;
		DeepTimingStageSnapshot raw_audio;
		DeepTimingStageSnapshot raw_video;
		DeepTimingStageSnapshot selected_video;
		std::array<DeepTimingStageSnapshot, 2> audio_input{};
		std::array<DeepTimingStageSnapshot, 2> audio_output{};
		std::array<uint64_t, 2> expected_input_timestamp_ns{};
		std::array<uint64_t, 2> next_output_timestamp_ns{};
		std::array<int64_t, 2> output_net_frame_adjustment{};
		std::array<double, 2> output_correction_ppm{};
		DeepTimingControlSnapshot control;
		DeepTimingAudioCursors cursors;
	};

	static int64_t signed_delta(uint64_t current, uint64_t previous) noexcept;
	static int64_t signed_delta(int64_t current, int64_t previous) noexcept;
	static const char *event_name(DeepTimingEvent event) noexcept;
	static void clear_stage(StageAtomics &stage) noexcept;
	static void publish(StageAtomics &stage, int64_t ndi_timestamp_100ns,
		int64_t ndi_timecode_100ns, uint64_t timestamp_ns, uint64_t wall_ns,
		uint32_t unit_a, uint32_t unit_b, uint32_t unit_c) noexcept;
	static DeepTimingStageSnapshot read_stage(const StageAtomics &stage) noexcept;

	std::atomic_bool enabled_{false};
	std::atomic<uint64_t> session_{0};
	std::atomic<uint8_t> last_event_{static_cast<uint8_t>(DeepTimingEvent::None)};
	std::atomic<uint64_t> last_event_wall_ns_{0};
	std::atomic<uint64_t> event_generation_{0};
	std::atomic<uint64_t> audio_route_contention_{0};
	std::atomic<uint64_t> video_route_contention_{0};
	StageAtomics raw_audio_;
	StageAtomics raw_video_;
	StageAtomics selected_video_;
	std::array<StageAtomics, 2> audio_input_{};
	std::array<StageAtomics, 2> audio_output_{};
	std::array<OutputExtras, 2> output_extras_{};

	mutable std::mutex ring_mutex_;
	std::vector<Sample> ring_;
	size_t write_index_ = 0;
	size_t count_ = 0;
	uint64_t overwritten_ = 0;
	uint64_t sampled_event_generation_ = 0;
};

} // namespace mcb
