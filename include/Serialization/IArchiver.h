#pragma once

#include <map>
#include <string>
#include <vector>

namespace gamelib::archive
{
	template <typename T>
	using NVP = std::pair<std::string_view, T&>;
	#define ARCHIVE_NVP(name) ::gamelib::archive::NVP<decltype(name)>{#name, name}

	struct Field
	{
		std::string Name;
	};

	struct ClassModel
	{
		std::string Name;
		std::vector<Field> Fields;
	};

	template <typename CRTP>
	struct IArchiver
	{
		enum class Mode
		{
			Loading,
			Saving,
			Modeling,
			Updating,
		};

		IArchiver(IErrorReporter& reporter, Mode mode) : mReporter(reporter),mMode(mode) {}

		virtual ~IArchiver() = default;

		template <typename T>
		IArchiver& operator&(NVP<T> nvp)
		{
			Archive((CRTP&)*this, nvp.first, nvp.second);
			return *this;
		}

		Mode CurrentMode() const { return mMode; }

	protected:

		IErrorReporter& mReporter;

		Mode mMode = Mode::Loading;

		std::map<std::string, ClassModel> mClasses;
	};

	struct JsonArchiver : IArchiver<JsonArchiver>
	{
		json Root;
		json* CurrentObject;
		std::vector<json*> ObjectStack;
		std::string FieldName;

		JsonArchiver(IErrorReporter& reporter) : IArchiver(reporter, Mode::Saving), Root(json::object_t{}), CurrentObject(&Root) {}
		JsonArchiver(IErrorReporter& reporter, json* target) : IArchiver(reporter, Mode::Saving), CurrentObject(target) {}
		JsonArchiver(IErrorReporter& reporter, json const& from) : IArchiver(reporter, Mode::Loading), CurrentObject(const_cast<json*>(&from)) {}
		JsonArchiver(IErrorReporter& reporter, json&& from) : IArchiver(reporter, Mode::Loading), Root(std::move(from)), CurrentObject(&Root) {}

		template <typename T>
		void Value(std::string_view name, T& val)
		{
			switch (mMode)
			{
			case Mode::Loading:
			{
				auto it = CurrentObject->find(name);
				if (it != CurrentObject->end())
				{
					try
					{
						it->get_to(val);
					}
					catch (std::exception const& e)
					{
						mReporter.ThrowError("Invalid value for field '{}': {}", name, e.what());
					}
				}
				break;
			}
			case Mode::Saving:
				CurrentObject->operator[]((std::string)name) = val;
				break;
			case Mode::Updating:
			case Mode::Modeling:
			default:
				break;
			}
		}
	};
	
	template <typename T>
	void Archive(JsonArchiver& archive, std::string_view name, T& val)
	{
		archive.Value(name, val);
	}
}