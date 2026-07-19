#include "deep-timing-recorder.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <sstream>

namespace mcb {

int64_t DeepTimingRecorder::signed_delta(uint64_t current, uint64_t previous) noexcept
{
	if (current >= previous) {
		const uint64_t delta = current - previous;
		return delta > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())
			? std::numeric_limits<int64_t>::max() : static_cast<int64_t>(delta);
	}
	const uint64_t delta = previous - current;
	return delta > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())
		? std::numeric_limits<int64_t>::min() : -static_cast<int64_t>(delta);
}

int64_t DeepTimingRecorder::signed_delta(int64_t current, int64_t previous) noexcept
{
	if ((previous > 0 && current < std::numeric_limits<int64_t>::min() + previous) ||
		(previous < 0 && current > std::numeric_limits<int64_t>::max() + previous))
		return current >= previous ? std::numeric_limits<int64_t>::max()
			: std::numeric_limits<int64_t>::min();
	return current - previous;
}

const char *DeepTimingRecorder::event_name(DeepTimingEvent event) noexcept
{
	switch (event) {
	case DeepTimingEvent::None: return "";
	case DeepTimingEvent::LoggingEnabled: return "logging_enabled";
	case DeepTimingEvent::ReceiverAttached: return "receiver_attached";
	case DeepTimingEvent::ReceiverDetached: return "receiver_detached";
	case DeepTimingEvent::ReceiverReconnectFresh: return "receiver_reconnect_fresh";
	case DeepTimingEvent::ReceiverReconnectVerify: return "receiver_reconnect_verify";
	case DeepTimingEvent::CountersReset: return "counters_reset";
	}
	return "unknown";
}

void DeepTimingRecorder::clear_stage(StageAtomics &stage) noexcept
{
	stage.sequence.store(0, std::memory_order_relaxed);
	stage.ndi_timestamp_100ns.store(0, std::memory_order_relaxed);
	stage.ndi_timecode_100ns.store(0, std::memory_order_relaxed);
	stage.timestamp_ns.store(0, std::memory_order_relaxed);
	stage.wall_ns.store(0, std::memory_order_relaxed);
	stage.timestamp_delta_ns.store(0, std::memory_order_relaxed);
	stage.wall_delta_ns.store(0, std::memory_order_relaxed);
	stage.ndi_timestamp_delta_100ns.store(0, std::memory_order_relaxed);
	stage.ndi_timecode_delta_100ns.store(0, std::memory_order_relaxed);
	stage.unit_a.store(0, std::memory_order_relaxed);
	stage.unit_b.store(0, std::memory_order_relaxed);
	stage.unit_c.store(0, std::memory_order_relaxed);
	stage.observations.store(0, std::memory_order_relaxed);
	stage.pacing_anomalies.store(0, std::memory_order_relaxed);
}

void DeepTimingRecorder::publish(StageAtomics &stage, int64_t ndi_timestamp_100ns,
	int64_t ndi_timecode_100ns, uint64_t timestamp_ns, uint64_t wall_ns,
	uint32_t unit_a, uint32_t unit_b, uint32_t unit_c) noexcept
{
	const uint64_t prior_timestamp = stage.timestamp_ns.load(std::memory_order_relaxed);
	const uint64_t prior_wall = stage.wall_ns.load(std::memory_order_relaxed);
	const int64_t prior_ndi_timestamp = stage.ndi_timestamp_100ns.load(std::memory_order_relaxed);
	const int64_t prior_ndi_timecode = stage.ndi_timecode_100ns.load(std::memory_order_relaxed);
	const int64_t timestamp_delta = prior_timestamp ? signed_delta(timestamp_ns, prior_timestamp) : 0;
	const int64_t wall_delta = prior_wall ? signed_delta(wall_ns, prior_wall) : 0;
	const int64_t ndi_timestamp_delta = prior_ndi_timestamp
		? signed_delta(ndi_timestamp_100ns, prior_ndi_timestamp) : 0;
	const int64_t ndi_timecode_delta = prior_ndi_timecode
		? signed_delta(ndi_timecode_100ns, prior_ndi_timecode) : 0;

	stage.sequence.fetch_add(1, std::memory_order_acq_rel);
	stage.ndi_timestamp_100ns.store(ndi_timestamp_100ns, std::memory_order_relaxed);
	stage.ndi_timecode_100ns.store(ndi_timecode_100ns, std::memory_order_relaxed);
	stage.timestamp_ns.store(timestamp_ns, std::memory_order_relaxed);
	stage.wall_ns.store(wall_ns, std::memory_order_relaxed);
	stage.timestamp_delta_ns.store(timestamp_delta, std::memory_order_relaxed);
	stage.wall_delta_ns.store(wall_delta, std::memory_order_relaxed);
	stage.ndi_timestamp_delta_100ns.store(ndi_timestamp_delta, std::memory_order_relaxed);
	stage.ndi_timecode_delta_100ns.store(ndi_timecode_delta, std::memory_order_relaxed);
	stage.unit_a.store(unit_a, std::memory_order_relaxed);
	stage.unit_b.store(unit_b, std::memory_order_relaxed);
	stage.unit_c.store(unit_c, std::memory_order_relaxed);
	stage.observations.fetch_add(1, std::memory_order_relaxed);
	if (prior_timestamp && prior_wall &&
		(std::llabs(timestamp_delta - wall_delta) > 2000000LL || timestamp_delta <= 0))
		stage.pacing_anomalies.fetch_add(1, std::memory_order_relaxed);
	stage.sequence.fetch_add(1, std::memory_order_release);
}

