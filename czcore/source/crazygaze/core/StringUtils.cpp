#include "StringUtils.h"

namespace cz
{

namespace
{

// As far as I've noticed, utfcpp doesn't have a ready to use function to validate a UTF16 string,
// so this based on the utf8::utf16to8 function. Instead of throwing we return a boolean.
template <typename u16bit_iterator>
bool isValidUtf16_Impl(u16bit_iterator start, u16bit_iterator end)
{
	while (start != end) {
		uint32_t cp = utf8::internal::mask16(*start++);
		// Take care of surrogate pairs first
		if (utf8::internal::is_lead_surrogate(cp)) {
			if (start != end) {
				uint32_t trail_surrogate = utf8::internal::mask16(*start++);
				if (utf8::internal::is_trail_surrogate(trail_surrogate))
					cp = (cp << 10) + trail_surrogate + utf8::internal::SURROGATE_OFFSET;
				else
					return false; // throw invalid_utf16(static_cast<uint16_t>(trail_surrogate));
			}
			else
				return false; // throw invalid_utf16(static_cast<uint16_t>(cp));

		}
		// Lone trail surrogate
		else if (utf8::internal::is_trail_surrogate(cp))
			return false; // throw invalid_utf16(static_cast<uint16_t>(cp));
	}

	return true;
}

// unchecked::utf16to8 can crash if given invalid input. See https://github.com/nemtrif/utfcpp/issues/78
// The author wants to leave it as-is, since it's in the "unckecked" namespace, so we have this function copied and adapted.
template <typename u16bit_iterator, typename octet_iterator>
octet_iterator utf16to8_lenient (u16bit_iterator start, u16bit_iterator end, octet_iterator result)
{
	while (start != end) {
		uint32_t cp = utf8::internal::mask16(*start++);
		// Take care of surrogate pairs first
		if (utf8::internal::is_lead_surrogate(cp)) {
			if (start >= end) // #RVF : This is the change compared to the original code
				return result;
			uint32_t trail_surrogate = utf8::internal::mask16(*start++);
			cp = (cp << 10) + trail_surrogate + utf8::internal::SURROGATE_OFFSET;
		}
		result = utf8::unchecked::append(cp, result);
	}
	return result;
}

} // unnamed namespace

std::wstring widen(std::string_view str)
{
	std::wstring result;
	if constexpr(sizeof(std::wstring::value_type)==2)
	{
		utf8::unchecked::utf8to16(str.begin(), str.end(), std::back_inserter(result));
	}
	else
	{
		utf8::unchecked::utf8to32(str.begin(), str.end(), std::back_inserter(result));
	}
	return result;
}

std::string narrow(std::wstring_view str)
{
	std::string result;
	if constexpr(sizeof(std::wstring::value_type)==2)
	{
		utf16to8_lenient(str.begin(), str.end(), std::back_inserter(result));
	}
	else
	{
		utf8::unchecked::utf32to8(str.begin(), str.end(), std::back_inserter(result));
	}
	return result;
}

std::string narrow(std::u16string_view str)
{
	std::string result;
	utf16to8_lenient(str.begin(), str.end(), std::back_inserter(result));
	return result;
}

std::string narrow(std::u32string_view str)
{
	std::string result;
	utf8::unchecked::utf32to8(str.begin(), str.end(), std::back_inserter(result));
	return result;
}

bool asciiStrEqualsCi(std::string_view str1, std::string_view str2)
{
	if (str1.size() != str2.size())
	{
		return false;
	}

	static auto tolower = [](char ch)
	{
		return (ch>='A' && ch<='Z') ? (ch + ('a'-'A')) : ch;
	};

	std::string_view::const_iterator it1 = str1.begin();
	std::string_view::const_iterator it2 = str2.begin();
	while (it1 != str1.end())
	{
		if (tolower(*it1) != tolower(*it2))
		{
			return false;
		}

		++it1;
		++it2;
	}

	return true;
}

std::vector<std::string> stringSplitIntoLinesVector(const char* textbuffer, size_t buffersize)
{
	if (textbuffer == nullptr)
	{
		return {};
	}

	std::vector<std::string> lines;
	const char* s = textbuffer;

	while (*s != 0 && s < textbuffer + buffersize)
	{
		const char* ptrToChar = s;
		while (!(*s == 0 || *s == 0xA || *s == 0xD))
		{
			s++;
		}

		auto numchars = s - ptrToChar;
		lines.emplace_back(ptrToChar, ptrToChar + numchars);

		// New lines format are:
		// Unix		: 0xA
		// Mac		: 0xD
		// Windows	: 0xD 0xA
		// If windows format a new line has 0xD 0xA, so we need to skip one extra character
		if (*s == 0xD && *(s + 1) == 0xA)
		{
			s++;
		}

		if (*s == 0)
		{
			break;
		}

		s++;  // skip the newline character
	}

	return lines;
}

bool whitespaceCharacter(int ch)
{
// See https://www.asciitable.com/
    return
		ch==' '  || // space
		ch=='\t' || // tab
		ch==0xA  || // line feed
		ch==0xB  || // vertical tab
		ch==0xC  || // form feed
		ch==0xD;    // carriage return
}

} // namespace cz


