// Header guard
#ifndef UNICODE_H
#define UNICODE_H


// Header files
#include <cstdint>
#include <regex>
#include <vector>

using namespace std;


// Classes

// Unicode class
class Unicode final {

	// Public
	public:
	
		// Constructor
		Unicode() = delete;
	
		// UTF-8 to UTF-32
		static u32string utf8ToUtf32(const char *text);
		static u32string utf8ToUtf32(const string &text);
		static char32_t utf8ToUtf32(char character);

		// UTF-32 to UTF-8
		static string utf32ToUtf8(const char32_t *text);
		static string utf32ToUtf8(const u32string &text);
		static string utf32ToUtf8(char32_t character);

		// UTF-8 to UTF-16
		static u16string utf8ToUtf16(const char *text);
		static u16string utf8ToUtf16(const string &text);
		static char16_t utf8ToUtf16(char character);

		// UTF-16 to UTF-8
		static string utf16ToUtf8(const char16_t *text);
		static string utf16ToUtf8(const u16string &text);
		static string utf16ToUtf8(char16_t character);

		// Is valid UTF-8
		static bool isValidUtf8(const string &text);
		static bool isValidUtf8(const vector<uint8_t> &data);

		// Is valid UTF-16
		static bool isValidUtf16(const u16string &text);
		static bool isValidUtf16(const vector<uint16_t> &data);

		// Is valid UTF-32
		static bool isValidUtf32(const u32string &text);
		static bool isValidUtf32(const vector<uint32_t> &data);

		// Remove invalid UTF-8
		static string removeInvalidUtf8(const string &text);
		static string removeInvalidUtf8(const vector<uint8_t> &data);

		// Remove invalid UTF-16
		static u16string removeInvalidUtf16(const u16string &text);
		static u16string removeInvalidUtf16(const vector<uint16_t> &data);

		// Remove invalid UTF-32
		static u32string removeInvalidUtf32(const u32string &text);
		static u32string removeInvalidUtf32(const vector<uint32_t> &data);
		
		// UTF-16 high and low surrogate ranges
		static const char16_t UTF16_HIGH_SURROGATE_RANGE_BEGIN;
		static const char16_t UTF16_HIGH_SURROGATE_RANGE_END;
		static const char16_t UTF16_LOW_SURROGATE_RANGE_BEGIN;
		static const char16_t UTF16_LOW_SURROGATE_RANGE_END;

		// UTF-16 code page
		static const uint16_t UTF16_CODE_PAGE;
		
		// UTF-32 max code point
		static const char32_t UTF32_MAX_CODE_POINT;
	
	// Private
	private:
	
		// Use alternative methods
		static const bool USE_ALTERNATIVE_METHODS;

		// UTF-8 code points pattern
		static const regex UTF8_CODE_POINTS_PATTERN;
};


#endif