DeepTimingStageSnapshot DeepTimingRecorder::read_stage(const StageAtomics &stage) noexcept
{
	DeepTimingStageSnapshot result;
	for (int attempt = 0; attempt < 4; ++attempt) {
		const uint64_t before = stage.sequence.load(std::memory_order_acquire);
		if (before & 1ULL)
			continue;
		result.ndi_timestamp_100ns = stage.ndi_timestamp_100ns.load(std::memory_order_relaxed);
		result.ndi_timecode_100ns = stage.ndi_timecode_100ns.load(std::memory_order_relaxed);
		result.timestamp_ns = stage.timestamp_ns.load(std::memory_order_relaxed);
		result.wall_ns = stage.wall_ns.load(std::memory_order_relaxed);
		result.timestamp_delta_ns = stage.timestamp_delta_ns.load(std::memory_order_relaxed);
		result.wall_delta_ns = stage.wall_delta_ns.load(std::memory_order_relaxed);
		result.ndi_timestamp_delta_100ns = stage.ndi_timestamp_delta_100ns.load(std::memory_order_relaxed);
		result.ndi_timecode_delta_100ns = stage.ndi_timecode_delta_100ns.load(std::memory_order_relaxed);
		result.unit_a = stage.unit_a.load(std::memory_order_relaxed);
		result.unit_b = stage.unit_b.load(std::memory_order_relaxed);
		result.unit_c = stage.unit_c.load(std::memory_order_relaxed);
		result.observations = stage.observations.load(std::memory_order_relaxed);
		result.pacing_anomalies = stage.pacing_anomalies.load(std::memory_order_relaxed);
		const uint64_t after = stage.sequence.load(std::memory_order_acquire);
		if (before == after && !(after & 1ULL))
			break;
	}
	return result;
}

void DeepTimingRecorder::set_enabled(bool enabled, uint64_t wall_ns)
{
	if (enabled == enabled_.load(std::memory_order_acquire))
		return;
	enabled_.store(false, std::memory_order_release);
	// Stopping capture deliberately preserves the completed flight recorder so
	// the user can turn logging off before opening the export dialog.
	if (!enabled)
		return;

	clear_stage(raw_audio_);
	clear_stage(raw_video_);
	clear_stage(selected_video_);
	for (size_t pair = 0; pair < 2; ++pair) {
		clear_stage(audio_input_[pair]);
		clear_stage(audio_output_[pair]);
		output_extras_[pair].expected_input_timestamp_ns.store(0, std::memory_order_relaxed);
		output_extras_[pair].next_output_timestamp_ns.store(0, std::memory_order_relaxed);
		output_extras_[pair].net_frame_adjustment.store(0, std::memory_order_relaxed);
		output_extras_[pair].correction_ppm.store(0.0, std::memory_order_relaxed);
	}
	audio_route_contention_.store(0, std::memory_order_relaxed);
	video_route_contention_.store(0, std::memory_order_relaxed);
	{
		std::lock_guard<std::mutex> lock(ring_mutex_);
		ring_.assign(kCapacity, Sample{});
		write_index_ = 0;
		count_ = 0;
		overwritten_ = 0;
		sampled_event_generation_ = 0;
	}
	session_.fetch_add(1, std::memory_order_acq_rel);
	last_event_.store(static_cast<uint8_t>(DeepTimingEvent::LoggingEnabled), std::memory_order_relaxed);
	last_event_wall_ns_.store(wall_ns, std::memory_order_relaxed);
	event_generation_.fetch_add(1, std::memory_order_acq_rel);
	enabled_.store(true, std::memory_order_release);
}

