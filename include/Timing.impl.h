#include "Timing.h"
#pragma once

namespace gamelib
{
	inline void TimingSystem::Reset()
	{
		mFixedFrames = 0;
		mFixedAccumulator = 0;

		mGameStartTime = -1.0;
		mPausedTime = 0;
		mCurrentTime = 0;
		mTimeSinceLastFrame = 0;
		mTotalGameTime = 0;
	}

	inline void TimingSystem::Pause(bool pause)
	{
		if (mPaused = pause)
			mPausedTime = mCurrentTime;
	}

	inline auto TimingSystem::Update() -> seconds_t
	{
		auto current_time = CurrentTime();
		mTimeSinceLastFrame = current_time - mCurrentTime;
		mCurrentTime = current_time;

		if (mGameStartTime < 0)
			mGameStartTime = current_time;

		if (!mPaused)
			mTotalGameTime += mTimeSinceLastFrame * TimerMultiplier;

		mFixedAccumulator += mTimeSinceLastFrame * TimerMultiplier;
		mFixedFrames = int(mFixedAccumulator / FixedTimestep);
		mFixedAccumulator -= mFixedFrames * FixedTimestep;

		return TimeSinceLastFrame();
	}

	inline auto TimingSystem::RealFrameStartTime() -> seconds_t { return mCurrentTime; }

	inline auto TimingSystem::RealTimeSinceLastFrame() -> seconds_t { return mTimeSinceLastFrame; }

	inline auto TimingSystem::RealTotalGameTime() -> seconds_t { return CurrentTime() - mGameStartTime; }

	inline auto TimingSystem::TimeSinceLastFrame() -> seconds_t { return mPaused ? 0.0 : mTimeSinceLastFrame * TimerMultiplier; }

	inline auto TimingSystem::FrameStartTime() -> seconds_t { return mPaused ? mPausedTime : mCurrentTime; }

	inline auto TimingSystem::TotalGameTime() -> seconds_t { return mTotalGameTime; }

	inline auto TimingSystem::FixedFrames() -> int { return mPaused ? 0 : mFixedFrames; }

	inline auto TimingSystem::FixedFrameTime() -> seconds_t { return FixedTimestep * FixedFrames(); }

	inline void TimingSystem::SetFixedTimestepInFPS(herz_t fps) { FixedTimestep = 1.0 / fps; }

	inline auto TimingSystem::FixedAccumulator() -> seconds_t { return mPaused ? 0 : mFixedAccumulator; }

	inline auto TimingSystem::FixedInterpolation() -> double { return FixedAccumulator() / FixedTimestep; }

	inline auto TimingSystem::FPS() -> herz_t { return 1.0 / mTimeSinceLastFrame; }

	inline void TimingSystem::SetFlag(std::string_view name) { mFlags[std::string{ name }] = CurrentTime(); }

	inline void TimingSystem::RemoveFlag(std::string_view name) { mFlags.erase(std::string{ name }); }

	inline void TimingSystem::ClearAllFlags() { mFlags.clear(); }

	inline const auto& TimingSystem::Flags() const { return mFlags; }

	inline seconds_t TimingSystem::TimeSinceFlag(std::string_view name, bool use_frame_start_time)
	{
		if (auto it = mFlags.find(name); it != mFlags.end())
			return (use_frame_start_time ? mCurrentTime : CurrentTime()) - it->second;
		return seconds_t{ -1.0 };
	}

	inline int TimingSystem::FlagRecurred(std::string_view name, seconds_t recurrence_time, bool use_frame_start_time)
	{
		if (auto it = mFlags.find(name); it != mFlags.end())
		{
			const auto current = (use_frame_start_time ? mCurrentTime : CurrentTime());
			const auto time_since_flag = current - it->second;
			const auto result = int(time_since_flag / recurrence_time);
			it->second += result * recurrence_time; /// replant the flag as close to current time as possible without messing up the future recurrence count
			return result;
		}
		return 0;
	}

	inline bool TimingSystem::FlagElapsedRestart(std::string_view name, seconds_t elapsed_time, bool use_frame_start_time)
	{
		if (auto it = mFlags.find(name); it != mFlags.end())
		{
			const auto current = (use_frame_start_time ? mCurrentTime : CurrentTime());
			const auto time_since_flag = current - it->second;
			const auto result = time_since_flag > elapsed_time;
			it->second = current;
			return result;
		}
		return false;
	}
}