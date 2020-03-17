#pragma once

#include "Includes/Format.h"
#include <vector>
#include <map>
#include <functional>
#include <mutex>

namespace gamelib
{

	enum class ReportType
	{
		Info,
		Warning,
		Error,
		AssumptionFailure
	};

	struct IErrorReporter;

	struct Reporter
	{
		IErrorReporter const& ErrorReporter;
		ReportType Type = ReportType::Error;
		std::vector<std::string> MessageLines;
		std::vector<std::pair<std::string, std::string>> AdditionalInfoLines;

		Reporter(IErrorReporter const& errep, ReportType type);
		
		template <typename... ARGS>
		Reporter& MessageLine(std::string_view fmt, ARGS&&... args)
		{
			MessageLines.push_back(fmt::format(fmt, std::forward<ARGS>(args)...));
			return *this;
		}
		template <typename... ARGS>
		Reporter& AdditionalInfo(std::string_view name, std::string_view fmt, ARGS&&... args)
		{
			AdditionalInfoLines.push_back({ (std::string)name, fmt::format(fmt, std::forward<ARGS>(args)...) });
			return *this;
		}

		template <typename ARG>
		Reporter& Value(std::string_view name, ARG&& val)
		{
			AdditionalInfo(name, "{}", std::forward<ARG>(val));
			return *this;
		}

		void Perform();
	};

	struct IErrorReporter
	{
		virtual ~IErrorReporter() = default;

		template <typename... ARGS>
		Reporter NewError(std::string_view fmt, ARGS&&... args) const
		{
			Reporter result{ *this, ReportType::Error };
			result.MessageLine(fmt, std::forward<ARGS>(args)...);
			return result;
		}

		template <typename... ARGS>
		Reporter NewWarning(std::string_view fmt, ARGS&&... args) const
		{
			Reporter result{ *this, ReportType::Warning };
			result.MessageLine(fmt, std::forward<ARGS>(args)...);
			return result;
		}

		template <typename... ARGS>
		void Error(std::string_view fmt, ARGS&&... args) const
		{
			NewError(fmt, std::forward<ARGS>(args)...).Perform();
		}

		template <typename... ARGS>
		void Warning(std::string_view fmt, ARGS&&... args) const
		{
			NewWarning(fmt, std::forward<ARGS>(args)...).Perform();
		}

		template <typename... ARGS>
		void ThrowError(std::string_view fmt, ARGS&&... args) const
		{
			throw NewError(fmt, std::forward<ARGS>(args)...);
		}

		virtual void PerformReport(Reporter const& holder) const;

	protected:

		std::mutex mMutex;
		/*
		struct Report
		{
			ReportType Type = ReportType::Error;
			uintptr_t UniqueID = 0;
			std::vector<std::string> MessageLines;
			std::vector<std::pair<std::string, std::string>> AdditionalInfoLines;

			Report(ReportType type);

			template <typename... ARGS>
			void FormatMessageLine(std::string_view fmt, ARGS&&... args)
			{
				MessageLines.push_back(fmt::format(fmt, std::forward<ARGS>(args)...));
			}
			void AddAdditionalInfo(ghassanpl::detail::source_location source_location);
			void AddAdditionalInfo(std::pair<std::string, std::string> pair);
			template <typename... ARGS>
			void FormatAdditionalInfo(std::string_view name, std::string_view fmt, ARGS&&... args)
			{
				AddAdditionalInfo({ (std::string)name, fmt::format(fmt, std::forward<ARGS>(args)...) });
			}
		};

		template <typename... REPORT_DATUMS>
		void Report(ReportType type, std::string message, REPORT_DATUMS&&... datums) const
		{
			Report holder{ type };

			holder.MessageLines.push_back(std::move(message));
			(holder.AddAdditionalInfo(std::forward<REPORT_DATUMS>(datums)), ...);
			for (auto& [name, getter] : mDefaultInfoGetters)
				getter(holder);
			PerformReport(holder);
		}

		virtual void PerformReport(Report const& holder) const;

		void AddDefaultInfoGetter(std::string_view name, std::function<void(Report&)> getter);

	protected:

		std::mutex mMutex;
		std::map<std::string, std::function<void(Report&)>, std::less<>> mDefaultInfoGetters;
		*/
	};

}