void DeepTimingRecorder::mark_event(DeepTimingEvent event, uint64_t wall_ns) noexcept
{
	if (!enabled() || event == DeepTimingEvent::None)
		return;
	last_event_.store(static_cast<uint8_t>(event), std::memory_order_relaxed);
	last_event_wall_ns_.store(wall_ns, std::memory_order_relaxed);
	event_generation_.fetch_add(1, std::memory_order_acq_rel);
}

void DeepTimingRecorder::observe_raw_audio(int64_t ndi_timestamp_100ns,
	int64_t ndi_timecode_100ns, uint64_t timestamp_ns, uint64_t wall_ns,
	uint32_t frames, uint32_t sample_rate, uint32_t channels) noexcept
{
	if (enabled())
		publish(raw_audio_, ndi_timestamp_100ns, ndi_timecode_100ns,
			timestamp_ns, wall_ns, frames, sample_rate, channels);
}

void DeepTimingRecorder::observe_raw_video(int64_t ndi_timestamp_100ns,
	int64_t ndi_timecode_100ns, uint64_t timestamp_ns, uint64_t wall_ns,
	uint32_t width, uint32_t height) noexcept
{
	if (enabled())
		publish(raw_video_, ndi_timestamp_100ns, ndi_timecode_100ns,
			timestamp_ns, wall_ns, width, height, 0);
}

void DeepTimingRecorder::observe_selected_video(uint64_t timestamp_ns, uint64_t wall_ns) noexcept
{
	if (enabled())
		publish(selected_video_, 0, 0, timestamp_ns, wall_ns, 0, 0, 0);
}

void DeepTimingRecorder::observe_audio_input(int pair, uint64_t timestamp_ns,
	uint64_t wall_ns, uint32_t frames, uint32_t sample_rate) noexcept
{
	if (enabled() && pair >= 0 && pair < 2)
		publish(audio_input_[static_cast<size_t>(pair)], 0, 0,
			timestamp_ns, wall_ns, frames, sample_rate, 0);
}

void DeepTimingRecorder::observe_audio_output(int pair, uint64_t timestamp_ns,
	uint64_t wall_ns, uint32_t input_frames, uint32_t output_frames,
	uint32_t sample_rate, uint64_t expected_input_timestamp_ns,
	uint64_t next_output_timestamp_ns, int64_t net_frame_adjustment,
	double correction_ppm) noexcept
{
	if (!enabled() || pair < 0 || pair >= 2)
		return;
	const size_t index = static_cast<size_t>(pair);
	publish(audio_output_[index], 0, 0, timestamp_ns, wall_ns,
		input_frames, output_frames, sample_rate);
	output_extras_[index].expected_input_timestamp_ns.store(
		expected_input_timestamp_ns, std::memory_order_relaxed);
	output_extras_[index].next_output_timestamp_ns.store(
		next_output_timestamp_ns, std::memory_order_relaxed);
	output_extras_[index].net_frame_adjustment.store(net_frame_adjustment, std::memory_order_relaxed);
	output_extras_[index].correction_ppm.store(correction_ppm, std::memory_order_relaxed);
}

void DeepTimingRecorder::note_route_contention(bool video) noexcept
{
	if (!enabled())
		return;
	(video ? video_route_contention_ : audio_route_contention_).fetch_add(1, std::memory_order_relaxed);
}

