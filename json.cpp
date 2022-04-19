// Header files
#include <cmath>
#include <iomanip>
#ifdef JSON_BASE64
	#include <openssl/ssl.h>
#endif
#include <sstream>
#include "json.h"
#include "unicode.h"

using namespace std;


// Supporting function implementation
Json::Json() {

	// Clear
	clear();
}

Json::Json(const Json &source) {

	// Set self to source
	*this = source;
}

Json::Json(Json &&source) {

	// Set self to source
	*this = source;
}

Json::Json(const char *value) {

	// Check if value is NULL
	if(value == nullptr)
	
		// Set NULL value
		setNullValue();
	
	// Otherwise
	else
	
		// Set string value
		setStringValue(value);
}

Json::Json(const String &value) {

	// Set string value
	setStringValue(value);
}

Json::Json(int8_t value) {

	// Set number value
	setNumberValue(value);
}

Json::Json(int16_t value) {

	// Set number value
	setNumberValue(value);
}

Json::Json(int32_t value) {

	// Set number value
	setNumberValue(value);
}

Json::Json(intmax_t value) {

	// Set number value
	setNumberValue(value);
}

Json::Json(uint8_t value) {

	// Set number value
	setNumberValue(value);
}

Json::Json(uint16_t value) {

	// Set number value
	setNumberValue(value);
}

Json::Json(uint32_t value) {

	// Set number value
	setNumberValue(value);
}

Json::Json(uintmax_t value) {

	// Set number value
	setNumberValue(value);
}

Json::Json(float value) {

	// Set number value
	setNumberValue(value);
}

Json::Json(double value) {

	// Set number value
	setNumberValue(value);
}

Json::Json(Number value) {

	// Set number value
	setNumberValue(value);
}

Json::Json(const Object &value) {

	// Set object value
	setObjectValue(value);
}

Json::Json(const Array &value) {

	// Set array value
	setArrayValue(value);
}

Json::Json(Boolean value) {

	// Set boolean value
	setBooleanValue(value);
}

Json &Json::operator=(const Json &source) {

	// Check if source isn't itself
	if(this != &source) {
	
		// Clear
		clear();
	
		// Copy type
		type = source.type;
		
		// Copy values
		if(source.type == Type::STRING)
			stringValue = source.stringValue;
		
		else if(source.type == Type::NUMBER)
			numberValue = source.numberValue;
		
		else if(source.type == Type::OBJECT)
			objectValue = source.objectValue;
		
		else if(source.type == Type::ARRAY)
			arrayValue = source.arrayValue;
		
		else if(source.type == Type::BOOLEAN)
			booleanValue = source.booleanValue;
	}
	
	// Return self
	return *this;
}

Json &Json::operator=(Json &&source) {

	// Check if source isn't itself
	if(this != &source) {
	
		// Clear
		clear();
	
		// Move type
		type = source.type;
		
		// Move values
		if(source.type == Type::STRING)
			stringValue = move(source.stringValue);
		
		else if(source.type == Type::NUMBER)
			numberValue = source.numberValue;
		
		else if(source.type == Type::OBJECT)
			objectValue = move(source.objectValue);
		
		else if(source.type == Type::ARRAY)
			arrayValue = move(source.arrayValue);
		
		else if(source.type == Type::BOOLEAN)
			booleanValue = source.booleanValue;
		
		// Clear source's type
		source.type = Type::NONE;
	}
	
	// Return self
	return *this;
}

Json &Json::operator=(const char *source) {

	// Check if source is NULL
	if(source == nullptr)
	
		// Set NULL value
		setNullValue();
	
	// Otherwise
	else

		// Set string value to source
		setStringValue(source);
	
	// Return self
	return *this;
}

Json &Json::operator=(const String &source) {

	// Set string value to source
	setStringValue(source);
	
	// Return self
	return *this;
}

Json &Json::operator=(int8_t source) {

	// Set number value to source
	setNumberValue(source);
	
	// Return self
	return *this;
}

Json &Json::operator=(int16_t source) {

	// Set number value to source
	setNumberValue(source);
	
	// Return self
	return *this;
}

Json &Json::operator=(int32_t source) {

	// Set number value to source
	setNumberValue(source);
	
	// Return self
	return *this;
}

Json &Json::operator=(intmax_t source) {

	// Set number value to source
	setNumberValue(source);
	
	// Return self
	return *this;
}

Json &Json::operator=(uint8_t source) {

	// Set number value to source
	setNumberValue(source);
	
	// Return self
	return *this;
}

Json &Json::operator=(uint16_t source) {

	// Set number value to source
	setNumberValue(source);
	
	// Return self
	return *this;
}

Json &Json::operator=(uint32_t source) {

	// Set number value to source
	setNumberValue(source);
	
	// Return self
	return *this;
}

Json &Json::operator=(uintmax_t source) {

	// Set number value to source
	setNumberValue(source);
	
	// Return self
	return *this;
}

Json &Json::operator=(float source) {

	// Set number value to source
	setNumberValue(source);
	
	// Return self
	return *this;
}

Json &Json::operator=(double source) {

	// Set number value to source
	setNumberValue(source);
	
	// Return self
	return *this;
}

Json &Json::operator=(Number source) {

	// Set number value to source
	setNumberValue(source);
	
	// Return self
	return *this;
}

Json &Json::operator=(const Object &source) {

	// Set object value to source
	setObjectValue(source);
	
	// Return self
	return *this;
}

Json &Json::operator=(const Array &source) {

	// Set array value to source
	setArrayValue(source);
	
	// Return self
	return *this;
}

Json &Json::operator=(Boolean source) {

	// Set boolean value to source
	setBooleanValue(source);
	
	// Return self
	return *this;
}

