#pragma once
#include "Common.h"
#include <string_view>
#include <map>
#include <functional>

namespace gamelib
{
	class TimingSystem
	{
	public:

		TimingSystem(std::function<seconds_t()> time_getter) : CurrentTime(std::move(time_getter)) {}

		std::function<seconds_t()> CurrentTime;

		void Reset();

		double TimerMultiplier = 1.0;

		void Pause(bool pause = true);
		bool Paused() const { return mPaused; }

		/// Function: Update
		/// Updates the timing system and returns `TimeSinceLastFrame()`
		auto Update() -> seconds_t;

		///# Getters #///

		auto RealFrameStartTime() -> seconds_t;
		auto RealTimeSinceLastFrame() -> seconds_t;
		auto RealTotalGameTime() -> seconds_t;

		auto TimeSinceLastFrame() -> seconds_t;
		auto FrameStartTime() -> seconds_t;
		auto TotalGameTime() -> seconds_t;

		///# Fixed Frames #///

		/// The fixed-frame framerate in seconds-per-frame
		seconds_t FixedTimestep = 1.0 / 60.0;

		/// Gets the number of fixed-rate ticks elapsed since last frame. This value is set at the beginning of the current frame.
		auto FixedFrames() -> int;

		/// Gets the fixed-rate time elapsed since last frame (basically FixedFrames * FixedTimestep). This value is set at the beginning of the current frame.
		auto FixedFrameTime() -> seconds_t;

		/// Sets the fixed-frame framerate in frames-per-second.
		void SetFixedTimestepInFPS(herz_t fps);

		/// Gets the time that elapsed between the start of the current fixed-rate frame and the start of the current variadic-rate frame.
		auto FixedAccumulator() -> seconds_t;

		/// As above, but returns a value between 0 and 1, where 0 is the beginning of the fixed frame,
		/// and 1 is the end of the fixed frame.
		auto FixedInterpolation() -> double;

		///# Flags #///

		void SetFlag(std::string_view name);
		void RemoveFlag(std::string_view name);
		void ClearAllFlags();
		auto Flags() const -> const auto&;

		/// Returns the time that elapsed from setting the flag to either the start of the frame (if `use_frame_start_time` is true) or the current moment
		auto TimeSinceFlag(std::string_view name, bool use_frame_start_time = true) -> seconds_t;

		/// Returns how many times the flag has overflowed the `recurrence_time` and resets the flag to the latest overflow time
		/// See: TimeSinceFlag
		auto FlagRecurred(std::string_view name, seconds_t recurrence_time, bool use_frame_start_time = true) -> int;

		/// Returns whether the flag has overflowed the `elapsed_time` and resets the flag to the current moment (or start of frame, see TimeSinceFlag)
		/// See: TimeSinceFlag
		auto FlagElapsedRestart(std::string_view name, seconds_t elapsed_time, bool use_frame_start_time = true) -> bool;

		///# Misc #///

		/// Returns the number of "frames" per second calculated based on the last few frame timings
		auto FPS() -> herz_t;

	private:

		std::map<std::string, seconds_t, std::less<>> mFlags;

		bool mPaused = false;

		int mFixedFrames = 0;
		seconds_t mFixedAccumulator = 0;

		seconds_t mGameStartTime = -1.0;
		seconds_t mPausedTime = 0;
		seconds_t mCurrentTime = 0;
		seconds_t mTimeSinceLastFrame = 0;
		seconds_t mTotalGameTime = 0;
	};

	/// Implementation
}

#include "Timing.impl.h"