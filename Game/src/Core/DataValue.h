#ifndef DATAVALUE_H
#define DATAVALUE_H

/* 
* This class is based on the cocos2d::Value class.
* https://github.com/cocos2d/cocos2d-x/blob/001614a9c57e4c62b63d4858de91bb7c728c81a6/cocos/base/CCValue.h
*/
class DataValue
{
public:
    /** A predefined Value that has not value. */
    static const DataValue Null;

    /** Default constructor. */
    DataValue();
    
    /** Create a Value by an unsigned char value. */
    explicit DataValue(unsigned char v);
    
    /** Create a Value by an integer value. */
    explicit DataValue(int v);
    
    /** Create a Value by a float value. */
    explicit DataValue(float v);
    
    /** Create a Value by a double value. */
    explicit DataValue(double v);
    
    /** Create a Value by a bool value. */
    explicit DataValue(bool v);
    
    /** Create a Value by a char pointer. It will copy the chars internally. */
    explicit DataValue(const char* v);
    
    /** Create a Value by a string. */
    explicit DataValue(const std::string& v);
    
    /** Create a Value by a ValueVector object. */
    explicit DataValue(const std::vector<DataValue>& v);
    /** Create a Value by a ValueVector object. It will use std::move internally. */
    explicit DataValue(std::vector<DataValue>&& v);
    
    /** Create a Value by a ValueMap object. */
    explicit DataValue(const std::map<std::string, DataValue>& v);
    /** Create a Value by a ValueMap object. It will use std::move internally. */
    explicit DataValue(std::map<std::string, DataValue>&& v);

    /** Create a Value by anthoer Value object. */
    DataValue(const DataValue& other);
    /** Create a Value by a Value object. It will use std::move internally. */
    DataValue(DataValue&& other);
    
    /** Destructor. */
    ~DataValue();

    /** Assignment operator, assign from Value to Value. */
    DataValue& operator= (const DataValue& other);
    /** Assignment operator, assign from Value to Value. It will use std::move internally. */
    DataValue& operator= (DataValue&& other);

    /** Assignment operator, assign from unsigned char to Value. */
    DataValue& operator= (unsigned char v);
    /** Assignment operator, assign from integer to Value. */
    DataValue& operator= (int v);
    /** Assignment operator, assign from float to Value. */
    DataValue& operator= (float v);
    /** Assignment operator, assign from double to Value. */
    DataValue& operator= (double v);
    /** Assignment operator, assign from bool to Value. */
    DataValue& operator= (bool v);
    /** Assignment operator, assign from char* to Value. */
    DataValue& operator= (const char* v);
    /** Assignment operator, assign from string to Value. */
    DataValue& operator= (const std::string& v);

    /** Assignment operator, assign from ValueVector to Value. */
    DataValue& operator= (const std::vector<DataValue>& v);
    /** Assignment operator, assign from ValueVector to Value. */
    DataValue& operator= (std::vector<DataValue>&& v);

    /** Assignment operator, assign from ValueMap to Value. */
    DataValue& operator= (const std::map<std::string, DataValue>& v);
    /** Assignment operator, assign from ValueMap to Value. It will use std::move internally. */
    DataValue& operator= (std::map<std::string, DataValue>&& v);

    /** != operator overloading */
    bool operator!= (const DataValue& v);
    /** != operator overloading */
    bool operator!= (const DataValue& v) const;
    /** == operator overloading */
    bool operator== (const DataValue& v);
    /** == operator overloading */
    bool operator== (const DataValue& v) const;

    /** Gets as a byte value. Will convert to unsigned char if possible, or will trigger assert error. */
    unsigned char asByte() const;
    /** Gets as an integer value. Will convert to integer if possible, or will trigger assert error. */
    int asInt() const;
    /** Gets as a float value. Will convert to float if possible, or will trigger assert error. */
    float asFloat() const;
    /** Gets as a double value. Will convert to double if possible, or will trigger assert error. */
    double asDouble() const;
    /** Gets as a bool value. Will convert to bool if possible, or will trigger assert error. */
    bool asBool() const;
    /** Gets as a string value. Will convert to string if possible, or will trigger assert error. */
    std::string asString() const;

    /** Gets as a ValueVector reference. Will convert to ValueVector if possible, or will trigger assert error. */
    std::vector<DataValue>& asVector();
    /** Gets as a const ValueVector reference. Will convert to ValueVector if possible, or will trigger assert error. */
	const std::vector<DataValue>& asVector() const;

    /** Gets as a ValueMap reference. Will convert to ValueMap if possible, or will trigger assert error. */
	std::map<std::string, DataValue>& asMap();
    /** Gets as a const ValueMap reference. Will convert to ValueMap if possible, or will trigger assert error. */
    const std::map<std::string, DataValue>& asMap() const;

    /**
     * Checks if the Value is null.
     * @return True if the Value is null, false if not.
     */
    inline bool isNull() const { return _type == Type::NONE; }

    /** Value type wrapped by Value. */
    enum class Type
    {
        /// no value is wrapped, an empty Value
        NONE = 0,
        /// wrap byte
        BYTE,
        /// wrap integer
        INTEGER,
        /// wrap float
        FLOAT,
        /// wrap double
        DOUBLE,
        /// wrap bool
        BOOLEAN,
        /// wrap string
        STRING,
        /// wrap vector
        VECTOR,
        /// wrap ValueMap
        MAP
    };

    /** Gets the value type. */
    inline Type getType() const { return _type; }

    /** Gets the description of the class. */
    std::string getDescription() const;

private:

    void clear();
    void reset(Type type);

    union
    {
        unsigned char byteVal;
        int intVal;
        float floatVal;
        double doubleVal;
        bool boolVal;

        std::string* strVal;
        std::vector<DataValue>* vectorVal;
        std::map<std::string, DataValue>* mapVal;
    }_field;

    Type _type;
};

#endif
