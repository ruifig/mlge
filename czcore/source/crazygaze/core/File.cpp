#include "File.h"
#include "Logging.h"
#include "StringUtils.h"

namespace cz
{

File::~File()
{
	if (m_handle)
	{
		fclose(m_handle);
	}
}

std::unique_ptr<File> File::open(const fs::path& path, Mode mode)
{
	return openImpl(path, mode, true);
}

std::unique_ptr<File> File::try_open(const fs::path& path, Mode mode)
{
	return openImpl(path, mode, false);
}

std::unique_ptr<File> File::openImpl(const fs::path& path, Mode mode, bool raiseError)
{
	wchar_t fmodebuf[4];
	wchar_t* fmode = &fmodebuf[0];

	switch(mode)
	{
		case Mode::Read:
			*fmode = 'r';
			break;
		case Mode::Write:
			*fmode = 'w';
			break;
		case Mode::Append:
			*fmode = 'a';
			break;
		case Mode::ReadWriteExisting:
			// Opens for read write, but the file needs to exist
			*(fmode++) = 'r';
			*fmode = '+';
			break;
		case Mode::ReadWriteNew:
			*(fmode++) = 'w';
			*fmode = '+';
			break;
		case Mode::ReadWriteAppend:
			*(fmode++) = 'a';
			*fmode = '+';
			break;
	}

	*(++fmode) ='b';
	(++fmode)[0] = 0;

	FILE* handle = _wfopen(path.c_str(), fmodebuf);
	if (handle == NULL)
	{
		if (raiseError)
		{
			CZ_LOG(Error, "Couldn't open file '{}', with mode '{}'.", narrow(path.c_str()), static_cast<int>(mode));
		}
		return nullptr;
	}

	auto file = std::make_unique<File>(this_is_private{0});
	file->m_path = path;
	file->m_handle = handle;
	file->m_mode = mode;

	return file;
}

size_t File::write(const void* buffer, size_t size, size_t count)
{
	// Any mode other than Mode::Read is writable
	CZ_CHECK(m_mode != Mode::Read);

	size_t r = fwrite(buffer, size, count, m_handle);

	CZ_CHECK_F((size*count==0) || (r==count), "{} failed. Requested {} elements ({} bytes each), and did {}", __FUNCTION__, static_cast<int>(count), static_cast<int>(size), static_cast<int>(r));

	return r;
}

size_t File::read(void* buffer, size_t size, size_t count)
{
	// Any other modes other than Mode:Write & Mode::Append are readable
	CZ_CHECK(!(m_mode==Mode::Write || m_mode==Mode::Append));

	size_t r = fread(buffer, size, count, m_handle);
	CZ_CHECK_F((size*count==0) || r==count, "{} failed. Requested {} elements ({} bytes each), and did {}", __FUNCTION__, static_cast<int>(count), static_cast<int>(size), static_cast<int>(r));
	return r;
}

size_t File::write(const void* buffer, size_t bytes)
{
	return write(buffer, bytes, 1);
}

size_t File::read(void* buffer, size_t bytes)
{
	return read(buffer, bytes, 1) * bytes;
}

bool File::eof() const
{
	return (feof(m_handle)==0) ? false : true;
}

size_t File::tell() const
{
	long p = ftell(m_handle);
	CZ_CHECK(p != -1);
	return static_cast<size_t>(p);
}

bool File::seek(size_t offset, SeekMode seekMode)
{
	int smode = 0;

	switch(seekMode)
	{
		case SeekMode::Set:
			smode = SEEK_SET;
			break;
		case SeekMode::Current:
			smode = SEEK_CUR;
			break;
		case SeekMode::End:
			smode = SEEK_END;
	}

	int r = fseek(m_handle, static_cast<long>(offset), smode);
	if (r != 0)
	{
		CZ_LOG(Error, "{} failed", __FUNCTION__);
	}

	return (r==0) ? true : false;
}

size_t File::size()
{
	size_t originalPos = tell();

	seek(0, SeekMode::End);
	size_t size = tell();
	seek(originalPos, SeekMode::Set);
	return size;
}

} // namespace cz