bool Json::operator==(const Json &source) const {
	
	// Check if source isn't itself
	if(this != &source) {

		// Check if types aren't equal
		if(type != source.type)
	
			// Return false
			return false;
		
		// Check if type is a string
		if(type == Type::STRING)
		
			// Return if string values are equal
			return stringValue == source.stringValue;
		
		// Otherwise check if type is a number
		else if(type == Type::NUMBER)
		
			// Return if number values are equal
			return numberValue == source.numberValue;
		
		// Otherwise check if type is an object
		else if(type == Type::OBJECT) {
		
			// Check if object values have different sizes
			if(objectValue.size() != source.objectValue.size())
			
				// Return false
				return false;
			
			// Go through all key value pairs
			for(Object::const_iterator i = objectValue.cbegin(), j = source.objectValue.cbegin(); i != objectValue.cend() && j != source.objectValue.cend(); ++i, ++j) {
			
				// Check if keys differ or value's types differ
				if(i->first != j->first || i->second->getType() != j->second->getType())
				
					// Return false
					return false;
				
				// Check value's type
				switch(i->second->getType()) {
				
					// String type
					case Type::STRING:
					
						// Check if value's string values differ
						if(i->second->getStringValue() != j->second->getStringValue())
						
							// Return false
							return false;
						
						// Break
						break;
					
					// Number type
					case Type::NUMBER:
					
						// Check if value's number values differ
						if(i->second->getNumberValue() != j->second->getNumberValue())
						
							// Return false
							return false;
						
						// Break
						break;
					
					// Object type
					case Type::OBJECT:
					
						// Check if value's object values differ
						if(Json(i->second->getObjectValue()) != Json(j->second->getObjectValue()))
						
							// Return false
							return false;
						
						// Break
						break;
					
					// Array type
					case Type::ARRAY:
					
						// Check if value's array values differ
						if(Json(i->second->getArrayValue()) != Json(j->second->getArrayValue()))
						
							// Return false
							return false;
						
						// Break
						break;
					
					// Boolean type
					case Type::BOOLEAN:
					
						// Check if value's boolean values differ
						if(i->second->getBooleanValue() != j->second->getBooleanValue())
						
							// Return false
							return false;
						
						// Break
						break;
					
					// Default
					default:
					
						// Break
						break;
				}
			}
		}
		
		// Otherwise check if type is an array
		else if(type == Type::ARRAY) {
		
			// Check if array values have different sizes
			if(arrayValue.size() != source.arrayValue.size())
			
				// Return false
				return false;
			
			// Go through all values
			for(Array::const_iterator i = arrayValue.cbegin(), j = source.arrayValue.cbegin(); i != arrayValue.cend() && j != source.arrayValue.cend(); ++i, ++j) {
			
				// Check if value's types differ
				if(i->getType() != j->getType())
				
					// Return false
					return false;
				
				// Check value's type
				switch(i->getType()) {
				
					// String type
					case Type::STRING:
					
						// Check if value's string values differ
						if(i->getStringValue() != j->getStringValue())
						
							// Return false
							return false;
						
						// Break
						break;
					
					// Number type
					case Type::NUMBER:
					
						// Check if value's number values differ
						if(i->getNumberValue() != j->getNumberValue())
						
							// Return false
							return false;
						
						// Break
						break;
					
					// Object type
					case Type::OBJECT:
					
						// Check if value's object values differ
						if(Json(i->getObjectValue()) != Json(j->getObjectValue()))
						
							// Return false
							return false;
						
						// Break
						break;
					
					// Array type
					case Type::ARRAY:
					
						// Check if value's array values differ
						if(Json(i->getArrayValue()) != Json(j->getArrayValue()))
						
							// Return false
							return false;
						
						// Break
						break;
					
					// Boolean type
					case Type::BOOLEAN:
					
						// Check if value's boolean values differ
						if(i->getBooleanValue() != j->getBooleanValue())
						
							// Return false
							return false;
						
						// Break
						break;
					
					// Default
					default:
					
						// Break
						break;
				}
			}
		}
		
		// Otherwise check if type is a boolean
		else if(type == Type::BOOLEAN)
		
			// Return if boolean values are equal
			return booleanValue == source.booleanValue;
	}
	
	// Return true
	return true;
}

bool Json::operator==(const char *source) const {

	// Return if self is equal to source
	return *this == Json(source);
}

bool Json::operator==(const String &source) const {

	// Return if self is equal to source
	return *this == Json(source);
}

bool Json::operator==(int8_t source) const {

	// Return if self is equal to source
	return *this == Json(source);
}

bool Json::operator==(int16_t source) const {

	// Return if self is equal to source
	return *this == Json(source);
}

bool Json::operator==(int32_t source) const {

	// Return if self is equal to source
	return *this == Json(source);
}

bool Json::operator==(intmax_t source) const {

	// Return if self is equal to source
	return *this == Json(source);
}

bool Json::operator==(uint8_t source) const {

	// Return if self is equal to source
	return *this == Json(source);
}

bool Json::operator==(uint16_t source) const {

	// Return if self is equal to source
	return *this == Json(source);
}

bool Json::operator==(uint32_t source) const {

	// Return if self is equal to source
	return *this == Json(source);
}

bool Json::operator==(uintmax_t source) const {

	// Return if self is equal to source
	return *this == Json(source);
}

bool Json::operator==(float source) const {

	// Return if self is equal to source
	return *this == Json(source);
}

bool Json::operator==(double source) const {

	// Return if self is equal to source
	return *this == Json(source);
}

bool Json::operator==(Number source) const {

	// Return if self is equal to source
	return *this == Json(source);
}

bool Json::operator==(const Object &source) const {

	// Return if self is equal to source
	return *this == Json(source);
}

bool Json::operator==(const Array &source) const {

	// Return if self is equal to source
	return *this == Json(source);
}

bool Json::operator==(Boolean source) const {

	// Return if self is equal to source
	return *this == Json(source);
}

bool operator==(const char *operand, const Json &source) {

	// Return if operand is equal to source
	return Json(operand) == source;
}

bool operator==(const Json::String &operand, const Json &source) {

	// Return if operand is equal to source
	return Json(operand) == source;
}

bool operator==(int8_t operand, const Json &source) {

	// Return if operand is equal to source
	return Json(operand) == source;
}

bool operator==(int16_t operand, const Json &source) {

	// Return if operand is equal to source
	return Json(operand) == source;
}

bool operator==(int32_t operand, const Json &source) {

	// Return if operand is equal to source
	return Json(operand) == source;
}

bool operator==(intmax_t operand, const Json &source) {

	// Return if operand is equal to source
	return Json(operand) == source;
}

bool operator==(uint8_t operand, const Json &source) {

	// Return if operand is equal to source
	return Json(operand) == source;
}

bool operator==(uint16_t operand, const Json &source) {

	// Return if operand is equal to source
	return Json(operand) == source;
}

bool operator==(uint32_t operand, const Json &source) {

	// Return if operand is equal to source
	return Json(operand) == source;
}

