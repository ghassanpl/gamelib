#pragma once

#include "../Common.h"
#include <string>

namespace gamelib
{
	struct IPlayer;

	struct IIdentityContext
	{
		virtual bool Identifies(std::string_view identity) const = 0;
		virtual std::string PrintableIdentityString(std::string_view identity) const = 0;
	};

	template <typename PLAYER_DATA>
	struct SimplePlayerIdentity : IIdentityContext
	{
		std::span<PLAYER_DATA> Players;
		virtual bool Identifies(std::string_view identity) const override { return identity.size() && Players.size() > identity[0]; }
		/// TODO: Make these translatable
		virtual std::string PrintableIdentityString(std::string_view identity) const override { return Identifies(identity) ? ("Player " + std::to_string(identity[0] + 1)) : "Unknown Player"; }
	};

	/// This is a context that represents the Users accounts local to the machine (e.g. Windows users)
	struct IMachineUserIdentityContext : IIdentityContext
	{
	};

}