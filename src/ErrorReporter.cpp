#include "../include/ErrorReporter.h"
#include <chrono>
#include <thread>
#include <array>
#include <gsl/span>
#include <iostream>
#include "../include/Includes/Assuming.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <shellapi.h>

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#pragma comment(lib, "Comctl32.lib")
#include <Commctrl.h>

enum class MessageBoxIcon
{
	Warning = -1,
	Error = -2,
	Information = -3,
	Security = -4
};

struct MessageBoxResult
{
	size_t ClickedButton;
	bool CheckboxValue;
};

enum class MessageBoxEvent
{
	DialogCreated = 0,
	ButtonClicked = 2, /// param1 = (int)button id
	LinkClicked = 3, /// param1 = (const wchar_t*)href
	DialogDestroyed = 5,
	CheckboxClicked = 8, /// param2 = (bool)checked
	HelpRequested = 9,
};

static MessageBoxResult ShowMessageBox(
	std::wstring_view title,
	MessageBoxIcon icon,
	std::wstring_view description,
	gsl::span<std::wstring_view const> buttons,
	size_t default_button = 0,
	std::wstring_view long_description = {},
	std::wstring_view checkbox_text = {},
	std::wstring_view additional_info = {},
	std::function<bool(MessageBoxEvent, uintptr_t, uintptr_t)> callback = {}
);

MessageBoxResult ShowMessageBox(
	std::wstring_view title,
	MessageBoxIcon icon,
	std::wstring_view description,
	gsl::span<std::wstring_view const> buttons,
	size_t default_button,
	std::wstring_view long_description,
	std::wstring_view checkbox_text,
	std::wstring_view additional_info,
	std::function<bool(MessageBoxEvent, uintptr_t, uintptr_t)> callback)
{
	/// Set the common parameters
	TASKDIALOGCONFIG task_config = {};
	task_config.cbSize = sizeof(TASKDIALOGCONFIG);
	task_config.hwndParent = GetActiveWindow();
	task_config.hInstance = GetModuleHandle(nullptr);
	task_config.dwFlags =
		TDF_ENABLE_HYPERLINKS
		| TDF_USE_COMMAND_LINKS
		//| TDF_EXPANDED_BY_DEFAULT
		| TDF_SIZE_TO_CONTENT;
	task_config.dwCommonButtons = 0;
	task_config.hMainIcon = nullptr;

	/// No radiobuttons, no other platforms use them
	task_config.cRadioButtons = 0;
	task_config.pRadioButtons = nullptr;
	task_config.nDefaultRadioButton = 0;

	task_config.pszCollapsedControlText = nullptr;
	task_config.hFooterIcon = nullptr;
	task_config.pszFooter = nullptr;
	task_config.pszFooterIcon = nullptr;
	task_config.cxWidth = 0;

	/// Titles and descriptions
	auto title_u16 = (std::wstring)title;
	auto description_u16 = (std::wstring)description;
	auto long_description_u16 = (std::wstring)long_description;
	auto checkbox_text_u16 = (std::wstring)checkbox_text;
	auto additional_info_u16 = (std::wstring)additional_info;

	task_config.pszWindowTitle = title_u16.c_str();
	//AssumingBetween(int(icon), int(MessageBoxIcon::Warning), int(MessageBoxIcon::Security));
	task_config.pszMainIcon = MAKEINTRESOURCEW(int(icon));
	task_config.pszMainInstruction = description_u16.c_str();
	if (!long_description.empty())
		task_config.pszContent = long_description_u16.c_str();

	if (!checkbox_text.empty())
		task_config.pszVerificationText = checkbox_text_u16.c_str();
	if (!additional_info.empty())
	{
		task_config.pszExpandedControlText = L"Additional Information";
		task_config.pszExpandedInformation = additional_info_u16.c_str();
	}

	/// Callback
	if (callback)
	{
		task_config.pfCallback = [](HWND hwnd, UINT uNotification, WPARAM wParam, LPARAM lParam, LONG_PTR dwRefData) {
			auto ptr = (decltype(callback)*)dwRefData;
			return ptr->operator()((MessageBoxEvent)uNotification, wParam, lParam) ? S_OK : S_FALSE;
		};
		task_config.lpCallbackData = (LONG_PTR)&callback;
	}

	/// Buttons
	//Assuming(!buttons.empty());
	std::vector<TASKDIALOG_BUTTON> button_vector;
	std::vector<std::wstring> button_texts;
	std::transform(buttons.begin(), buttons.end(), std::back_inserter(button_texts), [](auto v) {return (std::wstring)v; });
	int id = 0;
	for (auto& button : button_texts)
		button_vector.push_back({ id++, button.c_str() });

	task_config.cButtons = (UINT)button_vector.size();
	task_config.pButtons = button_vector.data();
	task_config.nDefaultButton = (int)default_button;

	int clicked_id = 0;
	BOOL checkbox_value = false;
	InitCommonControls();
	auto result = TaskDialogIndirect(&task_config, &clicked_id, nullptr, &checkbox_value);
	if (result != S_OK)
	{
		fmt::print(std::cerr, "Showing message box failed with error: {}\n", result);
		return {};
	}
	//AssumingEqual(result, S_OK);
	return{ size_t(clicked_id), checkbox_value != 0 };
}

