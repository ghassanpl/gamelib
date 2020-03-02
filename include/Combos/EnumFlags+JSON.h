#pragma once

#include <magic_enum.hpp>

/*
namespace ghassanpl
{
	template <typename ENUM_TYPE, typename BASE_TYPE>
	inline void to_json(json& j, const enum_flags<ENUM_TYPE, BASE_TYPE>& p) { j = p.bits; }
	template <typename ENUM_TYPE, typename BASE_TYPE>
	inline void from_json(const json& j, enum_flags<ENUM_TYPE, BASE_TYPE>& p) {
		if (j.is_string())
		{
			auto const& str = j.get_ref<std::string const&>();
			baselib::Split(std::string_view{ str }, std::string_view{ "," }, [&p](auto str, bool last) {
				auto val = magic_enum::enum_cast<ENUM_TYPE>(std::string_view{ baselib::TrimWhitespace(str) });
				if (val.has_value())
					p.set(val.value());
			});
		}
		else
			p.bits = j;
	}
}
*/