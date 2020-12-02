#pragma once
#include "Timing.h"
#include "Debugger.h"
#include <imgui.h>

namespace gamelib
{
	void TimingSystem::Reset()
	{
		mFixedAccumulator = 0;

		mGameStartTime = -1.0;
		mPausedTime = 0;
		mCurrentTime = 0;
		mTotalGameTime = 0;

		mFixedFrames.reset();
		mTimeSinceLastFrame.reset();
	}

	void TimingSystem::Pause(bool pause)
	{
		if ((mPaused = pause))
			mPausedTime = mCurrentTime;
	}

	auto TimingSystem::Update() -> seconds_t
	{
		auto current_time = CurrentTime();
		mTimeSinceLastFrame = std::min(current_time - mCurrentTime, mMaxTimeSinceLastFrame);

		mCurrentTime = current_time;

		if (mGameStartTime < 0)
			mGameStartTime = current_time;

		if (!mPaused)
			mTotalGameTime += mTimeSinceLastFrame * TimerMultiplier;

		mFixedAccumulator += mTimeSinceLastFrame * TimerMultiplier;
		mFixedFrames = int(mFixedAccumulator / FixedTimestep);
		mFixedAccumulator -= mFixedFrames * FixedTimestep;

		if (mRecordLastFrames)
		{
			mLastFrames.push_back(FrameData{
				.CurrentTime = current_time,
				.TotalGameTime = mTotalGameTime,
				.TimeSinceLastFrame = mTimeSinceLastFrame,
				.FixedFrames = mFixedFrames,
				.FixedAccumulator = mFixedAccumulator
			});
			if (mLastFrames.size() > mRecordLastFrames)
				mLastFrames.erase(mLastFrames.begin());
		}

		return TimeSinceLastFrame();
	}

	auto TimingSystem::RealFrameStartTime() -> seconds_t { return mCurrentTime; }

	auto TimingSystem::RealTimeSinceLastFrame() -> seconds_t { return mTimeSinceLastFrame; }

	auto TimingSystem::RealTotalGameTime() -> seconds_t { return CurrentTime() - mGameStartTime; }

	auto TimingSystem::TimeSinceLastFrame() -> seconds_t { return mPaused ? 0.0 : mTimeSinceLastFrame * TimerMultiplier; }
	
	auto TimingSystem::MaxTimeSinceLastFrame() const -> seconds_t { return mMaxTimeSinceLastFrame; }

	void TimingSystem::SetMaxTimeSinceLastFrame(seconds_t max_time) { mMaxTimeSinceLastFrame = max_time; }

	auto TimingSystem::FrameStartTime() -> seconds_t { return mPaused ? mPausedTime : mCurrentTime; }

	auto TimingSystem::TotalGameTime() -> seconds_t { return mTotalGameTime; }

	auto TimingSystem::FixedFrames() -> int { return mPaused ? 0 : mFixedFrames; }

	auto TimingSystem::FixedFrameTime() -> seconds_t { return FixedTimestep * FixedFrames(); }

	void TimingSystem::SetFixedTimestepInFPS(herz_t fps) { FixedTimestep = 1.0 / fps; }

	auto TimingSystem::FixedAccumulator() -> seconds_t { return mPaused ? 0 : mFixedAccumulator; }

	auto TimingSystem::FixedInterpolation() -> double { return FixedAccumulator() / FixedTimestep; }

	auto TimingSystem::FPS() -> herz_t { return 1.0 / mTimeSinceLastFrame; }

	void TimingSystem::SetFlag(std::string_view name) { mFlags[std::string{ name }] = CurrentTime(); }

	void TimingSystem::RemoveFlag(std::string_view name) { mFlags.erase(std::string{ name }); }

	void TimingSystem::ClearAllFlags() { mFlags.clear(); }

	const auto& TimingSystem::Flags() const { return mFlags; }

	seconds_t TimingSystem::TimeSinceFlag(std::string_view name, bool use_frame_start_time)
	{
		if (auto it = mFlags.find(name); it != mFlags.end())
			return (use_frame_start_time ? mCurrentTime : CurrentTime()) - it->second;
		return seconds_t{ -1.0 };
	}

	int TimingSystem::FlagRecurred(std::string_view name, seconds_t recurrence_time, bool use_frame_start_time)
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

	double TimingSystem::FlagDelta(std::string_view name, seconds_t recurrence_time, bool use_frame_start_time)
	{
		if (auto it = mFlags.find(name); it != mFlags.end())
		{
			const auto current = (use_frame_start_time ? mCurrentTime : CurrentTime());
			const auto time_since_flag = current - it->second;
			const auto result = time_since_flag / recurrence_time;
			it->second += int(result) * recurrence_time; /// replant the flag as close to current time as possible without messing up the future recurrence count
			return std::fmod(result, 1.0);
		}
		return 0;
	}

	bool TimingSystem::FlagElapsedRestart(std::string_view name, seconds_t elapsed_time, bool use_frame_start_time)
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

	void TimingSystem::Debug(IDebugger& debugger)
	{
		/// TODO: Flags
		if (ImGui::Button("Reset KPIs"))
		{
			mFixedFrames.reset();
			mTimeSinceLastFrame.reset();
		}

		debugger.Value("Paused", mPaused);
		debugger.Value("Paused Time", mPausedTime);

		debugger.Value("Game Start Time", mGameStartTime);
		debugger.Value("Total Game Time", mTotalGameTime);

		debugger.Value("Current Time", mCurrentTime);
		debugger.Value("Time Since Last Frame", mTimeSinceLastFrame);
		debugger.Value("Max Time Since Last Frame", mMaxTimeSinceLastFrame);

		debugger.Value("Fixed Frames", mFixedFrames);
		debugger.Value("Fixed Accumulator", mFixedAccumulator);

		debugger.Value("Record Last Frames", mRecordLastFrames);

		ImGui::PlotLines("Deltas", [](void* data, int indx) {
			auto timing = (TimingSystem*)data;
			return float(timing->mLastFrames[indx].TimeSinceLastFrame);
		}, this, (int)mLastFrames.size());
		ImGui::PlotLines("Fixed Frames", [](void* data, int indx) {
			auto timing = (TimingSystem*)data;
			return float(timing->mLastFrames[indx].FixedFrames);
		}, this, (int)mLastFrames.size());

	}
}