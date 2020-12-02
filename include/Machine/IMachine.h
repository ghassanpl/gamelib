#pragma once

#include "IHardwareDevice.h"
#include "IPlayer.h"
#include <functional>

namespace gamelib
{
	using file_path = std::filesystem::path;

	struct IMachine : IHardwareDevice
	{
		/// volumes / filesystems
		/// input devices
		/// displays
		/// audio devices
		/// network devices
		/// power
		/// 
		/// os stuff
		/// os identification (do we want it? shouldn't we abstract it?)
		/// clipboard
		/// running process
		/// permissions and users
		/// user preferences (mouse click time, mouse speed, UI colors, notification settings, etc)
		/// languages and locale (preffered language, currency/date/number formatting)
		/// execution of other processes/activities
		/// available social apis
		/// location? or should we leave that to input?
		///
		/// register application for recovery on crash (e.g. to finish saving a game)

		/// Returns true if the user canceled the recovery process; the application should then terminate.
		using CrashRecoveryProgressCallback = std::function<bool(progress_t recovery_progress)>;

		/// Return true from this callback if the recovery was a success
		using CrashRecoveryCallback = std::function<bool(seconds_t min_progress_callback_call_interval, CrashRecoveryProgressCallback progress_callback)>;

		virtual void RegisterCrashRecoveryCallback(CrashRecoveryCallback callback) = 0; /// RegisterApplicationRecoveryCallback
		virtual void UnregisterCrashRecoveryCallback() = 0;

		enum class SpecialPath
		{
			Resources,
			Temporary,
			Home,
			Documents,
			UserData,
			Settings,
			Executable,
			System,
		};

		virtual auto GetSpecialPath(SpecialPath path, const char* app_name = nullptr, const char* org_name = nullptr) -> file_path = 0; /// al_get_standard_path

		/// TODO: List available filesystems
	};
}