static bool UnderDebugger()
{
	return IsDebuggerPresent();
}

static auto CreateQueryHyperlink(std::string_view query, std::string_view inner_text = {})
{
	if (inner_text.empty())
		inner_text = query;

	std::string escaped = "<a href=\"https://www.google.com/search?q=";
	for (auto& c : query)
	{
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
			escaped += c;
		else
			escaped += fmt::format("%{:02X}", int((unsigned char)c));
	}
	escaped += "\">";
	escaped += inner_text;
	escaped += "</a>";
	return escaped;
}

void ghassanpl::ReportAssumptionFailure(ghassanpl::detail::source_location where, std::string_view expectation, std::initializer_list<std::pair<std::string_view, std::string>> values, std::string data)
{
	/*
	Reporter holder{ ReportType::AssumptionFailure };
	holder.FormatMessageLine("I assumed that {}", expectation);
	for (auto& [name, value] : values)
		holder.AddAdditionalInfo({ fmt::format("Value of '{}'", name), value });
	if (!data.empty())
		holder.AddAdditionalInfo({ "Additional Assumption Info", std::move(data) });

	Report->PerformReport(holder);
	*/
}

namespace gamelib
{
	Reporter::Reporter(IErrorReporter const& errep, ReportType type)
		: ErrorReporter(errep), Type(type)
	{
		AdditionalInfo("Timestamp", "{}", std::chrono::system_clock::now().time_since_epoch().count());
		AdditionalInfo("Thread", "{}", std::this_thread::get_id());

		std::array<char, 2048> message;
		const auto e = errno;
		strerror_s(message.data(), message.size(), e);
		AdditionalInfo("errno", "{} ({})", CreateQueryHyperlink(message.data()), CreateQueryHyperlink(fmt::format("errno {:X}", e), std::to_string(e)));

		auto gle = GetLastError();
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			NULL, gle & 0x3FFF, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message.data(), (DWORD)message.size(), NULL);
		AdditionalInfo("System Error", "{} ({})", CreateQueryHyperlink(message.data()), CreateQueryHyperlink(fmt::format("Windows Error Code 0x{:X}", gle), std::to_string(gle)));
	}
	
	void Reporter::Perform() { ErrorReporter.PerformReport(*this); }

	void IErrorReporter::PerformReport(Reporter const& holder) const
	{
		const char* type_str = "ERROR";
		const wchar_t* msg_header = L"Error";
		MessageBoxIcon icon = MessageBoxIcon::Error;
		switch (holder.Type)
		{
		case ReportType::Warning:
			type_str = "WARNING";
			msg_header = L"Warning";
			icon = MessageBoxIcon::Warning;
			break;
		case ReportType::AssumptionFailure:
			type_str = "ASSERT";
			msg_header = L"Assumption Failed";
			icon = MessageBoxIcon::Security;
			break;
		case ReportType::Info:
			type_str = "INFO";
			msg_header = L"Notice";
			icon = MessageBoxIcon::Information;
			break;
		default:
			break;
		}

		/*
		additional_info += fmt::format("Allegro errno : {}", al_get_errno());
		*/

		std::string message_text = fmt::format("{}", fmt::join(holder.MessageLines, "\n"));
		std::string additional_info_text;
		for (auto& [name, val] : holder.AdditionalInfoLines)
			additional_info_text += fmt::format("{}: {}\n", name, val);

		fmt::print(std::cerr, "[{}]\n{}\n{}\n", type_str, message_text, additional_info_text);

		static constexpr std::wstring_view buttons[] = { L"Debug", L"Abort" , L"Continue" };
		std::wstring long_msg{ message_text.begin(), message_text.end() }; /// poor man's conversion
		std::wstring long_add{ additional_info_text.begin(), additional_info_text.end() }; /// poor man's conversion
		auto result = ShowMessageBox(msg_header, icon, msg_header, buttons, 0, long_msg, { }, long_add, [](MessageBoxEvent event, uintptr_t a, uintptr_t b) {
			if (event == MessageBoxEvent::LinkClicked)
				ShellExecuteW(0, L"open", (const wchar_t*)b, NULL, NULL, SW_SHOWNORMAL);
			return true;
			});
		if (result.ClickedButton == 0)
			DebugBreak();
		else if (result.ClickedButton == 1)
			std::abort();
	}
}