void DeepTimingRecorder::sample(uint64_t wall_ns,
	const DeepTimingControlSnapshot &control, const DeepTimingAudioCursors &cursors)
{
	if (!enabled())
		return;
	Sample row;
	row.wall_ns = wall_ns;
	row.session = session_.load(std::memory_order_relaxed);
	row.event_generation = event_generation_.load(std::memory_order_acquire);
	row.audio_route_contention = audio_route_contention_.load(std::memory_order_relaxed);
	row.video_route_contention = video_route_contention_.load(std::memory_order_relaxed);
	row.raw_audio = read_stage(raw_audio_);
	row.raw_video = read_stage(raw_video_);
	row.selected_video = read_stage(selected_video_);
	for (size_t pair = 0; pair < 2; ++pair) {
		row.audio_input[pair] = read_stage(audio_input_[pair]);
		row.audio_output[pair] = read_stage(audio_output_[pair]);
		row.expected_input_timestamp_ns[pair] = output_extras_[pair].expected_input_timestamp_ns.load(std::memory_order_relaxed);
		row.next_output_timestamp_ns[pair] = output_extras_[pair].next_output_timestamp_ns.load(std::memory_order_relaxed);
		row.output_net_frame_adjustment[pair] = output_extras_[pair].net_frame_adjustment.load(std::memory_order_relaxed);
		row.output_correction_ppm[pair] = output_extras_[pair].correction_ppm.load(std::memory_order_relaxed);
	}
	row.control = control;
	row.cursors = cursors;

	std::lock_guard<std::mutex> lock(ring_mutex_);
	if (ring_.empty())
		return;
	if (row.event_generation != sampled_event_generation_) {
		row.event = static_cast<DeepTimingEvent>(last_event_.load(std::memory_order_relaxed));
		row.event_wall_ns = last_event_wall_ns_.load(std::memory_order_relaxed);
		sampled_event_generation_ = row.event_generation;
	}
	row.overwritten_at_capture = overwritten_;
	ring_[write_index_] = row;
	write_index_ = (write_index_ + 1) % ring_.size();
	if (count_ < ring_.size())
		++count_;
	else
		++overwritten_;
}

namespace {
void write_stage_header(std::ostringstream &out, const char *prefix,
	const char *unit_a, const char *unit_b, const char *unit_c)
{
	out << ',' << prefix << "_timestamp_ns," << prefix << "_wall_ns,"
		<< prefix << "_timestamp_delta_ns," << prefix << "_wall_delta_ns,"
		<< prefix << "_ndi_timestamp_100ns," << prefix << "_ndi_timecode_100ns,"
		<< prefix << "_ndi_timestamp_delta_100ns," << prefix << "_ndi_timecode_delta_100ns,"
		<< prefix << '_' << unit_a << ',' << prefix << '_' << unit_b << ',' << prefix << '_' << unit_c << ','
		<< prefix << "_observations," << prefix << "_pacing_anomalies";
}

void write_stage(std::ostringstream &out, const DeepTimingStageSnapshot &stage)
{
	out << ',' << stage.timestamp_ns << ',' << stage.wall_ns << ','
		<< stage.timestamp_delta_ns << ',' << stage.wall_delta_ns << ','
		<< stage.ndi_timestamp_100ns << ',' << stage.ndi_timecode_100ns << ','
		<< stage.ndi_timestamp_delta_100ns << ',' << stage.ndi_timecode_delta_100ns << ','
		<< stage.unit_a << ',' << stage.unit_b << ',' << stage.unit_c << ','
		<< stage.observations << ',' << stage.pacing_anomalies;
}
} // namespace

