#pragma once

#include "../Common.h"
#include "../Includes/EnumFlags.h"
#include <string_view>
#include <span>

enum class HardwareDeviceFlags
{

};

struct IHardwareDevice
{
	virtual ~IHardwareDevice() = default;

	virtual std::string_view Name() const;
	virtual enum_flags<HardwareDeviceFlags> DeviceFlags() const = 0;

	virtual bool IsConnected() const { return true; }

	virtual size_t SubDeviceCount() const { return 0; }
	virtual IHardwareDevice* SubDevice(size_t index) const { return nullptr; }

	enum class PowerSourceType
	{
		None,
		Wire,
		Batteries,
		InternalAccu,
		ExternalAccu,
		Unknown
	};

	struct PowerSourceStatus
	{
		progress_t ChargePercent{};
		seconds_t ChargeSeconds{};
		progress_t PowerDrawPercentage{};
		double MaximumPowerDraw{}; /// in amperes
	};

	virtual PowerSourceType PowerSource() const { return PowerSourceType::Unknown; }
	virtual enum_flags<PowerSourceType> AvailablePowerSources() const { return enum_flags<PowerSourceType>{}; }
	virtual PowerSourceStatus PowerSourceStatus(PowerSourceType type) const = 0;

	enum class StringProperty
	{
		Name,
		Company,
		ImageURL,
		Website,
		SerialNumber,
		VendorProductVersionID,
	};

	virtual bool IsStringPropertyValid(StringProperty property) const = 0;
	virtual std::string_view StringPropertyValue(StringProperty property) const = 0;

	enum class NumberProperty
	{
		PhysicalSize, /// vec3, in centimeters
		AutoOffTime, /// float, in seconds
		Range, /// float, in cm
		InternalTemperature, /// float, in C
		HIDUsage, /// vec3, first is usage page, second is usage id
	};

	virtual bool IsNumberPropertyValid(NumberProperty property) const = 0;
	virtual glm::vec3 NumberPropertyValue(NumberProperty property) const = 0;

	virtual void ForceRefresh() = 0;
	virtual void NewFrame() = 0;

	virtual void ResetDevice() {}
	virtual void EnableDevice(bool enable) {}

	enum class DataChannelFlags
	{
		CanSend,
		CanReceive,
		RequiresSynchronization,
		RequiresOwnership,
	};

	enum class DataChannelDirection
	{
		Send,
		Receive
	};

	using bytes_per_sec_t = double;

	struct IDataChannelDescriptor
	{
		virtual ~IDataChannelDescriptor() noexcept = default;
		virtual std::string_view Name() const = 0;
		virtual enum_flags<DataChannelFlags> Flags() const = 0;
		virtual size_t RequiredDataAlignment(DataChannelDirection dir) const = 0;
		virtual size_t PrefferedDataBufferSize(DataChannelDirection dir) const = 0;
		virtual bytes_per_sec_t CurrentBandwidth(DataChannelDirection dir) const = 0;
		virtual bytes_per_sec_t MaximumBandwidth(DataChannelDirection dir) const = 0;
	};

	virtual size_t DataChannelCount() = 0;
	virtual IDataChannelDescriptor const* DataChannelDescriptor(size_t index) = 0;
	virtual size_t SendData(size_t channel_index, std::span<std::byte const> data) = 0;
	virtual size_t ReceiveData(size_t channel_index, std::span<std::byte> data_buffer) = 0;
	virtual bool WaitForSync(size_t channel_index) = 0;
	virtual intptr_t TryGrabOwnership(size_t channel_index) = 0;
	virtual void ReleaseOwnership(size_t channel_index, intptr_t token) = 0;

	struct IMetricDescriptor
	{
		virtual ~IMetricDescriptor() noexcept = default;
		virtual std::string_view Name() = 0;
		virtual std::string_view Unit() = 0;
		virtual double Minimum() = 0;
		virtual double Maximum() = 0;
		virtual herz_t MinimumUpdateFrequency() = 0;
		virtual herz_t MaximumUpdateFrequency() = 0;
	};

	virtual size_t MetricCount() = 0;
	virtual IMetricDescriptor const* MetricDescriptor(size_t index) = 0;
	virtual double Metric(size_t index) = 0;
};

struct IDisplay : IHardwareDevice {};
struct IGPU : IHardwareDevice {};
struct ICamera : IHardwareDevice {};
struct ICPU : IHardwareDevice {};
struct IClock : IHardwareDevice {}; /// Do we even need this?
struct IAudioOutputDevice : IHardwareDevice {};
struct IAudioInputDevice : IHardwareDevice {};
struct INetworkDevice : IHardwareDevice {};
struct IIndicatorDevice : IHardwareDevice {}; /// lamps and such
struct IDataStorage : IHardwareDevice {};
