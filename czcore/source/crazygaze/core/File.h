#pragma once

#include "Common.h"
#include "PlatformUtils.h"

namespace cz
{

/**
 * Very simple file reading/writing class
 * Just because I hate using STL streams. 
 *
 * WARNING: Some of the C functions used (e.g: ftell) use signed 32 bits values, therefore this class should only be used for
 * files <= 2GB in size
 */
class File
{
  public:

	/**
	 * File opening mode. See https://en.cppreference.com/w/cpp/io/c/fopen
	 */
	enum class Mode
	{
		/**
		 * Open for reading. Translates to C's "r" mode
		 * - Action if file already exists     : Read from start
		 * - Action if the file does not exist : Fails to open
		 */
		Read,

		/**
		 * Open for writing. Translates to C's "w" mode
		 * - Action if file already exists     : Destroy contents
		 * - Action if the file does not exist : Create new
		 */
		Write,

		/**
		 * Opens for appending. Translates to C's "a"
		 * - Action if file already exists     : Write to end
		 * - Action if the file does not exist : Create new
		 */
		Append,

		/**
		 * Opens for read&write and the file needs to exist. Translates to C's "r+"
		 * - Action if file already exists     : Read from start
		 * - Action if the file does not exist : Fails to open
		 */
		ReadWriteExisting,

		/**
		 * Opens for read&write overwriting if the file already exists. Translates to C's "w+"
		 * - Action if file already exists     : Destroy contents
		 * - Action if the file does not exist : Create new
		 */
		ReadWriteNew,

		/**
		 * Opens for read&write and sets the cursor to the end of the file . Translates to C's "a+"
		 * - Action if file already exists     : Write to end
		 * - Action if the file does not exist : Create new
		 */
		ReadWriteAppend
	};

	enum class SeekMode
	{
		Set,
		Current,
		End
	};

	CZ_DELETE_COPY_AND_MOVE(File);

	~File();

	/**
	 * Opens the file
	 * It logs an error if the file can't be opened.
	 *
	 * @param path Path to the file. No attempt is made to resolve relative paths.
	 */
	static std::unique_ptr<File> open(const fs::path& path, Mode mode);

	/**
	 * Opens the file
	 * It doesn't log an error if the file can't be opened
	 *
	 * @param path Path to the file. No attempt is made to resolve relative paths.
	 */
	static std::unique_ptr<File> try_open(const fs::path& path, Mode mode);

	size_t write(const void* buffer, size_t size, size_t count);
	size_t read(void* buffer, size_t size, size_t count);
	size_t write(const void* buffer, size_t bytes);
	size_t read(void* buffer, size_t bytes);
	bool eof() const;
	size_t tell() const;
	bool seek(size_t offset, SeekMode seekMode);

	size_t size();

	const fs::path& getPath() const
	{
		return m_path;
	}

	//! Returns the internal FILE handle.
	/*
	* NOTE: Be careful what you do with it.
	*/
	FILE* getHandle()
	{
		return m_handle;
	}

protected:
	static std::unique_ptr<File> openImpl(const fs::path& path, Mode mode, bool raiseError);
	fs::path m_path;
	FILE* m_handle = nullptr;
	Mode m_mode;

	// Provides a way to use make_shared with private/protected constructors
	struct this_is_private
	{
		explicit this_is_private(int) {}
	};

public:	
	File(const this_is_private) {}

};

} // namespace cz



