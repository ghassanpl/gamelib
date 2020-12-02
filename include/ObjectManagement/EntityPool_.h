#pragma once

#include "../Common.h"
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <set>

/// Shamelessly stolen from: https://github.com/tesselode/nata/
namespace gamelib
{
	template <typename T>
	auto to_address(T& ptr)
	{
		if constexpr (std::is_pointer_v<T> || requires { typename T::element_type; })
			return std::to_address(ptr);
		else
			return std::addressof(ptr);
	}

	template <typename ENTITY_TYPE>
	struct EntityEventReceiver
	{
		std::function<void(ENTITY_TYPE&)> Added;
		std::function<void(ENTITY_TYPE&)> Removed;
		std::function<void(std::string_view, ENTITY_TYPE&)> AddedToGroup;
		std::function<void(std::string_view, ENTITY_TYPE&)> RemovedFromGroup;
	};

	template <typename ENTITY_TYPE>
	struct EntityGroupOptions
	{
		std::function<bool(ENTITY_TYPE const&)> Filter;
		std::function<bool(ENTITY_TYPE const&, ENTITY_TYPE const&)> Sort;
	};

	template <typename ENTITY_TYPE>
	struct EntityPoolOptions
	{
		std::map<std::string, EntityGroupOptions<ENTITY_TYPE>> Groups;
		std::vector<SystemDefinition<ENTITY_TYPE>> Systems;
	};

	template <typename ENTITY_TYPE>
	struct EntityPool : EntityEventReceiver<ENTITY_TYPE>
	{
		using entity_ptr = decltype(to_address(*(ENTITY_TYPE*)nullptr));

		void Flush()
		{
			auto to_flush = std::move(mQueue);
			for (auto& entity : to_flush)
			{
				auto ptr = to_address(entity);
				if (!Contains(ptr))
				{
					mEntities.push_back(std::move(entity));
					ptr = to_address(mEntities.back());
				}
				Emit(&EntityEventReceiver<ENTITY_TYPE>::Added, *ptr);

				for (auto& [name, group] : mGroups)
				{
					if (!group.Options.Filter && group.Options.Filter(*ptr))
					{
						if (!Contains(group.Entities, ptr))
						{
							group.Entities.push_back(ptr);
							Emit(&EntityEventReceiver<ENTITY_TYPE>::AddedToGroup, name, *ptr);
						}
					}
					else if (Contains(group.Entities, ptr))
					{
						Erase(group.Entities, ptr);
						Emit(&EntityEventReceiver<ENTITY_TYPE>::RemovedFromGroup, name, *ptr);
					}
				}
			}
		}

		template <typename FUNC>
		void RemoveIf(FUNC&& predicate)
		{
			for (auto& [name, group] : mGroups)
			{
				std::erase_if(group.Entities, [&](entity_ptr entity) {
					if (predicate(entity))
					{
						Emit(&EntityEventReceiver<ENTITY_TYPE>::RemovedFromGroup, name, entity);
						return true;
					}
					return false;
				});
			}

			std::erase_if(mEntities, [&](ENTITY_TYPE const& entity) {
				if (predicate(entity))
				{
					Emit(&EntityEventReceiver<ENTITY_TYPE>::Removed, entity);
					return true;
				}
				return false;
			});
		}

		bool Contains(entity_ptr ptr) const
		{
			return Contains(mEntities, ptr);
		}

		bool Contains(ENTITY_TYPE const& entity) const
		{
			return Contains(mEntities, entity);
		}

	private:

		struct EntityGroup
		{
			GroupOptions Options;
			std::vector<entity_ptr> Entities;
		};

		std::vector<ENTITY_TYPE> mQueue;
		std::vector<ENTITY_TYPE> mEntities;

		template <typename FIELD_PTR, typename... ARGS>
		void Emit(FIELD_PTR field, ARGS&&... args)
		{
			auto& function = (this->*field);
			if (function)
				function(args...);
		}

		template <typename T>
		static bool Contains(T const& span, entity_ptr ptr)
		{
			return std::find_if(begin(span), end(span), [ptr](auto&& ref) {
				return to_address(ref) == ptr;
			}) != end(span);
		}

		template <typename T>
		static bool Erase(T& span, entity_ptr ptr)
		{
			auto it = std::find_if(begin(span), end(span), [ptr](auto&& ref) {
				return to_address(ref) == ptr;
			});;
			if (it != end(span))
			{
				span.erase(it);
				return true;
			}
			return false;
		}

		std::map<std::string, EntityGroup, std::less<>> mGroups;
	};

}