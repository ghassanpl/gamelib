#pragma once

#include "IIdentity.h"
#include "../Includes/EnumFlags.h"
#include <vector>

namespace gamelib
{
	/// Refers to a player known locally; that player's ID should never change.
	/// This can be used as an opaque value on your own, or you can use a `PlayerManager` to
	/// map them to `IPlayer*`s.
	struct PlayerID
	{
		uintptr_t Value = {};
		constexpr explicit operator bool() const noexcept { return Value != 0; }
		constexpr std::weak_ordering operator <=>(PlayerID const&) const noexcept = default;
	};
	static constexpr inline PlayerID InvalidPlayerID = {};

	struct ContextSpecificIdentity
	{
		IIdentityContext* Context = nullptr;
		std::string Identity;

		std::string PrintableString() const;
	};

	enum class PlayerFlags
	{
		Admin,
		Guest, /// Means that the primary identity is synthesized/temporary, in a Guest identity context
		Adult,
		Remote,
		LoggedIn,
		Playing,
	};

	struct IPlayer
	{
		virtual bool IsAdmin() const { return Flags().is_set(PlayerFlags::Admin); }
		virtual bool IsGuest() const { return Flags().is_set(PlayerFlags::Guest); }
		virtual bool IsAdult() const { return Flags().is_set(PlayerFlags::Adult); }
		virtual bool IsRemote() const { return Flags().is_set(PlayerFlags::Remote); }
		virtual bool IsLoggedIn() const { return Flags().is_set(PlayerFlags::LoggedIn); }
		virtual bool IsPlaying() const { return Flags().is_set(PlayerFlags::Playing); }

		virtual enum_flags<PlayerFlags> Flags() const { return { PlayerFlags::Adult, PlayerFlags::LoggedIn, PlayerFlags::Playing }; }

		virtual ContextSpecificIdentity PrimaryIdentity();
		virtual std::vector<ContextSpecificIdentity> KnownIdentities();
		virtual ContextSpecificIdentity IdentityInContext(IIdentityContext const* context);
	};

	struct PlayerManager
	{
		PlayerID NewOrExistingPlayerForIdentity(ContextSpecificIdentity const& identity);
		PlayerID ExistingPlayerByIdentity(ContextSpecificIdentity const& identity);
		std::vector<PlayerID> PlayersByFlags(enum_flags<PlayerFlags> flags);
		std::vector<PlayerID> PlayersByIdentityContext(IIdentityContext const* context);

		/// And lots of other things
	};

}