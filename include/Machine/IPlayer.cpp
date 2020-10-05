#include "IPlayer.h"
#include "../Includes/Format.h"

namespace gamelib
{
	std::string ContextSpecificIdentity::PrintableString() const
	{
		return Context ? Context->PrintableIdentityString(Identity) : Identity;
	}
	
	ContextSpecificIdentity IPlayer::PrimaryIdentity()
	{
		return { nullptr, fmt::format("#{:016X}", reinterpret_cast<uintptr_t>(this)) };
	}

	std::vector<ContextSpecificIdentity> IPlayer::KnownIdentities() { return { PrimaryIdentity() }; }

	ContextSpecificIdentity IPlayer::IdentityInContext(IIdentityContext const* context)
	{
		auto ids = KnownIdentities();
		for (auto& id : ids)
			if (id.Context == context)
				return id;
		return {};
	}
}
