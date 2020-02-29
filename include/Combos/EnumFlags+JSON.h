#pragma once

#if defined(GAMELIB_ENUM_FLAGS) && defined(GAMELIB_JSON)
namespace enum_flags
{
	template <typename ENUM_TYPE, typename BASE_TYPE>
	inline void to_json(json& j, const enum_flags<ENUM_TYPE, BASE_TYPE>& p) { j = p.bits; }
	template <typename ENUM_TYPE, typename BASE_TYPE>
	inline void from_json(const json& j, enum_flags<ENUM_TYPE, BASE_TYPE>& p) {
		if (j.is_string())
		{
			auto const& str = j.get_ref<std::string const&>();
			baselib::Split(baselib::string_view{ str }, baselib::string_view{ "," }, [&p](auto str, bool last) {
				auto val = magic_enum::enum_cast<ENUM_TYPE>(std::string_view{ baselib::TrimWhitespace(str) });
				if (val.has_value())
					p.set(val.value());
				});
		}
		else
			p.bits = j;
	}
}
#endif