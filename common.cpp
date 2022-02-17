// Header files
#include <algorithm>
#include <cmath>
#include "common.h"
#include "openssl/sha.h"

using namespace std;


// Constants

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
