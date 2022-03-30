// Header guard
#ifndef COMMON_H
#define COMMON_H


// Header files
#include <string>
#include <vector>

using namespace std;


// Definitions

// To string
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)


// Classes

// Common class
class Common final {

	// Public
	public:
	
		// Constructor
		Common() = delete;
	
		// To lower case
		static string toLowerCase(const string &text);
		
		// Trim
		static string trim(const string &text);
		
		// SHA1 hash
		static vector<uint8_t> sha1Hash(const vector<uint8_t> &data);
		
		// Is alphanumeric
		static bool isAlphanumeric(const string &text);
		
		// Is numeric
		static bool isNumeric(const string &text);
		
		// Gzip
		static bool gzip(vector<uint8_t> &output, const vector<uint8_t> &input);
		
		// Inflate
		static bool inflate(vector<uint8_t> &output, const vector<uint8_t> &input);
		
		// Deflate
		static bool deflate(vector<uint8_t> &output, const vector<uint8_t> &input);
		
		// Microseconds in a millisecond
		static const int MICROSECONDS_IN_A_MILLISECOND;
		
		// Milliseconds in a second
		static const int MILLISECONDS_IN_A_SECOND;

		// Seconds in a minute
		static const int SECONDS_IN_A_MINUTE;

		// Minutes in an hour
		static const int MINUTES_IN_AN_HOUR;

		// Hours in a day
		static const int HOURS_IN_A_DAY;

		// Days in a week
		static const int DAYS_IN_A_WEEK;

		// Weeks in a year
		static const int WEEKS_IN_A_YEAR;
		
		// Bits in a byte
		static const int BITS_IN_A_BYTE;
		
		// Bytes in a kilobyte
		static const int BYTES_IN_A_KILOBYTE;
		
		// Kilobytes in a megabyte
		static const int KILOBYTE_IN_A_MEGABYTE;
		
		// Deflate BFINAL flag
		static const uint8_t DEFLATE_BFINAL_FLAG;
	
	// Private
	private:
	
		// Compress
		static bool compress(vector<uint8_t> &output, const vector<uint8_t> &input, int windowBits);
		
		// Decompress
		static bool decompress(vector<uint8_t> &output, const vector<uint8_t> &input, int windowBits);
	
		// Chunk size
		static const size_t CHUNK_SIZE;

		// Window bits
		static const int WINDOWS_BITS;

		// Deflate scalar
		static const int DEFLATE_SCALAR;

		// Gzip flag
		static const int GZIP_FLAG;
		
		// Maximum decompress size
		static const size_t MAXIMUM_DECOMPRESS_SIZE;
};


#endif
