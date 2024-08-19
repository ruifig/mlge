#pragma once

namespace cz
{

/**
 * Convert a utf16/32 to utf8
 */
std::string narrow(std::wstring_view str);
std::string narrow(std::u16string_view str);
std::string narrow(std::u32string_view str);

/**
 * Converts a utf8 string to a wide string (OS dependent).
 * std::wstring character width is OS dependent, so this fucntion is handy when interacting with the OS or some of the STL. E.g,
 * opening a file.
 */
 std::wstring widen(std::string_view str);



/**
 * Using this, so we can use std::string as a key to the map, but not having to allocate a std::string every time we need to do a
 * lookup.
 * Explanation: https://www.cppstories.com/2021/heterogeneous-access-cpp20/ 
 *
 * A map holding values Foo can be declared as:
 * std::unordered_map<std::string, Foo, string_hash, std::equal_to<>> values;
 * 
 */
 struct string_hash
 {
	 using is_transparent = void;
	 [[nodiscard]] size_t operator()(const char* txt) const
	 {
		 return std::hash<std::string_view>{}(txt);
	 }

	 [[nodiscard]] size_t operator()(std::string_view txt) const
	 {
		 return std::hash<std::string_view>{}(txt);
	 }

	 [[nodiscard]] size_t operator()(const std::string& txt) const
	 {
		 return std::hash<std::string>{}(txt);
	 }
 };


 /**
  * Returns tru if the specified character is a whitespace character
  */
bool whitespaceCharacter(int a);

/**
 * Returns true if the specified character is NOT space tab or line ending
 */
inline bool notWhitespaceCharacter(int ch)
{
	return !whitespaceCharacter(ch);
}

/**
 * Trim the left side of a string
 */
template <class StringType>
static inline StringType ltrim(const StringType& s_)
{
	StringType s = s_;
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), notWhitespaceCharacter));
	return s;
}

/**
 * Trim right side of a string
 */
template <class StringType>
static inline StringType rtrim(const StringType& s_)
{
	StringType s = s_;
	s.erase(std::find_if(s.rbegin(), s.rend(), notWhitespaceCharacter).base(), s.end());
	return s;
}


/**
 * Trim both sides of a string
 */
template <class StringType>
static inline StringType trim(const StringType& s)
{
	return ltrim(rtrim(s));
}

/**
 * Compares two ASCII strings, ignoring case
 * Only the characters 'ABCDEFGHIJKLMNOPQRSTUVWXYZ' are converted for comparison.
 */
bool asciiStrEqualsCi(std::string_view str1, std::string_view str2);

/**
 * Splits a string into lines and puts them into a vector
 */
std::vector<std::string> stringSplitIntoLinesVector(const char* textbuffer, size_t buffersize);

} // namespace cz