bool operator==(uintmax_t operand, const Json &source) {

	// Return if operand is equal to source
	return Json(operand) == source;
}

bool operator==(float operand, const Json &source) {

	// Return if operand is equal to source
	return Json(operand) == source;
}

bool operator==(double operand, const Json &source) {

	// Return if operand is equal to source
	return Json(operand) == source;
}

bool operator==(Json::Number operand, const Json &source) {

	// Return if operand is equal to source
	return Json(operand) == source;
}

bool operator==(const Json::Object &operand, const Json &source) {

	// Return if operand is equal to source
	return Json(operand) == source;
}

bool operator==(const Json::Array &operand, const Json &source) {

	// Return if operand is equal to source
	return Json(operand) == source;
}

bool operator==(Json::Boolean operand, const Json &source) {

	// Return if operand is equal to source
	return Json(operand) == source;
}

bool Json::operator!=(const Json &source) const {

	// Return if self doesn't equal source
	return !(*this == source);
}

bool Json::operator!=(const char *source) const {

	// Return if self isn't equal to source
	return *this != Json(source);
}

bool Json::operator!=(const String &source) const {

	// Return if self isn't equal to source
	return *this != Json(source);
}

bool Json::operator!=(int8_t source) const {

	// Return if self isn't equal to source
	return *this != Json(source);
}

bool Json::operator!=(int16_t source) const {

	// Return if self isn't equal to source
	return *this != Json(source);
}

bool Json::operator!=(int32_t source) const {

	// Return if self isn't equal to source
	return *this != Json(source);
}

bool Json::operator!=(intmax_t source) const {

	// Return if self isn't equal to source
	return *this != Json(source);
}

bool Json::operator!=(uint8_t source) const {

	// Return if self isn't equal to source
	return *this != Json(source);
}

bool Json::operator!=(uint16_t source) const {

	// Return if self isn't equal to source
	return *this != Json(source);
}

bool Json::operator!=(uint32_t source) const {

	// Return if self isn't equal to source
	return *this != Json(source);
}

bool Json::operator!=(uintmax_t source) const {

	// Return if self isn't equal to source
	return *this != Json(source);
}

bool Json::operator!=(float source) const {

	// Return if self isn't equal to source
	return *this != Json(source);
}

bool Json::operator!=(double source) const {

	// Return if self isn't equal to source
	return *this != Json(source);
}

bool Json::operator!=(Number source) const {

	// Return if self isn't equal to source
	return *this != Json(source);
}

bool Json::operator!=(const Object &source) const {

	// Return if self isn't equal to source
	return *this != Json(source);
}

bool Json::operator!=(const Array &source) const {

	// Return if self isn't equal to source
	return *this != Json(source);
}

bool Json::operator!=(Boolean source) const {

	// Return if self isn't equal to source
	return *this != Json(source);
}

bool operator!=(const char *operand, const Json &source) {

	// Return if operand isn't equal to source
	return Json(operand) != source;
}

bool operator!=(const Json::String &operand, const Json &source) {

	// Return if operand isn't equal to source
	return Json(operand) != source;
}

bool operator!=(int8_t operand, const Json &source) {

	// Return if operand isn't equal to source
	return Json(operand) != source;
}

bool operator!=(int16_t operand, const Json &source) {

	// Return if operand isn't equal to source
	return Json(operand) != source;
}

bool operator!=(int32_t operand, const Json &source) {

	// Return if operand isn't equal to source
	return Json(operand) != source;
}

bool operator!=(intmax_t operand, const Json &source) {

	// Return if operand isn't equal to source
	return Json(operand) != source;
}

bool operator!=(uint8_t operand, const Json &source) {

	// Return if operand isn't equal to source
	return Json(operand) != source;
}

bool operator!=(uint16_t operand, const Json &source) {

	// Return if operand isn't equal to source
	return Json(operand) != source;
}

bool operator!=(uint32_t operand, const Json &source) {

	// Return if operand isn't equal to source
	return Json(operand) != source;
}

bool operator!=(uintmax_t operand, const Json &source) {

	// Return if operand isn't equal to source
	return Json(operand) != source;
}

bool operator!=(float operand, const Json &source) {

	// Return if operand isn't equal to source
	return Json(operand) != source;
}

bool operator!=(double operand, const Json &source) {

	// Return if operand isn't equal to source
	return Json(operand) != source;
}

bool operator!=(Json::Number operand, const Json &source) {

	// Return if operand isn't equal to source
	return Json(operand) != source;
}

bool operator!=(const Json::Object &operand, const Json &source) {

	// Return if operand isn't equal to source
	return Json(operand) != source;
}

bool operator!=(const Json::Array &operand, const Json &source) {

	// Return if operand isn't equal to source
	return Json(operand) != source;
}

bool operator!=(Json::Boolean operand, const Json &source) {

	// Return if operand isn't equal to source
	return Json(operand) != source;
}

Json::Type Json::getType() const {

	// Return type
	return type;
}

const void *Json::getValue() const {

	// Check type
	switch(type) {
	
		// String
		case Type::STRING:
		
			// Return string value
			return &stringValue;
		
		// Number
		case Type::NUMBER:
		
			// Return number value
			return &numberValue;
		
		// Object
		case Type::OBJECT:
		
			// Return object value
			return &objectValue;
		
		// Array
		case Type::ARRAY:
		
			// Return array value
			return &arrayValue;
		
		// Boolean
		case Type::BOOLEAN:
		
			// Return boolean value
			return &booleanValue;
		
		// NULL
		case Type::NULL_VALUE:
		
			// Return NULL value
			return nullptr;
		
		// None
		case Type::NONE:
		
			// Break
			break;
	}
	
	// Throw exception
	throw runtime_error("Value doesn't exist");
}

void *Json::getValue() {

	// Check type
	switch(type) {
	
		// String
		case Type::STRING:
		
			// Return string value
			return &stringValue;
		
		// Number
		case Type::NUMBER:
		
			// Return number value
			return &numberValue;
		
		// Object
		case Type::OBJECT:
		
			// Return object value
			return &objectValue;
		
		// Array
		case Type::ARRAY:
		
			// Return array value
			return &arrayValue;
		
		// Boolean
		case Type::BOOLEAN:
		
			// Return boolean value
			return &booleanValue;
		
		// NULL
		case Type::NULL_VALUE:
		
			// Return NULL value
			return nullptr;
		
		// None
		case Type::NONE:
		
			// Break
			break;
	}
	
	// Throw exception
	throw runtime_error("Value doesn't exist");
}

