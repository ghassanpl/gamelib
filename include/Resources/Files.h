#pragma once

#include <filesystem>
#include <span>
#include <optional>
#include <concepts>
#include <bit>
#include <functional>
#include "../Common.h"
#include "../Includes/EnumFlags.h"

namespace gamelib
{
	using file_path = std::filesystem::path;

	/// NOTE: This enum is declared outside of IFileSystemEntry for now, because Visual Studio has a bug where 
	/// it does not match `requires` expressions between function declarations and definitions unless 
	/// they are token-for-token identical...
	enum class IterationResult
	{
		NextRecurse,
		NextSkip,
		Stop
	};

	struct IFile
	{
		IFile(std::shared_ptr<IErrorReporter> rep);

		enum class RelativeSeek
		{
			Set,
			Current,
			End
		};

		virtual ~IFile() = default;

		virtual auto Read(std::span<byte> buffer) -> size_t = 0;
		//virtual auto ReadLine()->std::string= 0;
		//virtual auto ReadString()->std::string= 0;/// nul-terminated
		virtual auto Write(std::span<byte const> buffer) -> size_t = 0;
		virtual auto Write(std::string_view str) -> size_t;

		template <typename T> 
		requires std::is_integral_v<T>
		auto TryRead(T& buffer, std::endian source_endianness) -> bool;

		template <std::endian ENDIANNESS, typename T>
		requires std::is_integral_v<T>
		auto TryRead(T& buffer) -> bool;

		virtual void Flush() = 0;
		virtual auto CurrentPosition() -> intptr_t = 0;
		virtual auto Seek(intptr_t offset, RelativeSeek from) -> intptr_t = 0;
		virtual auto Size() -> intptr_t = 0;
		virtual bool EndOfFile() = 0;
		virtual bool IsError() = 0;

		template <typename T> 
		requires std::is_trivially_copyable_v<T>
		auto Read() -> T;

		template <typename T> 
		requires std::is_trivially_copyable_v<T>
		auto TryRead() -> std::optional<T>;

		template <typename T>
		requires std::is_trivially_copyable_v<T>
		bool Write(T const& val);
		
		/// TODO: Buffering?
	};

	struct IFileSystemEntry : std::enable_shared_from_this<IFileSystemEntry>
	{
		IFileSystemEntry(std::shared_ptr<IErrorReporter> rep);

		virtual ~IFileSystemEntry() = default;

		enum class Property
		{
			Readable,
			Writeable,
			Executable,
			Hidden,
			File,
			Directory,
			Compressed,
			Encrypted
		};

		using FileTime = std::chrono::time_point<std::chrono::system_clock>;

		/// Properties
		virtual bool Exists() = 0;
		virtual auto Path() const -> file_path = 0;
		virtual auto Properties() const-> enum_flags<Property> = 0;
		virtual bool IsDirectory() const { return Properties().is_set(Property::Directory); }
		virtual bool IsFile() const { return Properties().is_set(Property::File); }
		virtual bool IsHidden() const { return Properties().is_set(Property::Hidden); }
		virtual bool IsExecutable() const { return Properties().is_set(Property::Executable); }
		virtual bool IsWriteable() const { return Properties().is_set(Property::Writeable); }
		virtual bool IsReadable() const { return Properties().is_set(Property::Readable); }
		virtual bool IsCompressed() const { return Properties().is_set(Property::Compressed); }
		virtual bool IsEncrypted() const { return Properties().is_set(Property::Encrypted); } /// FileEncryptionStatusW
		virtual auto LastAccessTime() const -> FileTime = 0;
		virtual auto CreationTime() const -> FileTime = 0;
		virtual auto LastModificationTime() const -> FileTime = 0;
		virtual auto Size() const -> size_t = 0;
		virtual void Refresh() {}

		/// TODO: owner, group, permissions

		/// Actions
		virtual bool Remove() = 0;
		virtual bool SoftLinkTo(file_path const& destination) = 0; /// Junctions/CreateSymbolicLinkA 
		virtual bool HardLinkTo(file_path const& destination) = 0; /// CreateHardLinkA

		/// TODO: Encrypt/Decrypt
	};

	struct IFileSystemEntry_Directory : IFileSystemEntry
	{
		IFileSystemEntry_Directory(std::shared_ptr<IErrorReporter> rep);

