// Header files
#include <codecvt>
#include <locale>
#include "unicode.h"

using namespace std;


// Constants

// UTF-16 high and low surrogate ranges
const char16_t Unicode::UTF16_HIGH_SURROGATE_RANGE_BEGIN = 0xD800;
const char16_t Unicode::UTF16_HIGH_SURROGATE_RANGE_END = 0xDBFF;
const char16_t Unicode::UTF16_LOW_SURROGATE_RANGE_BEGIN = 0xDC00;
const char16_t Unicode::UTF16_LOW_SURROGATE_RANGE_END = 0xDFFF;

// UTF-16 code page
const uint16_t Unicode::UTF16_CODE_PAGE = 1200;

// UTF-32 max code point
const char32_t Unicode::UTF32_MAX_CODE_POINT = 0x10FFFF;

// Use alternative methods
const bool Unicode::USE_ALTERNATIVE_METHODS = true;

// UTF-8 code points pattern
const regex Unicode::UTF8_CODE_POINTS_PATTERN("(?:[\x09\x0A\x0D\x20-\x7E]|[\xC2-\xDF][\x80-\xBF]|\xE0[\xA0-\xBF][\x80-\xBF]|[\xE1-\xEC\xEE\xEF][\x80-\xBF]{2}|\xED[\x80-\x9F][\x80-\xBF]|\xF0[\x90-\xBF][\x80-\xBF]{2}|[\xF1-\xF3][\x80-\xBF]{3}|\xF4[\x80-\x8F][\x80-\xBF]{2})*", regex_constants::ECMAScript);


// Supporting function implementation
u32string Unicode::utf8ToUtf32(const char *text) {

	// Return UTF-32 string
	return wstring_convert<codecvt_utf8<char32_t>, char32_t>{}.from_bytes(text);
}

u32string Unicode::utf8ToUtf32(const string &text) {

	// Return UTF-32 string
	return utf8ToUtf32(text.c_str());
}

char32_t Unicode::utf8ToUtf32(char character) {

	// Return UTF-32 character
	return utf8ToUtf32(string(1, character))[0];
}

string Unicode::utf32ToUtf8(const char32_t *text) {

	// Return UTF-8 string
	return wstring_convert<codecvt_utf8<char32_t>, char32_t>{}.to_bytes(text);
}

string Unicode::utf32ToUtf8(const u32string &text) {

	// Return UTF-8 string
	return utf32ToUtf8(text.c_str());
}

string Unicode::utf32ToUtf8(char32_t character) {

	// Return UTF-8 character
	return utf32ToUtf8(u32string(1, character));
}

u16string Unicode::utf8ToUtf16(const char *text) {

	// Return UTF-16 string
	return wstring_convert<codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(text);
}

u16string Unicode::utf8ToUtf16(const string &text) {

	// Return UTF-16 string
	return utf8ToUtf16(text.c_str());
}

char16_t Unicode::utf8ToUtf16(char character) {

	// Return UTF-16 character
	return utf8ToUtf16(string(1, character))[0];
}

string Unicode::utf16ToUtf8(const char16_t *text) {

	// Return UTF-8 string
	return wstring_convert<codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(text);
}

string Unicode::utf16ToUtf8(const u16string &text) {

	// Return UTF-8 string
	return utf16ToUtf8(text.c_str());
}

string Unicode::utf16ToUtf8(char16_t character) {

	// Return UTF-8 character
	return utf16ToUtf8(u16string(1, character));
}