const Json::String &Json::getStringValue() const {

	// Check if type isn't a string
	if(type != Type::STRING)
	
		// Throw exception
		throw runtime_error("Value doesn't exist");

	// Return string value
	return stringValue;
}

Json::String &Json::getStringValue() {

	// Check if type isn't a string
	if(type != Type::STRING)
	
		// Throw exception
		throw runtime_error("Value doesn't exist");

	// Return string value
	return stringValue;
}

Json::Number Json::getNumberValue() const {

	// Check if type isn't a number
	if(type != Type::NUMBER)
	
		// Throw exception
		throw runtime_error("Value doesn't exist");

	// Return number value
	return numberValue;
}

const Json::Object &Json::getObjectValue() const {

	// Check if type isn't an object
	if(type != Type::OBJECT)
	
		// Throw exception
		throw runtime_error("Value doesn't exist");

	// Return object value
	return objectValue;
}

Json::Object &Json::getObjectValue() {

	// Check if type isn't an object
	if(type != Type::OBJECT)
	
		// Throw exception
		throw runtime_error("Value doesn't exist");

	// Return object value
	return objectValue;
}

const Json::Array &Json::getArrayValue() const {

	// Check if type isn't an array
	if(type != Type::ARRAY)
	
		// Throw exception
		throw runtime_error("Value doesn't exist");

	// Return array value
	return arrayValue;
}

Json::Array &Json::getArrayValue() {

	// Check if type isn't an array
	if(type != Type::ARRAY)
	
		// Throw exception
		throw runtime_error("Value doesn't exist");

	// Return array value
	return arrayValue;
}

Json::Boolean Json::getBooleanValue() const {

	// Check if type isn't a boolean
	if(type != Type::BOOLEAN)
	
		// Throw exception
		throw runtime_error("Value doesn't exist");

	// Return boolean value
	return booleanValue;
}

Json::Null Json::getNullValue() const {

	// Check if type isn't a NULL value
	if(type != Type::NULL_VALUE)
	
		// Throw exception
		throw runtime_error("Value doesn't exist");

	// Return NULL value
	return nullptr;
}

void Json::setStringValue(const String &value) {

	// Clear
	clear();
	
	// Check if escaped value isn't a valid UTF-8 string
	if(!Unicode::isValidUtf8(escape(value)))
	
		// Throw exception
		throw runtime_error("Invalid string");

	// Set type
	setType(Type::STRING);
	
	// Set string value
	stringValue = value;
}

void Json::setNumberValue(Number value) {

	// Clear
	clear();
	
	// Check if value isn't finite
	if(!isfinite(value))
	
		// Throw exception
		throw runtime_error("Invalid number");

	// Set type
	setType(Type::NUMBER);
	
	// Set number value
	numberValue = value;
}

void Json::setBooleanValue(Boolean value) {

	// Clear
	clear();

	// Set type
	setType(Type::BOOLEAN);
	
	// Set boolean value
	booleanValue = value;
}

void Json::setObjectValue(const Object &value) {

	// Clear
	clear();
	
	// Go through all key value pairs
	for(const Object::value_type &object : value)
	
		// Check if escaped key value isn't a valid UTF-8 string
		if(!Unicode::isValidUtf8(escape(object.first)))
		
			// Throw exception
			throw runtime_error("Invalid object");

	// Set type
	setType(Type::OBJECT);
	
	// Set object value
	objectValue = value;
}

void Json::setArrayValue(const Array &value) {

	// Clear
	clear();

	// Set type
	setType(Type::ARRAY);
	
	// Set array value
	arrayValue = value;
}

void Json::setNullValue() {

	// Clear
	clear();
	
	// Set type
	setType(Type::NULL_VALUE);
}

string Json::encode() const {

	// Initialize return value
	string returnValue;
	
	// Check type
	switch(type) {
	
		// String
		case Type::STRING:
		
			// Append string starting character to return value
			returnValue += '"';
			
			// Append string value to return value
			returnValue += escape(stringValue);
			
			// Append string ending character to return value
			returnValue += '"';
			
			// Break
			break;
		
		// Number
		case Type::NUMBER:

			{
				// Append number value without trailing zeros and decimal points to return value
				string formattedNumberValue = to_string(numberValue);
				formattedNumberValue.erase(formattedNumberValue.find_last_not_of('0') + 1);
				formattedNumberValue.erase(formattedNumberValue.find_last_not_of('.') + 1);
				returnValue += formattedNumberValue;
			}
			
			// Break
			break;
		
		// Object
		case Type::OBJECT:
		
			// Append object starting character to return value
			returnValue += '{';
			
			// Go through all pairs in the object value
			for(Object::const_iterator i = objectValue.cbegin(); i != objectValue.cend(); ++i) {
			
				// Append string starting character to return value
				returnValue += '"';
				
				// Append pair's key to return value
				returnValue += escape(i->first);
				
				// Append string ending character to return value
				returnValue += '"';
				
				// Append key value separator to return value
				returnValue += ':';
				
				// Append pair's encoded value to return value
				returnValue += i->second->encode();
				
				// Check if not at the last value
				if(next(i) != objectValue.cend())
				
					// Append value separator to return value
					returnValue += ',';
			}
			
			// Append object ending character to return value
			returnValue += '}';
			
			// Break
			break;
		
		// Array
		case Type::ARRAY:
		
			// Append array starting character to return value
			returnValue += '[';
			
			// Go through all JSON values in the array value
			for(Array::const_iterator i = arrayValue.cbegin(); i != arrayValue.cend(); ++i) {
				
				// Append encoded value to return value
				returnValue += i->encode();
				
				// Check if not at the last value
				if(next(i) != arrayValue.cend())
				
					// Append value separator to return value
					returnValue += ',';
			}
			
			// Append array ending character to return value
			returnValue += ']';
			
			// Break
			break;
		
		// Boolean
		case Type::BOOLEAN:
		
			// Append boolean value to return value
			returnValue += booleanValue ? "true" : "false";
			
			// Break
			break;
		
		// NULL
		case Type::NULL_VALUE:
		
			// Append NULL value to return value
			returnValue += "null";
			
			// Break
			break;
		
		// None
		case Type::NONE:
		
			// Break
			break;
	}
	
	// Return return value
	return returnValue;
}