		virtual auto ChildEntry(std::string_view with_name) -> std::shared_ptr<IFileSystemEntry> = 0;

		/// Directory Iteration
		/// TODO: This requires this file system entry to hold an iteration state, probably not the best idea
		virtual bool OpenDirectory() = 0;
		virtual auto NextDirectoryEntry()->std::shared_ptr<IFileSystemEntry> = 0;
		virtual bool CloseDirectory() = 0;

		/// TODO: Should we do `auto DirectoryEntries() -> vector<shared_ptr<IFileSystemEntry>>` ???
		/// TODO: Should we try to emulate std::directory_iterator ?

		template <typename CALLBACK>
		requires invocable_with_result<CALLBACK, IterationResult, std::shared_ptr<IFileSystemEntry>>
		bool ForEachEntry(CALLBACK&& predicate);
	};

	struct IFileSystemEntry_File : IFileSystemEntry
	{
		IFileSystemEntry_File(std::shared_ptr<IErrorReporter> rep);

		virtual auto OpenFile(const char* mode) -> std::unique_ptr<IFile> = 0;
		virtual bool ReplaceWith(std::shared_ptr<IFileSystemEntry> other) = 0;
		virtual bool CopyTo(file_path const& destination) = 0; /// TODO: Flags (CopySymlink, FailIfExists, Resumable); Callback progress
	};

	enum class FileSystemFlags
	{
		ReadOnly,
		Encryptable,
		FilesCompressable,
		Compressed,
		SupportsOwnerships,
		CaseSensitive,
	};

	struct IFileSystem : std::enable_shared_from_this<IFileSystem>
	{
		IFileSystem(std::shared_ptr<IErrorReporter> rep);

		virtual ~IFileSystem() = default;

		virtual auto Resolve(file_path const& path) -> std::shared_ptr<IFileSystemEntry> = 0;
		virtual auto Root()->std::shared_ptr<IFileSystemEntry>;

		virtual bool Exists(file_path const& path);
		virtual bool Remove(file_path const& path);
		
		virtual auto CurrentDirectory() -> file_path = 0;
		virtual bool ChangeDirectory(file_path const& path) = 0;
		virtual bool CreateDirectory(file_path const& path) = 0;

		virtual auto OpenFile(file_path const& path, const char* mode) -> std::unique_ptr<IFile>;

		struct Sizes
		{
			size_t TotalSize;
			size_t FreeSize;
		};

		/// GetVolumeInformationW, GetDiskFreeSpaceExW, GetDriveTypeW 
		virtual std::string Name() = 0;
		virtual std::string Type() = 0;
		virtual Sizes Sizes() = 0;
		virtual enum_flags<FileSystemFlags> Flags() = 0;

		enum class SpecialPath
		{
			Resources,
			Temporary,
			Home,
			Documents,
			UserData,
			Settings,
			Executable
		};

		virtual auto GetSpecialPath(SpecialPath path, const char* app_name = nullptr, const char* org_name = nullptr) -> file_path = 0; /// al_get_standard_path
	};

	using virtual_file_path = file_path;

	struct IVirtualFileSystem : IFileSystem
	{
		IVirtualFileSystem(std::shared_ptr<IErrorReporter> rep);

		virtual auto MountPoints() const -> std::vector<std::pair<virtual_file_path, file_path>> = 0;
		virtual void ForEachMountPoint(std::function<bool(virtual_file_path const&, file_path const&)> callback) = 0;

		virtual auto ResolveVirtualPath(virtual_file_path const& path) const -> std::pair<file_path, virtual_file_path> = 0;

		virtual bool Mount(file_path const& real_path, virtual_file_path const& virtual_path) = 0;
		virtual bool Mount(std::shared_ptr<IFileSystem> other_fs, virtual_file_path const& virtual_path) = 0;
		virtual bool Mount(std::unique_ptr<IFile> archive_file, virtual_file_path const& virtual_path) = 0;

		virtual bool Unmount(file_path const& real_path) = 0;
		virtual bool Unmount(IFileSystem& other_fs) = 0;
		virtual bool Unmount(IFile& archive_file) = 0;

		/// TODO: This?
		/// virtual bool MountAppend(file_path const& real_path, virtual_file_path const& virtual_path) = 0;

	protected:

	};

	/// TODO: List available filesystems
}

#define GAMELIB_IMPL
#include "Files.impl.h"
#undef GAMELIB_IMPL