std::string DeepTimingRecorder::csv() const
{
	std::lock_guard<std::mutex> lock(ring_mutex_);
	std::ostringstream out;
	out << "sample_wall_ns,session,event,event_generation,event_wall_ns,overwritten_samples,audio_route_contention,video_route_contention";
	write_stage_header(out, "raw_audio", "frames", "sample_rate", "channels");
	write_stage_header(out, "raw_video", "width", "height", "unused");
	write_stage_header(out, "selected_video", "unused_a", "unused_b", "unused_c");
	write_stage_header(out, "program_input", "frames", "sample_rate", "unused");
	write_stage_header(out, "mic_input", "frames", "sample_rate", "unused");
	write_stage_header(out, "program_output", "input_frames", "output_frames", "sample_rate");
	write_stage_header(out, "mic_output", "input_frames", "output_frames", "sample_rate");
	out << ",program_expected_input_timestamp_ns,program_next_output_timestamp_ns,program_net_frame_adjustment,program_correction_ppm"
		",mic_expected_input_timestamp_ns,mic_next_output_timestamp_ns,mic_net_frame_adjustment,mic_correction_ppm"
		",canonical_source_audio_cursor_ns,program_audio_cursor_ns,mic_audio_cursor_ns"
		",raw_video_minus_raw_audio_projected_ns,selected_video_minus_raw_video_projected_ns"
		",program_input_minus_raw_audio_projected_ns,program_output_minus_program_input_ns"
		",program_cursor_minus_program_output_ns,mic_cursor_minus_mic_output_ns,program_minus_mic_cursor_ns"
		",governor_enabled,baseline_valid,measurement_fresh,phase,relation_ns,baseline_ns,raw_deviation_ns,corrected_deviation_ns"
		",drift_ppm,native_audio_error_ppm,controller_correction_ppm,target_ppm,confidence,drift_samples,discontinuities"
		",quarantined_samples,corrected_blocks,controller_net_frame_adjustment,ndi_frame_sync,ndi_sync_mode,ndi_audio_enabled,ndi_behavior\n";
	if (ring_.empty() || !count_)
		return out.str();
	const size_t first = count_ == ring_.size() ? write_index_ : 0;
	out.setf(std::ios::fixed);
	out.precision(4);
	for (size_t offset = 0; offset < count_; ++offset) {
		const Sample &row = ring_[(first + offset) % ring_.size()];
		out << row.wall_ns << ',' << row.session << ',' << event_name(row.event) << ','
			<< row.event_generation << ',' << row.event_wall_ns << ',' << row.overwritten_at_capture << ','
			<< row.audio_route_contention << ',' << row.video_route_contention;
		write_stage(out, row.raw_audio);
		write_stage(out, row.raw_video);
		write_stage(out, row.selected_video);
		write_stage(out, row.audio_input[0]);
		write_stage(out, row.audio_input[1]);
		write_stage(out, row.audio_output[0]);
		write_stage(out, row.audio_output[1]);
		for (size_t pair = 0; pair < 2; ++pair) {
			out << ',' << row.expected_input_timestamp_ns[pair]
				<< ',' << row.next_output_timestamp_ns[pair]
				<< ',' << row.output_net_frame_adjustment[pair]
				<< ',' << row.output_correction_ppm[pair];
		}
		const auto &c = row.control;
		auto projected_relation = [](const DeepTimingStageSnapshot &a,
			const DeepTimingStageSnapshot &b) {
			if (!a.timestamp_ns || !a.wall_ns || !b.timestamp_ns || !b.wall_ns)
				return int64_t{0};
			return DeepTimingRecorder::signed_delta(a.timestamp_ns, b.timestamp_ns) +
				DeepTimingRecorder::signed_delta(b.wall_ns, a.wall_ns);
		};
		auto direct_relation = [](uint64_t a, uint64_t b) {
			return a && b ? DeepTimingRecorder::signed_delta(a, b) : int64_t{0};
		};
		out << ',' << row.cursors.canonical_source_ns << ',' << row.cursors.program_proxy_ns
			<< ',' << row.cursors.mic_proxy_ns
			<< ',' << projected_relation(row.raw_video, row.raw_audio)
			<< ',' << projected_relation(row.selected_video, row.raw_video)
			<< ',' << projected_relation(row.audio_input[0], row.raw_audio)
			<< ',' << direct_relation(row.audio_output[0].timestamp_ns, row.audio_input[0].timestamp_ns)
			<< ',' << direct_relation(row.cursors.program_proxy_ns, row.audio_output[0].timestamp_ns)
			<< ',' << direct_relation(row.cursors.mic_proxy_ns, row.audio_output[1].timestamp_ns)
			<< ',' << direct_relation(row.cursors.program_proxy_ns, row.cursors.mic_proxy_ns)
			<< ',' << (c.governor_enabled ? 1 : 0)
			<< ',' << (c.baseline_valid ? 1 : 0) << ',' << (c.measurement_fresh ? 1 : 0)
			<< ',' << static_cast<unsigned>(c.phase) << ',' << c.relation_ns << ',' << c.baseline_ns
			<< ',' << c.raw_deviation_ns << ',' << c.corrected_deviation_ns << ',' << c.drift_ppm
			<< ',' << c.native_audio_error_ppm << ',' << c.correction_ppm << ',' << c.target_ppm
			<< ',' << c.confidence << ',' << c.drift_samples << ',' << c.discontinuities
			<< ',' << c.quarantined_samples << ',' << c.corrected_blocks << ',' << c.net_frame_adjustment
			<< ',' << (c.ndi_frame_sync ? 1 : 0) << ',' << c.ndi_sync_mode
			<< ',' << (c.ndi_audio_enabled ? 1 : 0) << ',' << c.ndi_behavior << '\n';
	}
	return out.str();
}

size_t DeepTimingRecorder::sample_count() const
{
	std::lock_guard<std::mutex> lock(ring_mutex_);
	return count_;
}

uint64_t DeepTimingRecorder::overwritten_samples() const
{
	std::lock_guard<std::mutex> lock(ring_mutex_);
	return overwritten_;
}

} // namespace mcb