bool Json::decode(const string &value, intmax_t maxDepth) {

	// Clear
	clear();

	// Remove whitespace from the value
	string standardValue;
	try {
		standardValue = removeWhitespace(value);
	}
	
	// Check if an exception occurred
	catch(const runtime_error &error) {
	
		// Return false
		return false;
	}
	
	// Check if value isn't a valid UTF-8 string
	if(!Unicode::isValidUtf8(standardValue))
	
		// Return false
		return false;
	
	// Check if parsing value failed
	if(!parseValue(standardValue, 0, maxDepth)) {
	
		// Clear
		clear();
	
		// Return false
		return false;
	}

	// Return true
	return true;
}

void Json::clear() {

	// Set type
	setType(Type::NONE);
	
	// Clear other values
	stringValue.clear();
	objectValue.clear();
	arrayValue.clear();
}

bool Json::empty() const {

	// Return if empty
	return type == Type::NONE;
}

// Check if using JSON base64
#ifdef JSON_BASE64

	string Json::base64Encode(const vector<uint8_t> &value) {

		// Check if value is empty
		if(value.empty())
		
			// Throw exception
			throw runtime_error("Failed to encode value");
		
		// Initialize output
		int outputSize = ((value.size() + ((value.size() % 3) ? (3 - value.size() % 3) : 0)) / 3) * 4;
		uint8_t output[outputSize];

		// Check if creating a base64 BIO failed
		BIO *base64Bio = BIO_new(BIO_f_base64());
		if(!base64Bio)
		
			// Throw exception
			throw runtime_error("Failed to encode value");
		
		// Check if creating a memory BIO failed
		BIO *memoryBio = BIO_new(BIO_s_mem());
		if(!memoryBio) {
		
			// Free memory
			BIO_free(base64Bio);
		
			// Throw exception
			throw runtime_error("Failed to encode value");
		}
		
		// Set flag
		BIO_set_flags(base64Bio, BIO_FLAGS_BASE64_NO_NL);
		
		// Append the memory BIO to the base64 BIOS
		BIO_push(base64Bio, memoryBio);
		
		// Check if encoding value failed, flushing the buffer failed, or moving value to output failed
		int bytesRead;
		if(BIO_write(base64Bio, value.data(), value.size()) <= 0 || BIO_flush(base64Bio) != 1 || (bytesRead = BIO_read(memoryBio, output, outputSize)) <= 0) {
		
			// Free memory
			BIO_free(memoryBio);
			BIO_free(base64Bio);
		
			// Throw exception
			throw runtime_error("Failed to encode value");
		}
		
		// Check if freeing memory failed
		if(!BIO_free(memoryBio)) {
		
			// Free memory
			BIO_free(base64Bio);
			
			// Throw exception
			throw runtime_error("Failed to encode value");
		}
		
		// Check if freeing memory failed
		if(!BIO_free(base64Bio))
			
			// Throw exception
			throw runtime_error("Failed to encode value");
		
		// Return output
		return {output, output + bytesRead};
	}

	vector<uint8_t> Json::base64Decode(const string &value) {

		// Check if value is empty
		if(value.empty())
		
			// Throw exception
			throw runtime_error("Failed to decode value");
		
		// Go through all characters in value
		for(char character : value)
		
			// Check if character isn't valid base64
			if(!isalnum(character) && character != '+' && character != '/' && character != '=')
				
				// Throw exception
				throw runtime_error("Failed to decode value");
		
		// Initialize output
		int outputSize = value.size() / 4 * 3 - ((value.back() == '=') ? ((value.size() >= 2 && value[value.size() - 2] == '=') ? 2 : 1) : 0);
		uint8_t output[outputSize];

		// Check if creating a base64 BIO failed
		BIO *base64Bio = BIO_new(BIO_f_base64());
		if(!base64Bio)
		
			// Throw exception
			throw runtime_error("Failed to decode value");
		
		// Check if creating a memory BIO failed
		BIO *memoryBio = BIO_new(BIO_s_mem());
		if(!memoryBio) {
		
			// Free memory
			BIO_free(base64Bio);
		
			// Throw exception
			throw runtime_error("Failed to decode value");
		}
		
		// Set flag
		BIO_set_flags(base64Bio, BIO_FLAGS_BASE64_NO_NL);
		
		// Append the memory BIO to the base64 BIOS
		BIO_push(base64Bio, memoryBio);
		
		// Check if decoding value failed, flushing the buffer failed, or moving value to output failed
		int bytesRead;
		if(BIO_write(memoryBio, value.data(), value.size()) <= 0 || BIO_flush(base64Bio) != 1 || (bytesRead = BIO_read(base64Bio, output, outputSize)) <= 0) {
		
			// Free memory
			BIO_free(memoryBio);
			BIO_free(base64Bio);
		
			// Throw exception
			throw runtime_error("Failed to decode value");
		}
		
		// Check if freeing memory failed
		if(!BIO_free(memoryBio)) {
		
			// Free memory
			BIO_free(base64Bio);
			
			// Throw exception
			throw runtime_error("Failed to decode value");
		}
		
		// Check if freeing memory failed
		if(!BIO_free(base64Bio))
			
			// Throw exception
			throw runtime_error("Failed to decode value");
		
		// Return output
		return {output, output + bytesRead};
	}
#endif

void Json::setType(Type value) {

	// Set type
	type = value;
}

string Json::removeWhitespace(const string &value) {

	// Initialize variables
	string returnValue;
	bool insideString = false;
	char lastCharacter = '\0';
	bool ignoreEscapeSequence = false;
	
	// Go through all characters
	for(string::size_type i = 0; i < value.length(); ++i) {
	
		// Set character
		const char &character = value[i];
		
		// Check if inside a string
		if(insideString) {

			// Check if at the end of a string
			if((ignoreEscapeSequence || lastCharacter != '\\') && character == '"')

				// Clear inside string
				insideString = false;
		}

		// Otherwise check if at the start of a string
		else if((ignoreEscapeSequence || lastCharacter != '\\') && character == '"')

			// Set inside string
			insideString = true;

		// Check if not inside a string
		if(!insideString) {

			// Check if character isn't a space, tab, newline, or carriage return
			if(character != ' ' && character != '\t' && character != '\n' && character != '\r')

				// Append character to return value
				returnValue.push_back(character);
		}

		// Otherwise
		else

			// Append character to return value
			returnValue.push_back(character);
		
		// Check if last character is a backslash
		if(!ignoreEscapeSequence && lastCharacter == '\\') {
		
			// Check if character is an escaped backslash
			if(character == '\\')
			
				// Set to ignore escape sequence caused by this backslash
				ignoreEscapeSequence = true;
		}
		
		// Otherwise check if ignoring escape sequence
		else if(ignoreEscapeSequence)
			
			// Clear ignore escape sequence
			ignoreEscapeSequence = false;
	
		// Set last character
		lastCharacter = character;
	}
	
	// Check if value contains an unterminated string
	if(insideString)
	
		// Throw exception
		throw runtime_error("Invalid string");
	
	// Return return value
	return returnValue;
}

