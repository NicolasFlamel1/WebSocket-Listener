// Header guard
#ifndef JSON_H
#define JSON_H


// Header files
#include <memory>
#include <unordered_map>
#include <vector>

using namespace std;


// Classes

// JSON class
class Json final {

	// Public
	public:
		
		// Type
		enum class Type {
			NONE,
			STRING,
			NUMBER,
			OBJECT,
			ARRAY,
			BOOLEAN,
			NULL_VALUE
		};
		
		// Unlimited constant
		static const int8_t UNLIMITED = -1;
		
		// Type definitions
		typedef string String;
		typedef long double Number;
		typedef unordered_map<string, shared_ptr<Json>> Object;
		typedef vector<Json> Array;
		typedef bool Boolean;
		typedef void * Null;
	
		// Constructor
		Json();
		
		// Copy constructor
		Json(const Json &source);
		
		// Move constructor
		Json(Json &&source);
		
		// Conversion constructors
		Json(const char *value);
		Json(const String &value);
		Json(int8_t value);
		Json(int16_t value);
		Json(int32_t value);
		Json(intmax_t value);
		Json(uint8_t value);
		Json(uint16_t value);
		Json(uint32_t value);
		Json(uintmax_t value);
		Json(float value);
		Json(double value);
		Json(Number value);
		Json(const Object &value);
		Json(const Array &value);
		Json(Boolean value);
		
		// Assignment operator
		Json &operator=(const Json &source);
		Json &operator=(Json &&source);
		Json &operator=(const char *source);
		Json &operator=(const String &source);
		Json &operator=(int8_t source);
		Json &operator=(int16_t source);
		Json &operator=(int32_t source);
		Json &operator=(intmax_t source);
		Json &operator=(uint8_t source);
		Json &operator=(uint16_t source);
		Json &operator=(uint32_t source);
		Json &operator=(uintmax_t source);
		Json &operator=(float source);
		Json &operator=(double source);
		Json &operator=(Number source);
		Json &operator=(const Object &source);
		Json &operator=(const Array &source);
		Json &operator=(Boolean source);
		
		// Equality operator
		bool operator==(const Json &source) const;
		bool operator==(const char *source) const;
		bool operator==(const String &source) const;
		bool operator==(int8_t source) const;
		bool operator==(int16_t source) const;
		bool operator==(int32_t source) const;
		bool operator==(intmax_t source) const;
		bool operator==(uint8_t source) const;
		bool operator==(uint16_t source) const;
		bool operator==(uint32_t source) const;
		bool operator==(uintmax_t source) const;
		bool operator==(float source) const;
		bool operator==(double source) const;
		bool operator==(Number source) const;
		bool operator==(const Object &source) const;
		bool operator==(const Array &source) const;
		bool operator==(Boolean source) const;
		friend bool operator==(const char *operand, const Json &source);
		friend bool operator==(const String &operand, const Json &source);
		friend bool operator==(int8_t operand, const Json &source);
		friend bool operator==(int16_t operand, const Json &source);
		friend bool operator==(int32_t operand, const Json &source);
		friend bool operator==(intmax_t operand, const Json &source);
		friend bool operator==(uint8_t operand, const Json &source);
		friend bool operator==(uint16_t operand, const Json &source);
		friend bool operator==(uint32_t operand, const Json &source);
		friend bool operator==(uintmax_t operand, const Json &source);
		friend bool operator==(float operand, const Json &source);
		friend bool operator==(double operand, const Json &source);
		friend bool operator==(Number operand, const Json &source);
		friend bool operator==(const Object &operand, const Json &source);
		friend bool operator==(const Array &operand, const Json &source);
		friend bool operator==(Boolean operand, const Json &source);
		
		// Inequality operator
		bool operator!=(const Json &source) const;
		bool operator!=(const char *source) const;
		bool operator!=(const String &source) const;
		bool operator!=(int8_t source) const;
		bool operator!=(int16_t source) const;
		bool operator!=(int32_t source) const;
		bool operator!=(intmax_t source) const;
		bool operator!=(uint8_t source) const;
		bool operator!=(uint16_t source) const;
		bool operator!=(uint32_t source) const;
		bool operator!=(uintmax_t source) const;
		bool operator!=(float source) const;
		bool operator!=(double source) const;
		bool operator!=(Number source) const;
		bool operator!=(const Object &source) const;
		bool operator!=(const Array &source) const;
		bool operator!=(Boolean source) const;
		friend bool operator!=(const char *operand, const Json &source);
		friend bool operator!=(const String &operand, const Json &source);
		friend bool operator!=(int8_t operand, const Json &source);
		friend bool operator!=(int16_t operand, const Json &source);
		friend bool operator!=(int32_t operand, const Json &source);
		friend bool operator!=(intmax_t operand, const Json &source);
		friend bool operator!=(uint8_t operand, const Json &source);
		friend bool operator!=(uint16_t operand, const Json &source);
		friend bool operator!=(uint32_t operand, const Json &source);
		friend bool operator!=(uintmax_t operand, const Json &source);
		friend bool operator!=(float operand, const Json &source);
		friend bool operator!=(double operand, const Json &source);
		friend bool operator!=(Number operand, const Json &source);
		friend bool operator!=(const Object &operand, const Json &source);
		friend bool operator!=(const Array &operand, const Json &source);
		friend bool operator!=(Boolean operand, const Json &source);
		
		// Get type
		Type getType() const;
		
		// Get value
		const void *getValue() const;
		void *getValue();
		const String &getStringValue() const;
		String &getStringValue();
		Number getNumberValue() const;
		const Object &getObjectValue() const;
		Object &getObjectValue();
		const Array &getArrayValue() const;
		Array &getArrayValue();
		Boolean getBooleanValue() const;
		Null getNullValue() const;
		
		// Set value
		void setStringValue(const String &value);
		void setNumberValue(Number value);
		void setObjectValue(const Object &value);
		void setArrayValue(const Array &value);
		void setBooleanValue(Boolean value);
		void setNullValue();
		
		// Encode
		string encode() const;
		
		// Decode
		bool decode(const string &value, intmax_t maxDepth = UNLIMITED);
		
		// Clear
		void clear();
		
		// Empty
		bool empty() const;
		
		// Check if using JSON base64
		#ifdef JSON_BASE64
		
			// Base64 encode
			static string base64Encode(const vector<uint8_t> &value);

			// Base64 decode
			static vector<uint8_t> base64Decode(const string &value);
		#endif
	
	// Private
	private:
	
		// Set type
		void setType(Type value);
		
		// Remove whitespace
		static string removeWhitespace(const string &value);
		
		// Escape
		static string escape(const string &value);
		
		// Unescape
		static string unescape(const string &value);
		
		// Parse value
		bool parseValue(const string &value, intmax_t currentDepth, intmax_t maxDepth);
		
		// Compare
		bool compare(const Json *value) const;
		
		// Type
		Type type;
		
		// Value
		String stringValue;
		Number numberValue;
		Object objectValue;
		Array arrayValue;
		Boolean booleanValue;
};


#endif
