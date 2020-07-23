#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <functional>
#include <stdexcept>

namespace gamelib::ioc
{
	struct RegisteredType;

	struct UnregisteredType : std::invalid_argument
	{
		std::type_index TypeRequested;

		explicit UnregisteredType(std::type_index index) : invalid_argument(index.name()), TypeRequested(index) {}
	};

	struct TypeAlreadyExists : std::invalid_argument
	{
		std::type_index TypeRequested;

		explicit TypeAlreadyExists(std::type_index index) : invalid_argument(index.name()), TypeRequested(index) {}
	};

	struct Container : std::enable_shared_from_this<Container>
	{
		/// Use
		
		template <typename INTERFACE_TYPE>
		[[nodiscard]] std::shared_ptr<INTERFACE_TYPE> GetInstance();

		template <typename... INTERFACE_TYPES>
		void GetInstances(std::tuple<std::shared_ptr<INTERFACE_TYPES>...>& instances)
		{
			((std::get<INTERFACE_TYPES>(instances) = this->GetInstance<INTERFACE_TYPES>()), ...);
		}

		template <typename... INTERFACE_TYPES>
		auto GetInstances() -> std::tuple<std::shared_ptr<INTERFACE_TYPES>...>
		{
			std::tuple<std::shared_ptr<INTERFACE_TYPES>...> result;
			this->GetInstances(result);
			return result;
		}

		template <typename... INTERFACE_TYPES>
		operator std::tuple<std::shared_ptr<INTERFACE_TYPES>...>()
		{
			return this->GetInstances<INTERFACE_TYPES...>();
		}

		/// Build 
		template <typename INTERFACE_TYPE, typename CONCRETE_TYPE>
		RegisteredType& RegisterConcreteType()
		{
			static_assert(!std::is_abstract_v<CONCRETE_TYPE>, "concrete type cannot be abstract");
			static_assert(std::is_base_of_v<INTERFACE_TYPE, CONCRETE_TYPE>, "types must be related");

			return Register<INTERFACE_TYPE>(std::type_identity<INTERFACE_TYPE>{}, std::type_identity<CONCRETE_TYPE>{});
		}

		template <typename INTERFACE_TYPE, typename CONCRETE_TYPE>
		RegisteredType& RegisterInstance(std::shared_ptr<CONCRETE_TYPE> const& instance)
		{
			static_assert(!std::is_abstract_v<CONCRETE_TYPE>, "concrete type cannot be abstract");
			static_assert(std::is_base_of_v<INTERFACE_TYPE, CONCRETE_TYPE>, "types must be related");

			if (!instance)
				throw std::invalid_argument{ "instance" };

			return Register<INTERFACE_TYPE>(instance);
		}

		template <typename INTERFACE_TYPE>
		RegisteredType& RegisterFactory(std::function<std::shared_ptr<INTERFACE_TYPE>()> const& factory)
		{
			if (!factory)
				throw std::invalid_argument{ "factory" };

			return Register<INTERFACE_TYPE>(factory);
		}

		template <typename INTERFACE_TYPE, typename CONCRETE_TYPE>
		RegisteredType& RegisterInstance(CONCRETE_TYPE* instance)
		{
			return this->RegisterInstance<INTERFACE_TYPE, CONCRETE_TYPE>(std::shared_ptr<CONCRETE_TYPE>{std::shared_ptr<CONCRETE_TYPE>{}, instance});
		}

	protected:

		template <typename INTERFACE_TYPE, typename... ARGS>
		RegisteredType& Register(ARGS&&... args)
		{
			const std::type_index type = typeid(INTERFACE_TYPE);
			auto result = mRegisteredTypes.try_emplace(type, std::forward<ARGS>(args)...);

			if (!result.second) /// did not emplace, exists
				throw TypeAlreadyExists{ type };

			return result.first->second;
		}

		template <typename INTERFACE_TYPE>
		RegisteredType& GetRegisteredTypeFor()
		{
			if (auto it = mRegisteredTypes.find(typeid(INTERFACE_TYPE)); it != mRegisteredTypes.end())
				return it->second;

			if (!mParentContainer.expired())
				return mParentContainer.lock()->GetRegisteredTypeFor<INTERFACE_TYPE>();

			throw UnregisteredType{ typeid(INTERFACE_TYPE) };
		}

		std::weak_ptr<Container> mParentContainer;
		std::unordered_map<std::type_index, RegisteredType> mRegisteredTypes;
	};

	struct RegisteredType
	{
		template <typename INTERFACE_TYPE, typename CONCRETE_TYPE>
		RegisteredType(std::type_identity<INTERFACE_TYPE>, std::type_identity<CONCRETE_TYPE>)
			: mInterfaceType(typeid(INTERFACE_TYPE))
			, mFactoryFunction([]() { return std::make_shared<CONCRETE_TYPE>(); })
		{

		}

		template <typename INTERFACE_TYPE>
		RegisteredType(std::shared_ptr<INTERFACE_TYPE> instance)
			: mInterfaceType(typeid(INTERFACE_TYPE))
			, mInstance(std::move(instance))
		{
			if (!mInstance)
				throw std::invalid_argument{ "instance" };
		}

		template <typename INTERFACE_TYPE>
		RegisteredType(std::function<std::shared_ptr<INTERFACE_TYPE>()> factory)
			: mInterfaceType(typeid(INTERFACE_TYPE))
			, mFactoryFunction(std::move(factory))
		{
			if (!mFactoryFunction)
				throw std::invalid_argument{ "factory" };
		}

		RegisteredType& Named(std::string_view name)
		{
			mName = name;
			return *this;
		}

		RegisteredType& SingleInstance()
		{
			if (!mInstance && mFactoryFunction)
				mInstance = mFactoryFunction();
			return *this;
		}

		template <typename INTERFACE_TYPE>
		std::shared_ptr<INTERFACE_TYPE> ResolveInstance()
		{
			if (mInstance)
				return std::static_pointer_cast<INTERFACE_TYPE>(mInstance);
			else if (mFactoryFunction)
				return std::static_pointer_cast<INTERFACE_TYPE>(mFactoryFunction());
			else if constexpr (!std::is_abstract_v<INTERFACE_TYPE>)
				return std::make_shared<INTERFACE_TYPE>();

			throw std::exception{ "registered type is invalid, no instance or factory" };
		}

	private:

		std::type_index mInterfaceType = typeid(void);
		std::shared_ptr<void> mInstance;
		std::function<std::shared_ptr<void>()> mFactoryFunction;
		std::string mName;

	};

	template<typename INTERFACE_TYPE>
	[[nodiscard]] inline std::shared_ptr<INTERFACE_TYPE> Container::GetInstance()
	{
		return this->GetRegisteredTypeFor<INTERFACE_TYPE>().ResolveInstance<INTERFACE_TYPE>();
	}

}