string Json::escape(const string &value) {

	// Initialize variables
	string returnValue;
	
	// Go through all characters
	for(char character : value)
	
		// Check character
		switch(character) {
		
			// Double quote
			case '"':
			
				// Append escaped character to return value
				returnValue += "\\\"";
				
				// Break
				break;
			
			// Backslash
			case '\\':
			
				// Append escaped character to return value
				returnValue += "\\\\";
				
				// Break
				break;
			
			// Forward slash
			case '/':
			
				// Append escaped character to return value
				returnValue += "\\/";
				
				// Break
				break;
			
			// Backspace
			case '\b':
			
				// Append escaped character to return value
				returnValue += "\\b";
				
				// Break
				break;
			
			// Form feed
			case '\f':
			
				// Append escaped character to return value
				returnValue += "\\f";
				
				// Break
				break;
			
			// Newline
			case '\n':
			
				// Append escaped character to return value
				returnValue += "\\n";
				
				// Break
				break;
			
			// Carriage return
			case '\r':
			
				// Append escaped character to return value
				returnValue += "\\r";
				
				// Break
				break;
			
			// Tab
			case '\t':
			
				// Append escaped character to return value
				returnValue += "\\t";
				
				// Break
				break;
			
			// Other control characters
			case '\0':
			case '\x1':
			case '\x2':
			case '\x3':
			case '\x4':
			case '\x5':
			case '\x6':
			case '\x7':
			case '\xB':
			case '\xE':
			case '\xF':
			case '\x10':
			case '\x11':
			case '\x12':
			case '\x13':
			case '\x14':
			case '\x15':
			case '\x16':
			case '\x17':
			case '\x18':
			case '\x19':
			case '\x1A':
			case '\x1B':
			case '\x1C':
			case '\x1D':
			case '\x1E':
			case '\x1F':
			
				{
			
					// Append escaped character to return value
					stringstream escapedCharacter;
					escapedCharacter << hex << uppercase << setfill('0') << setw(4) << static_cast<uint16_t>(character);
					returnValue += "\\u" + escapedCharacter.str();
				}
				
				// Break
				break;
			
			// Default
			default:
			
				// Append escaped character to return value
				returnValue += character;
				
				// Break
				break;
		}
	
	// Return return value
	return returnValue;
}

string Json::unescape(const string &value) {

	// Initialize variables
	string returnValue;
	char lastCharacter = '\0';
	bool ignoreEscapeSequence = false;
	
	// Go through all characters
	for(string::size_type i = 0; i < value.length(); ++i) {
	
		// Set character
		const char &character = value[i];
		
		// Check if last character is a backslash
		if(!ignoreEscapeSequence && lastCharacter == '\\') {
		
			// Check character
			switch(character) {
			
				// Double quote, backslash, or forward slash
				case '"':
				case '\\':
				case '/':
				
					// Append unescaped character to return value
					returnValue += character;
					
					// Break
					break;
				
				// Backspace
				case 'b':
				
					// Append unescaped character to return value
					returnValue += '\b';
					
					// Break
					break;
				
				// Form feed
				case 'f':
				
					// Append unescaped character to return value
					returnValue += '\f';
					
					// Break
					break;
				
				// Newline
				case 'n':
				
					// Append unescaped character to return value
					returnValue += '\n';
					
					// Break
					break;
				
				// Carriage return
				case 'r':
				
					// Append unescaped character to return value
					returnValue += '\r';
					
					// Break
					break;
				
				// Tab
				case 't':
				
					// Append unescaped character to return value
					returnValue += '\t';
					
					// Break
					break;
				
				// Escaped character
				case 'u':
				
					{
					
						// Increment index
						++i;
						
						// Initialie UTF-16 string
						u16string utf16String;
						
						// Go through both potential surrogate pair values
						for(uint8_t j = 0; j < 2; ++j) {
							
							// Check if the escape character has an invalid length
							if(value.length() - i < sizeof("FFFF") - 1)
							
								// Throw exception
								throw runtime_error("Invalid escaped character");
							
							// Go through all characters in the value
							for(string::size_type k = i; k < i + sizeof("FFFF") - 1; ++k)
							
								// Check if character isn't a hexadecimal character
								if(!isxdigit(value[k]))
								
									// Throw exception
									throw runtime_error("Invalid escaped character");
							
							// Get UTF-16 code point
							char16_t utf16Character = stoi(value.substr(i, sizeof("FFFF") - 1), nullptr, 16);
							
							// Check if UTF-16 code point is a single unpaired surrogate
							if(j == 0 && utf16Character >= Unicode::UTF16_LOW_SURROGATE_RANGE_BEGIN && utf16Character <= Unicode::UTF16_LOW_SURROGATE_RANGE_END)
							
								// Throw exception
								throw runtime_error("Invalid escaped character");
							
							// Check if previous UTF-16 code point is a single unpaired surrogate
							if(j == 1 && (utf16Character < Unicode::UTF16_LOW_SURROGATE_RANGE_BEGIN || utf16Character > Unicode::UTF16_LOW_SURROGATE_RANGE_END))
							
								// Throw exception
								throw runtime_error("Invalid escaped character");
							
							// Append code point to UTF-16 string
							utf16String += utf16Character;
							
							// Increment index
							i += sizeof("FFFF") - 1;
							
							// Check if code point is part of a surrogate pair
							if(j == 0 && utf16Character >= Unicode::UTF16_HIGH_SURROGATE_RANGE_BEGIN && utf16Character <= Unicode::UTF16_HIGH_SURROGATE_RANGE_END && value.length() - i >= sizeof("\\u") - 1 && value[i] == '\\' && value[i + 1] == 'u')
							
								// Increment index
								i += sizeof("\\u") - 1;
							
							// Otherwise
							else {
							
								// Check if UTF-16 code point is a single unpaired surrogate
								if(j == 0 && utf16Character >= Unicode::UTF16_HIGH_SURROGATE_RANGE_BEGIN && utf16Character <= Unicode::UTF16_HIGH_SURROGATE_RANGE_END)
								
									// Throw exception
									throw runtime_error("Invalid escaped character");
							
								// Break
								break;
							}
						}
						
						// Decrement index
						--i;
						
						// Append code point to return value
						returnValue += Unicode::utf16ToUtf8(utf16String);
					}
					
					// Break
					break;
			}
		}
		
		// Otherwise Check if character isn't a backslash
		else if(character != '\\')
		
			// Append character to return value
			returnValue += character;
		
		// Check if last character is a backslash
		if(!ignoreEscapeSequence && lastCharacter == '\\') {
		
			// Check if character is an escaped backslash
			if(character == '\\')
			
				// Set to ignore escape sequence caused by this backslash
				ignoreEscapeSequence = true;
		}
		
		// Otherwise check if ignoring escape sequence
		else if(ignoreEscapeSequence)
			
			// Clear ignore escape sequence
			ignoreEscapeSequence = false;
		
		// Set last character
		lastCharacter = value[i];
	}
	
	// Return return value
	return returnValue;
}

