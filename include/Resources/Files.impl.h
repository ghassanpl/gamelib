#pragma once

#ifndef GAMELIB_IMPL
#include "Files.h"
#endif

namespace gamelib
{
	
	inline auto IFileSystem::Root() -> std::shared_ptr<IFileSystemEntry>
	{
		return Resolve("/");
	}
	
	inline bool IFileSystem::Exists(file_path const& path)
	{
		return !!Resolve(path);
	}

	inline bool IFileSystem::Remove(file_path const& path)
	{
		if (auto entry = Resolve(path))
			return entry->Remove();
		return false;
	}

	inline auto IFileSystem::OpenFile(file_path const& path, const char* mode) -> std::unique_ptr<IFile>
	{
		if (auto entry = Resolve(path); entry && entry->IsFile())
			return static_cast<IFileSystemEntry_File*>(entry.get())->OpenFile(mode);
		return {};
	}

	template <typename CALLBACK>
	requires invocable_with_result<CALLBACK, IterationResult, std::shared_ptr<IFileSystemEntry>>
	inline bool IFileSystemEntry_Directory::ForEachEntry(CALLBACK&& predicate)
	{
		if (!OpenDirectory())
			return false;

		while (auto entry = NextDirectoryEntry())
		{
			auto result = predicate(entry);
			if (result == IterationResult::NextSkip)
				continue;
			else if (result == IterationResult::NextRecurse)
			{
				if (!entry->IsDirectory() || static_cast<IFileSystemEntry_Directory*>(entry.get())->ForEachEntry(predicate))
					continue;
			}

			break;
		}

		return CloseDirectory();
	}

	template <typename T> 
	requires std::is_trivially_copyable_v<T>
	inline auto IFile::Read() -> T
	{
		T val = {};
		if (Read(&val, sizeof(val)) != sizeof(val))
			throw std::underflow_error("too few bytes read");
		return val;
	}

	template <typename T>
	requires std::is_trivially_copyable_v<T>
	inline auto IFile::TryRead() -> std::optional<T>
	{
		T val = {};
		if (Read(&val, sizeof(val)) != sizeof(val))
			return std::nullopt;
		return val;
	}

	template<typename T> 
	requires std::is_trivially_copyable_v<T>
	inline bool IFile::Write(T const & val)
	{
		return Write(&val, sizeof(val)) != sizeof(val);
	}

	inline auto IFile::Write(std::string_view str) -> size_t
	{
		return Write(std::span<byte const>{ reinterpret_cast<byte const*>(str.data()), str.size() });
	}
}