bool Unicode::isValidUtf8(const string &text) {

	// Check if using alternative methods
	if(USE_ALTERNATIVE_METHODS) {
	
		// Go through all code points in the text
		for(string::size_type i = 0; i < text.length(); ++i) {
		
			// Initialize code point
			const char &codePoint = text[i];
			
			// Initialize character
			string character;
			
			// Check if code point represents an ASCII character
			if((codePoint & 0b10000000) == 0b00000000)
			
				// Append code point to character
				character += codePoint;
			
			// Otherwise code point starts the represention of a non-ASCII character
			else if((codePoint & 0b11000000) == 0b11000000) {
			
				// Set length
				uint8_t length;
				if((codePoint & 0b11100000) == 0b11000000)
					length = 2;
				else if((codePoint & 0b11110000) == 0b11100000)
					length = 3;
				else if((codePoint & 0b11111000) == 0b11110000)
					length = 4;
				
				// Otherwise assume length is invalid
				else
				
					// Return false
					return false;
				
				// Go through all bytes in the code point
				for(uint8_t j = 0; j < length && i < text.length(); ++j, ++i) {
				
					// Initialize byte
					const char &byte = text[i];
					
					// Check if byte is invalid
					if(j != 0 && (byte & 0b11000000) != 0b10000000)
						
						// Return false
						return false;
				
					// Append byte to character
					character += byte;
				}
				
				// Decrement index
				--i;
				
				// Check if code point doesn't have its expected size
				if(character.length() != length)
				
					// Return false
					return false;
			}
			
			// Otherwise
			else
			
				// Return false
				return false;
			
			// Check if character isn't a valid UTF-8 character
			if(!regex_match(character, UTF8_CODE_POINTS_PATTERN, regex_constants::format_default | regex_constants::match_default))
			
				// Return false
				return false;
		}
		
		// Return true
		return true;
	}
	
	// Otherwise
	else
	
		// Return if text is a valid UTF-8 string
		return regex_match(text, UTF8_CODE_POINTS_PATTERN, regex_constants::format_default | regex_constants::match_default);
}

bool Unicode::isValidUtf8(const vector<uint8_t> &data) {

	// Return if data is a valid UTF-8 string
	return isValidUtf8(string(data.begin(), data.end()));
}

bool Unicode::isValidUtf16(const u16string &text) {

	// Go through all code points in the text
	bool inSurrogatePair = false;
	for(char16_t codePoint : text) {
	
		// Check if code point is a single unpaired surrogate
		if(!inSurrogatePair && codePoint >= UTF16_LOW_SURROGATE_RANGE_BEGIN && codePoint <= UTF16_LOW_SURROGATE_RANGE_END)
		
			// Return false
			return false;
		
		// Otherwise check if previous code point is a single unpaired surrogate
		else if(inSurrogatePair && (codePoint < UTF16_LOW_SURROGATE_RANGE_BEGIN || codePoint > UTF16_LOW_SURROGATE_RANGE_END))
		
			// Return false
			return false;
		
		// Otherwise check if code point starts a surrogate pair
		else if(codePoint >= UTF16_HIGH_SURROGATE_RANGE_BEGIN && codePoint <= UTF16_HIGH_SURROGATE_RANGE_END)
		
			// Set in surrogate pair
			inSurrogatePair = true;
	}
	
	// Check if text ended with a single unpaired surrogate
	if(inSurrogatePair)
	
		// Return false
		return false;
	
	// Return true
	return true;
}

bool Unicode::isValidUtf16(const vector<uint16_t> &data) {

	// Return if data is a valid UTF-16 string
	return isValidUtf16(u16string(data.begin(), data.end()));
}

bool Unicode::isValidUtf32(const u32string &text) {

	// Go through all code points in the text
	for(char32_t codePoint : text)
	
		// Check if code point is invalid
		if(codePoint > UTF32_MAX_CODE_POINT)
		
			// Return false
			return false;

	// Return true
	return true;
}

bool Unicode::isValidUtf32(const vector<uint32_t> &data) {

	// Return if data is a valid UTF-32 string
	return isValidUtf32(u32string(data.begin(), data.end()));
}