bool Json::parseValue(const string &value, intmax_t currentDepth, intmax_t maxDepth) {

	// Check if value is empty
	if(value.empty())
	
		// Return false
		return false;
	
	// Check if character starts an object
	if(value[0] == '{' && value.length() >= sizeof("{}") - 1 && value.back() == '}') {
	
		// Set object value
		setObjectValue({});
		
		// Initialize variables
		bool insideString = false;
		char lastCharacter = '\0';
		bool ignoreEscapeSequence = false;
		intmax_t arrayDepth = 0, objectDepth = 0;
		string key;
		
		// Go through all characters
		for(string::size_type i = 1, j = i; i < value.length(); ++i) {
		
			// Set character
			const char &character = value[i];
			
			// Check if inside a string
			if(insideString) {

				// Check if at the end of a string
				if((ignoreEscapeSequence || lastCharacter != '\\') && character == '"')

					// Clear inside string
					insideString = false;
			}

			// Otherwise check if at the start of a string
			else if((ignoreEscapeSequence || lastCharacter != '\\') && character == '"')

				// Set inside string
				insideString = true;

			// Check if not inside a string
			if(!insideString) {
			
				// Check if at the start of a contained array
				if(character == '[') {
				
					// Increment array depth
					++arrayDepth;
					
					// Check if array depth overflowed
					if(arrayDepth == 0)
					
						// Return false
						return false;
				}
				
				// Check if at the end of a contained array
				else if(character == ']') {
				
					// Decrement array depth
					--arrayDepth;
					
					// Check if a nonexistent array was closed
					if(arrayDepth < 0)
					
						// Return false
						return false;
				}
				
				// Otherwise check if at the start of a contained object
				else if(character == '{') {
				
					// Increment object depth
					++objectDepth;
					
					// Check if object depth overflowed
					if(objectDepth == 0)
					
						// Return false
						return false;
				}
				
				// Check if at the end of a contained object
				else if(character == '}' && i != value.length() - 1) {
				
					// Decrement object depth
					--objectDepth;
					
					// Check if a nonexistent object was closed
					if(objectDepth < 0)
					
						// Return false
						return false;
				}
			
				// Otherwise check if not inside a contained array or object
				else if(arrayDepth == 0 && objectDepth == 0) {
				
					// Check if at the end of a key
					if(character == ':') {
					
						// Check if value is empty and a value preceded it
						if(lastCharacter == ',')
						
							// Return false
							return false;
						
						// Check if preceded by a key
						if(!key.empty())
						
							// Return false
							return false;
						
						// Check if the key is empty
						if(i == j)
						
							// Return false;
							return false;
						
						// Check if key isn't a string
						if(value[j] != '"' || value[i - 1] != '"')
						
							// Return false
							return false;
						
						// Try setting key
						try {
							key = unescape(value.substr(j + sizeof('"'), i - j - sizeof('"') * 2));
						}
						
						// Check if an exception occurred
						catch(const runtime_error &error) {
						
							// Return false
							return false;
						}
						
						// Increment start of value
						j = i + 1;
					}
				
					// Otherwise check if at the end of a value
					else if(character == ',' || character == '}') {
					
						// Check if value is empty and a value preceded it
						if(lastCharacter == ',')
						
							// Return false
							return false;
						
						// Check if not preceded by a key or value is empty when it shouldn't be
						if((key.empty() || i == j) && j != value.length() - 1)
						
							// Return false
							return false;
						
						// Check if at the max depth
						if(currentDepth == maxDepth && maxDepth != UNLIMITED)

							// Return false
							return false;
						
						// Check if value is empty
						if(i != j) {
						
							// Check if appending value to object value failed
							objectValue.emplace(key, make_unique<Json>());
							if(!objectValue.at(key)->parseValue(value.substr(j, i - j), currentDepth + 1, maxDepth))
							
								// Return false
								return false;
							
							// Increment start of value
							j = i + 1;
						}
						
						// Clear key
						key.clear();
					}
				}
			}
			
			// Check if last character is a backslash
			if(!ignoreEscapeSequence && lastCharacter == '\\') {
			
				// Check if character is an escaped backslash
				if(character == '\\')
				
					// Set to ignore escape sequence caused by this backslash
					ignoreEscapeSequence = true;
			}
			
			// Otherwise check if ignoring escape sequence
			else if(ignoreEscapeSequence)
				
				// Clear ignore escape sequence
				ignoreEscapeSequence = false;
		
			// Set last character
			lastCharacter = character;
		}
		
		// Check if a contained array or object wasn't closed
		if(arrayDepth != 0 || objectDepth != 0)
		
			// Return false
			return false;
	}
	
	// Otherwise check if character starts an array
	else if(value[0] == '[' && value.length() >= sizeof("[]") - 1 && value.back() == ']') {
	
		// Set array value
		setArrayValue({});
		
		// Initialize variables
		bool insideString = false;
		char lastCharacter = '\0';
		bool ignoreEscapeSequence = false;
		intmax_t arrayDepth = 0, objectDepth = 0;
		
		// Go through all characters
		for(string::size_type i = 1, j = i; i < value.length(); ++i) {
		
			// Set character
			const char &character = value[i];
			
			// Check if inside a string
			if(insideString) {

				// Check if at the end of a string
				if((ignoreEscapeSequence || lastCharacter != '\\') && character == '"')

					// Clear inside string
					insideString = false;
			}

			// Otherwise check if at the start of a string
			else if((ignoreEscapeSequence || lastCharacter != '\\') && character == '"')

				// Set inside string
				insideString = true;

			// Check if not inside a string
			if(!insideString) {
			
				// Check if at the start of a contained array
				if(character == '[') {
				
					// Increment array depth
					++arrayDepth;
					
					// Check if array depth overflowed
					if(arrayDepth == 0)
					
						// Return false
						return false;
				}
				
				// Check if at the end of a contained array
				else if(character == ']' && i != value.length() - 1) {
				
					// Decrement array depth
					--arrayDepth;
					
					// Check if a nonexistent array was closed
					if(arrayDepth < 0)
					
						// Return false
						return false;
				}
				
				// Otherwise check if at the start of a contained object
				else if(character == '{') {
				
					// Increment object depth
					++objectDepth;
					
					// Check if object depth overflowed
					if(objectDepth == 0)
					
						// Return false
						return false;
				}
				
				// Check if at the end of a contained object
				else if(character == '}') {
				
					// Decrement object depth
					--objectDepth;
					
					// Check if a nonexistent object was closed
					if(objectDepth < 0)
					
						// Return false
						return false;
				}
			
				// Otherwise check if not inside a contained array or object
				else if(arrayDepth == 0 && objectDepth == 0) {
				
					// Check if at the end of a value
					if(character == ',' || character == ']') {
					
						// Check if value is empty and a value preceded it
						if(lastCharacter == ',')
						
							// Return false
							return false;
						
						// Check if value is empty when it shouldn't be
						if(i == j && j != value.length() - 1)
						
							// Return false
							return false;
						
						// Check if at the max depth
						if(currentDepth == maxDepth && maxDepth != UNLIMITED)

							// Return false
							return false;
						
						// Check if value is empty
						if(i != j) {
						
							// Check if appending value to array value failed
							arrayValue.emplace_back();
							if(!arrayValue.back().parseValue(value.substr(j, i - j), currentDepth + 1, maxDepth))
							
								// Return false
								return false;
							
							// Increment start of value
							j = i + 1;
						}
					}
				}
			}
			
			// Check if last character is a backslash
			if(!ignoreEscapeSequence && lastCharacter == '\\') {
			
				// Check if character is an escaped backslash
				if(character == '\\')
				
					// Set to ignore escape sequence caused by this backslash
					ignoreEscapeSequence = true;
			}
			
			// Otherwise check if ignoring escape sequence
			else if(ignoreEscapeSequence)
				
				// Clear ignore escape sequence
				ignoreEscapeSequence = false;
		
			// Set last character
			lastCharacter = character;
		}
		
		// Check if a contained array or object wasn't closed
		if(arrayDepth != 0 || objectDepth != 0)
		
			// Return false
			return false;
	}
	
	// Otherwise check if character starts a string
	else if(value[0] == '"' && value.length() >= sizeof("\"\"") - 1 && value.back() == '"') {
		
		// Initialize variables
		bool insideString = false;
		char lastCharacter = '\0';
		bool ignoreEscapeSequence = false;
		
		// Go through all characters
		for(string::size_type i = 0; i < value.length(); ++i) {
		
			// Set character
			const char &character = value[i];
			
			// Check if inside a string
			if(insideString) {

				// Check if at the end of a string
				if((ignoreEscapeSequence || lastCharacter != '\\') && character == '"')

					// Clear inside string
					insideString = false;
			}

			// Otherwise check if at the start of a string
			else if((ignoreEscapeSequence || lastCharacter != '\\') && character == '"')

				// Set inside string
				insideString = true;

			// Check if not inside a string and not at the last character
			if(!insideString && i != value.length() - 1)
			
				// Return false
				return false;
			
			// Check if last character is a backslash
			if(!ignoreEscapeSequence && lastCharacter == '\\') {
			
				// Check if character is an escaped backslash
				if(character == '\\')
				
					// Set to ignore escape sequence caused by this backslash
					ignoreEscapeSequence = true;
			}
			
			// Otherwise check if ignoring escape sequence
			else if(ignoreEscapeSequence)
				
				// Clear ignore escape sequence
				ignoreEscapeSequence = false;
		
			// Set last character
			lastCharacter = character;
		}
		
		// Try setting string value
		try {
			setStringValue(unescape(value.substr(sizeof('"'), value.length() - 1 - sizeof('"'))));
		}
		
		// Check if an exception occurred
		catch(const runtime_error &error) {
		
			// Return false
			return false;
		}
	}
	
	// Otherwise check if character starts a number
	else if(isdigit(value[0]) || (value.length() > 1 && value[0] == '-' && isdigit(value[1]))) {
		
		// Check if number has leading zeros
		if((value.length() > 1 && value[0] == '0' && value[1] != '.' && value[1] != 'e' && value[1] != 'E') || (value.length() > 2 && value[0] == '-' && value[1] == '0' && value[2] != '.' && value[2] != 'e' && value[2] != 'E'))
		
			// Return false
			return false;
		
		// Check if number contains a period not followed by a digit
		string::size_type i = value.find('.');
		if(i != string::npos && (i == value.length() - 1 || !isdigit(value[i + 1])))
			
			// Return false
			return false;
		
		// Try setting number value
		string::size_type offset;
		try {
			setNumberValue(stold(value, &offset));
		}
		
		// Check if an exception occurred
		catch(const runtime_error &error) {
		
			// Return false
			return false;
		}
		
		// Check if the value consists of more than just the number
		if(offset != value.length())
		
			// Return false
			return false;
		
		// Check if number isn't finite
		if(!isfinite(numberValue))
		
			// Return false
			return false;
	}
	
	// Otherwise check if character starts a NULL value
	else if(value == "null")
	
		// Set NULL value
		setNullValue();
	
	// Otherwise check if character starts a boolean value
	else if(value == "true" || value == "false")
	
		// Set boolean value
		setBooleanValue(value == "true");
	
	// Otherwise
	else
	
		// Return false
		return false;

	// Return true;
	return true;
}
