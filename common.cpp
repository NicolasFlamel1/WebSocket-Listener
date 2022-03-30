// Header files
#include <algorithm>
#include <cmath>
#include "common.h"
#include "openssl/sha.h"
#include "zlib.h"

using namespace std;


// Constants

// Microseconds in a millisecond
const int Common::MICROSECONDS_IN_A_MILLISECOND = 1000;

// Milliseconds in a second
const int Common::MILLISECONDS_IN_A_SECOND = 1000;

// Seconds in a minute
const int Common::SECONDS_IN_A_MINUTE = 60;

// Minutes in an hour
const int Common::MINUTES_IN_AN_HOUR = 60;

// Hours in a day
const int Common::HOURS_IN_A_DAY = 24;

// Days in a week
const int Common::DAYS_IN_A_WEEK = 7;

// Weeks in a year
const int Common::WEEKS_IN_A_YEAR = 52;

// Bits in a byte
const int Common::BITS_IN_A_BYTE = 8;

// Bytes in a kilobyte
const int Common::BYTES_IN_A_KILOBYTE = pow(2, 10);

// Kilobytes in a megabyte
const int Common::KILOBYTE_IN_A_MEGABYTE = Common::BYTES_IN_A_KILOBYTE;

// Deflate BFINAL flag
const uint8_t Common::DEFLATE_BFINAL_FLAG = 0x00;

// Chunk size
const size_t Common::CHUNK_SIZE = 1024;

// Window bits
const int Common::WINDOWS_BITS = MAX_WBITS;

// Deflate scalar
const int Common::DEFLATE_SCALAR = -1;

// Gzip flag
const int Common::GZIP_FLAG = 0x10;


// Supporting function implementation

// To lower case
string Common::toLowerCase(const string &text) {

	// Initialize return value
	string returnValue = text;
	
	// Convert return value to lower case
	transform(returnValue.begin(), returnValue.end(), returnValue.begin(), ::tolower);
	
	// Return return value
	return returnValue;
}

// Trim
string Common::trim(const string &text) {

	// Initialize return value
	string returnValue = text;

	// Remove whitespace from end of text
	returnValue.erase(returnValue.begin(), find_if(returnValue.begin(), returnValue.end(), [](int character) {
	
		// Return if character isn't whitespace
		return !isspace(character);
	}));
	
	// Remove whitespace from beginning of text
	returnValue.erase(find_if(returnValue.rbegin(), returnValue.rend(), [](int character) {
	
		// Return if character isn't whitespace
		return !isspace(character);
		
	}).base(), returnValue.end());
	
	// Return return value
	return returnValue;
}

// SHA1 hash
vector<uint8_t> Common::sha1Hash(const vector<uint8_t> &data) {

	// Initialize output
	uint8_t output[SHA_DIGEST_LENGTH];
	
	// Check if getting SHA1 hash of data failed
	if(!SHA1(reinterpret_cast<const unsigned char *>(data.data()), data.size(), output)) {
	
		// Throw exception
		throw runtime_error("Failed to hash data");
	}
	
	// Return output
	return {output, output + sizeof(output)};
}

// Is alphanumeric
bool Common::isAlphanumeric(const string &text) {

	// Check if text is empty
	if(text.empty()) {
	
		// Return false
		return false;
	}

	// Return if every character in the text is alphanumeric
	return all_of(text.begin(), text.end(), ::isalnum);
}

// Is numeric
bool Common::isNumeric(const string &text) {

	// Check if text is empty
	if(text.empty()) {
	
		// Return false
		return false;
	}

	// Return if every character in the text is a digit
	return all_of(text.begin(), text.end(), ::isdigit);
}

// Gzip
bool Common::gzip(vector<uint8_t> &output, const vector<uint8_t> &input) {

	// Return compressing input
	return compress(output, input, WINDOWS_BITS | GZIP_FLAG);
}

// Inflate
bool Common::inflate(vector<uint8_t> &output, const vector<uint8_t> &input) {

	// Return decompressing input
	return decompress(output, input, WINDOWS_BITS * DEFLATE_SCALAR);
}

// Deflate
bool Common::deflate(vector<uint8_t> &output, const vector<uint8_t> &input) {

	// Return compressing input
	return compress(output, input, WINDOWS_BITS * DEFLATE_SCALAR);
}

// Compress
bool Common::compress(vector<uint8_t> &output, const vector<uint8_t> &input, int windowBits) {

	// Check if initializing stream failed
	z_stream stream;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = input.size();
	stream.next_in = const_cast<uint8_t *>(input.data());
	
	if(deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, windowBits, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK) {
	
		// Return false
		return false;
	}
	
	// Go through all data
	int result;
	do {
	
		// Go through data in the current chunk
		do {
		
			// Set stream to deflate chunk
			uint8_t chunk[CHUNK_SIZE];
			
			stream.avail_out = sizeof(chunk);
			stream.next_out = chunk;
			
			// Check if error occured while deflating chunk
			result = ::deflate(&stream, Z_FINISH);
			
			if(result != Z_OK && result != Z_STREAM_END && result != Z_BUF_ERROR) {
			
				// End stream
				deflateEnd(&stream);
			
				// Return false
				return false;
			}
			
			// Append deflated chunk to output
			output.insert(output.end(), chunk, chunk + sizeof(chunk) - stream.avail_out);
		
		} while(!stream.avail_out);
		
	} while(result != Z_STREAM_END && result != Z_BUF_ERROR && stream.avail_in);
	
	// Check if ending stream failed
	if(deflateEnd(&stream) != Z_OK) {
	
		// Return false
		return false;
	}
	
	// Return true
	return true;
}

// Decompress
bool Common::decompress(vector<uint8_t> &output, const vector<uint8_t> &input, int windowBits) {

	// Check if initializing stream failed
	z_stream stream;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = input.size();
	stream.next_in = const_cast<uint8_t *>(input.data());
	
	if(inflateInit2(&stream, windowBits) != Z_OK) {
	
		// Return false
		return false;
	}
	
	// Go through all data
	int result;
	do {
	
		// Go through data in the current chunk
		do {
		
			// Set stream to inflate chunk
			uint8_t chunk[CHUNK_SIZE];
			
			stream.avail_out = sizeof(chunk);
			stream.next_out = chunk;
			
			// Check if error occured while inflating chunk
			result = ::inflate(&stream, Z_SYNC_FLUSH);
			
			if(result != Z_OK && result != Z_STREAM_END && result != Z_BUF_ERROR) {
			
				// End stream
				inflateEnd(&stream);
				
				// Return false
				return false;
			}
			
			// Append inflated chunk to output
			output.insert(output.end(), chunk, chunk + sizeof(chunk) - stream.avail_out);
		
		} while(!stream.avail_out);
	
	} while(result != Z_STREAM_END && result != Z_BUF_ERROR && stream.avail_in);
	
	// Check if ending stream failed
	if(inflateEnd(&stream) != Z_OK) {
	
		// Return false
		return false;
	}
	
	// Return true
	return true;
}