string Unicode::removeInvalidUtf8(const string &text) {

	// Initialize return value
	string returnValue;
	
	// Check if using alternative methods
	if(USE_ALTERNATIVE_METHODS) {

		// Go through all code points in the text
		for(string::size_type i = 0; i < text.length(); ++i) {
		
			// Initialize code point
			const char &codePoint = text[i];
			
			// Initialize character
			string character;
			
			// Check if code point represents an ASCII character
			if((codePoint & 0b10000000) == 0b00000000)
			
				// Append code point to character
				character += codePoint;
			
			// Otherwise code point starts the represention of a non-ASCII character
			else if((codePoint & 0b11000000) == 0b11000000) {
			
				// Set length
				uint8_t length;
				if((codePoint & 0b11100000) == 0b11000000)
					length = 2;
				else if((codePoint & 0b11110000) == 0b11100000)
					length = 3;
				else if((codePoint & 0b11111000) == 0b11110000)
					length = 4;
				
				// Otherwise assume length is invalid
				else
				
					// Go to the next code point
					continue;
				
				// Go through all bytes in the code point
				for(uint8_t j = 0; j < length && i < text.length(); ++j, ++i) {
				
					// Initialize byte
					const char &byte = text[i];
					
					// Check if byte is invalid
					if(j != 0 && (byte & 0b11000000) != 0b10000000)
						
						// Stop going through bytes
						break;
				
					// Append byte to character
					character += byte;
				}
				
				// Decrement index
				--i;
				
				// Check if code point doesn't have its expected size
				if(character.length() != length)
				
					// Go to the next code point
					continue;
			}
			
			// Otherwise
			else
			
				// Go to the next code point
				continue;
			
			// Check if character is a valid UTF-8 character
			if(isValidUtf8(character))
			
				// Append character to return value
				returnValue.append(character);
		}
	}
	
	// Otherwise
	else
	
		// Go through all valid UTF-8 code points in the text
		for(sregex_iterator i(text.begin(), text.end(), UTF8_CODE_POINTS_PATTERN, regex_constants::format_default | regex_constants::match_default), end; i != end; ++i)
			
			// Append code point's character to return value
			returnValue.append(i->str());
	
	// Return return value
	return returnValue;
}

string Unicode::removeInvalidUtf8(const vector<uint8_t> &data) {

	// Return data with only valid UTF-8 parts
	return removeInvalidUtf8(string(data.begin(), data.end()));
}

u16string Unicode::removeInvalidUtf16(const u16string &text) {

	// Initialize return value
	u16string returnValue;

	// Go through all code points in the text
	bool inSurrogatePair = false;
	char16_t previousCodePoint = u'\0';
	for(char16_t codePoint : text) {
		
		// Check if code point isn't part of a surrogate pair
		if(codePoint < UTF16_HIGH_SURROGATE_RANGE_BEGIN || codePoint > UTF16_LOW_SURROGATE_RANGE_END) {
		
			// Clear in surrogate pair
			inSurrogatePair = false;
		
			// Append code point to return value
			returnValue.push_back(codePoint);
		}
		
		// Otherwise check if code point finishes a surrogate pair
		else if(inSurrogatePair && codePoint >= UTF16_LOW_SURROGATE_RANGE_BEGIN && codePoint <= UTF16_LOW_SURROGATE_RANGE_END) {
		
			// Clear in surrogate pair
			inSurrogatePair = false;
		
			// Append code points to return value
			returnValue.push_back(previousCodePoint);
			returnValue.push_back(codePoint);
		}
		
		// Otherwise check if code point starts a surrogate pair
		else if(codePoint >= UTF16_HIGH_SURROGATE_RANGE_BEGIN && codePoint <= UTF16_HIGH_SURROGATE_RANGE_END) {
		
			// Set in surrogate pair
			inSurrogatePair = true;
		
			// Update previous code point
			previousCodePoint = codePoint;
		}
	}
	
	// Return return value
	return returnValue;
}

u16string Unicode::removeInvalidUtf16(const vector<uint16_t> &data) {

	// Return data with only valid UTF-16 parts
	return removeInvalidUtf16(u16string(data.begin(), data.end()));
}

u32string Unicode::removeInvalidUtf32(const u32string &text) {

	// Initialize return value
	u32string returnValue;

	// Go through all code points in the text
	for(char32_t codePoint : text)
	
		// Check if code point is valid
		if(codePoint <= UTF32_MAX_CODE_POINT)
		
			// Append code point to return value
			returnValue.push_back(codePoint);

	// Return return value
	return returnValue;
}

u32string Unicode::removeInvalidUtf32(const vector<uint32_t> &data) {

	// Return data with only valid UTF-32 parts
	return removeInvalidUtf32(u32string(data.begin(), data.end()));
}
