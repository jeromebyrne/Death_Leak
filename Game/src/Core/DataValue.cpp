#include "precompiled.h"
#include "DataValue.h"
#include <sstream>
#include <iomanip>

const std::vector<DataValue> VectorNull;
const std::map<std::string, DataValue> MapNull;

const DataValue DataValue::Null;

DataValue::DataValue()
: _type(Type::NONE)
{
    memset(&_field, 0, sizeof(_field));
}

DataValue::DataValue(unsigned char v)
: _type(Type::BYTE)
{
    _field.byteVal = v;
}

DataValue::DataValue(int v)
: _type(Type::INTEGER)
{
    _field.intVal = v;
}

DataValue::DataValue(float v)
: _type(Type::FLOAT)
{
    _field.floatVal = v;
}

DataValue::DataValue(double v)
: _type(Type::DOUBLE)
{
    _field.doubleVal = v;
}

DataValue::DataValue(bool v)
: _type(Type::BOOLEAN)
{
    _field.boolVal = v;
}

DataValue::DataValue(const char* v)
: _type(Type::STRING)
{
    _field.strVal = new std::string();
    if (v)
    {
        *_field.strVal = v;
    }
}

DataValue::DataValue(const std::string& v)
: _type(Type::STRING)
{
    _field.strVal = new std::string();
    *_field.strVal = v;
}

DataValue::DataValue(const std::vector<DataValue>& v)
: _type(Type::VECTOR)
{
    _field.vectorVal = new (std::nothrow) std::vector<DataValue>();
    *_field.vectorVal = v;
}

DataValue::DataValue(std::vector<DataValue>&& v)
: _type(Type::VECTOR)
{
    _field.vectorVal = new (std::nothrow) std::vector<DataValue>();
    *_field.vectorVal = std::move(v);
}

DataValue::DataValue(const std::map<std::string, DataValue>& v)
: _type(Type::MAP)
{
    _field.mapVal = new (std::nothrow) std::map<std::string, DataValue>();
    *_field.mapVal = v;
}

DataValue::DataValue(std::map<std::string, DataValue>&& v)
: _type(Type::MAP)
{
    _field.mapVal = new (std::nothrow) std::map<std::string, DataValue>();
    *_field.mapVal = std::move(v);
}

DataValue::DataValue(const DataValue& other)
: _type(Type::NONE)
{
    *this = other;
}

DataValue::DataValue(DataValue&& other)
: _type(Type::NONE)
{
    *this = std::move(other);
}

DataValue::~DataValue()
{
    clear();
}

DataValue& DataValue::operator= (const DataValue& other)
{
    if (this != &other) {
        reset(other._type);

        switch (other._type) {
            case Type::BYTE:
                _field.byteVal = other._field.byteVal;
                break;
            case Type::INTEGER:
                _field.intVal = other._field.intVal;
                break;
            case Type::FLOAT:
                _field.floatVal = other._field.floatVal;
                break;
            case Type::DOUBLE:
                _field.doubleVal = other._field.doubleVal;
                break;
            case Type::BOOLEAN:
                _field.boolVal = other._field.boolVal;
                break;
            case Type::STRING:
                if (_field.strVal == nullptr)
                {
                    _field.strVal = new std::string();
                }
                *_field.strVal = *other._field.strVal;
                break;
            case Type::VECTOR:
                if (_field.vectorVal == nullptr)
                {
                    _field.vectorVal = new (std::nothrow) std::vector<DataValue>();
                }
                *_field.vectorVal = *other._field.vectorVal;
                break;
            case Type::MAP:
                if (_field.mapVal == nullptr)
                {
                    _field.mapVal = new (std::nothrow) std::map<std::string, DataValue>();
                }
                *_field.mapVal = *other._field.mapVal;
                break;
            default:
                break;
        }
    }
    return *this;
}

DataValue& DataValue::operator= (DataValue&& other)
{
    if (this != &other)
    {
        clear();
        switch (other._type)
        {
            case Type::BYTE:
                _field.byteVal = other._field.byteVal;
                break;
            case Type::INTEGER:
                _field.intVal = other._field.intVal;
                break;
            case Type::FLOAT:
                _field.floatVal = other._field.floatVal;
                break;
            case Type::DOUBLE:
                _field.doubleVal = other._field.doubleVal;
                break;
            case Type::BOOLEAN:
                _field.boolVal = other._field.boolVal;
                break;
            case Type::STRING:
                _field.strVal = other._field.strVal;
                break;
            case Type::VECTOR:
                _field.vectorVal = other._field.vectorVal;
                break;
            case Type::MAP:
                _field.mapVal = other._field.mapVal;
                break;
            default:
                break;
        }
        _type = other._type;

        memset(&other._field, 0, sizeof(other._field));
        other._type = Type::NONE;
    }

    return *this;
}

DataValue& DataValue::operator= (unsigned char v)
{
    reset(Type::BYTE);
    _field.byteVal = v;
    return *this;
}

DataValue& DataValue::operator= (int v)
{
    reset(Type::INTEGER);
    _field.intVal = v;
    return *this;
}

DataValue& DataValue::operator= (float v)
{
    reset(Type::FLOAT);
    _field.floatVal = v;
    return *this;
}

DataValue& DataValue::operator= (double v)
{
    reset(Type::DOUBLE);
    _field.doubleVal = v;
    return *this;
}

DataValue& DataValue::operator= (bool v)
{
    reset(Type::BOOLEAN);
    _field.boolVal = v;
    return *this;
}

DataValue& DataValue::operator= (const char* v)
{
    reset(Type::STRING);
    *_field.strVal = v ? v : "";
    return *this;
}

DataValue& DataValue::operator= (const std::string& v)
{
    reset(Type::STRING);
    *_field.strVal = v;
    return *this;
}

DataValue& DataValue::operator= (const std::vector<DataValue>& v)
{
    reset(Type::VECTOR);
    *_field.vectorVal = v;
    return *this;
}

DataValue& DataValue::operator= (std::vector<DataValue>&& v)
{
    reset(Type::VECTOR);
    *_field.vectorVal = std::move(v);
    return *this;
}

DataValue& DataValue::operator= (const std::map<std::string, DataValue>& v)
{
    reset(Type::MAP);
    *_field.mapVal = v;
    return *this;
}

DataValue& DataValue::operator= (std::map<std::string, DataValue>&& v)
{
    reset(Type::MAP);
    *_field.mapVal = std::move(v);
    return *this;
}

bool DataValue::operator!= (const DataValue& v)
{
    return !(*this == v);
}
bool DataValue::operator!= (const DataValue& v) const
{
    return !(*this == v);
}

bool DataValue::operator== (const DataValue& v)
{
    const auto &t = *this;
    return t == v;
}
bool DataValue::operator== (const DataValue& v) const
{
    if (this == &v) return true;
    if (v._type != this->_type) return false;
    if (this->isNull()) return true;
    switch (_type)
    {
    case Type::BYTE:    return v._field.byteVal   == this->_field.byteVal;
    case Type::INTEGER: return v._field.intVal    == this->_field.intVal;
    case Type::BOOLEAN: return v._field.boolVal   == this->_field.boolVal;
    case Type::STRING:  return *v._field.strVal   == *this->_field.strVal;
    case Type::FLOAT:   return fabs(v._field.floatVal  - this->_field.floatVal)  <= FLT_EPSILON;
    case Type::DOUBLE:  return fabs(v._field.doubleVal - this->_field.doubleVal) <= FLT_EPSILON;
    case Type::VECTOR:
    {
        const auto &v1 = *(this->_field.vectorVal);
        const auto &v2 = *(v._field.vectorVal);
        const auto size = v1.size();
        if (size == v2.size())
        {
            for (size_t i = 0; i < size; i++)
            {
                if (v1[i] != v2[i]) return false;
            }
        }
        return true;
    }
    case Type::MAP:
    {
        const auto &map1 = *(this->_field.mapVal);
        const auto &map2 = *(v._field.mapVal);
        for (const auto &kvp : map1)
        {
            auto it = map2.find(kvp.first);
            if (it == map2.end() || it->second != kvp.second)
            {
                return false;
            }
        }
        return true;
    }
    default:
        break;
    };

    return false;
}

/// Convert value to a specified type
unsigned char DataValue::asByte() const
{
    GAME_ASSERT(_type != Type::VECTOR && _type != Type::MAP);

    if (_type == Type::BYTE)
    {
        return _field.byteVal;
    }

    if (_type == Type::INTEGER)
    {
        return static_cast<unsigned char>(_field.intVal);
    }

    if (_type == Type::STRING)
    {
        return static_cast<unsigned char>(atoi(_field.strVal->c_str()));
    }

    if (_type == Type::FLOAT)
    {
        return static_cast<unsigned char>(_field.floatVal);
    }

    if (_type == Type::DOUBLE)
    {
        return static_cast<unsigned char>(_field.doubleVal);
    }

    if (_type == Type::BOOLEAN)
    {
        return _field.boolVal ? 1 : 0;
    }

    return 0;
}

int DataValue::asInt() const
{
    GAME_ASSERT(_type != Type::VECTOR && _type != Type::MAP);
    if (_type == Type::INTEGER)
    {
        return _field.intVal;
    }

    if (_type == Type::BYTE)
    {
        return _field.byteVal;
    }

    if (_type == Type::STRING)
    {
        return atoi(_field.strVal->c_str());
    }

    if (_type == Type::FLOAT)
    {
        return static_cast<int>(_field.floatVal);
    }

    if (_type == Type::DOUBLE)
    {
        return static_cast<int>(_field.doubleVal);
    }

    if (_type == Type::BOOLEAN)
    {
        return _field.boolVal ? 1 : 0;
    }

    return 0;
}

float DataValue::asFloat() const
{
    GAME_ASSERT(_type != Type::VECTOR && _type != Type::MAP);
    if (_type == Type::FLOAT)
    {
        return _field.floatVal;
    }

    if (_type == Type::BYTE)
    {
        return static_cast<float>(_field.byteVal);
    }

    if (_type == Type::STRING)
    {
		return Utilities::ConvertStringToDouble(_field.strVal->c_str());
    }

    if (_type == Type::INTEGER)
    {
        return static_cast<float>(_field.intVal);
    }

    if (_type == Type::DOUBLE)
    {
        return static_cast<float>(_field.doubleVal);
    }

    if (_type == Type::BOOLEAN)
    {
        return _field.boolVal ? 1.0f : 0.0f;
    }

    return 0.0f;
}

double DataValue::asDouble() const
{
    GAME_ASSERT(_type != Type::VECTOR && _type != Type::MAP);
    if (_type == Type::DOUBLE)
    {
        return _field.doubleVal;
    }

    if (_type == Type::BYTE)
    {
        return static_cast<double>(_field.byteVal);
    }

    if (_type == Type::STRING)
    {
		return static_cast<double>(Utilities::ConvertStringToDouble(_field.strVal->c_str()));
    }

    if (_type == Type::INTEGER)
    {
        return static_cast<double>(_field.intVal);
    }

    if (_type == Type::FLOAT)
    {
        return static_cast<double>(_field.floatVal);
    }

    if (_type == Type::BOOLEAN)
    {
        return _field.boolVal ? 1.0 : 0.0;
    }

    return 0.0;
}

bool DataValue::asBool() const
{
    GAME_ASSERT(_type != Type::VECTOR && _type != Type::MAP);
    if (_type == Type::BOOLEAN)
    {
        return _field.boolVal;
    }

    if (_type == Type::BYTE)
    {
        return _field.byteVal == 0 ? false : true;
    }

    if (_type == Type::STRING)
    {
        return (*_field.strVal == "0" || *_field.strVal == "false") ? false : true;
    }

    if (_type == Type::INTEGER)
    {
        return _field.intVal == 0 ? false : true;
    }

    if (_type == Type::FLOAT)
    {
        return _field.floatVal == 0.0f ? false : true;
    }

    if (_type == Type::DOUBLE)
    {
        return _field.doubleVal == 0.0 ? false : true;
    }

    return false;
}

std::string DataValue::asString() const
{
    GAME_ASSERT(_type != Type::VECTOR && _type != Type::MAP);

    if (_type == Type::STRING)
    {
        return *_field.strVal;
    }

    std::stringstream ret;

    switch (_type)
    {
        case Type::BYTE:
            ret << _field.byteVal;
            break;
        case Type::INTEGER:
            ret << _field.intVal;
            break;
        case Type::FLOAT:
            ret << std::fixed << std::setprecision( 7 )<< _field.floatVal;
            break;
        case Type::DOUBLE:
            ret << std::fixed << std::setprecision( 16 ) << _field.doubleVal;
            break;
        case Type::BOOLEAN:
            ret << (_field.boolVal ? "true" : "false");
            break;
        default:
            break;
    }
    return ret.str();
}

std::vector<DataValue>& DataValue::asVector()
{
    GAME_ASSERT(_type == Type::VECTOR);
    return *_field.vectorVal;
}

const std::vector<DataValue>& DataValue::asVector() const
{
    GAME_ASSERT(_type == Type::VECTOR);
    return *_field.vectorVal;
}

std::map<std::string, DataValue>& DataValue::asMap()
{
    GAME_ASSERT(_type == Type::MAP);
    return *_field.mapVal;
}

const std::map<std::string, DataValue>& DataValue::asMap() const
{
    GAME_ASSERT(_type == Type::MAP);
    return *_field.mapVal;
}

static std::string getTabs(int depth)
{
    std::string tabWidth;

    for (int i = 0; i < depth; ++i)
    {
        tabWidth += "\t";
    }

    return tabWidth;
}

static std::string visit(const DataValue& v, int depth);

static std::string visitVector(const std::vector<DataValue>& v, int depth)
{
    std::stringstream ret;

    if (depth > 0)
        ret << "\n";

    ret << getTabs(depth) << "[\n";

    int i = 0;
    for (const auto& child : v)
    {
        ret << getTabs(depth+1) << i << ": " << visit(child, depth + 1);
        ++i;
    }

    ret << getTabs(depth) << "]\n";

    return ret.str();
}

template <class T>
static std::string visitMap(const T& v, int depth)
{
    std::stringstream ret;

    if (depth > 0)
        ret << "\n";

    ret << getTabs(depth) << "{\n";

    for (auto iter = v.begin(); iter != v.end(); ++iter)
    {
        ret << getTabs(depth + 1) << iter->first << ": ";
        ret << visit(iter->second, depth + 1);
    }

    ret << getTabs(depth) << "}\n";

    return ret.str();
}

static std::string visit(const DataValue& v, int depth)
{
    std::stringstream ret;

    switch (v.getType())
    {
        case DataValue::Type::NONE:
        case DataValue::Type::BYTE:
        case DataValue::Type::INTEGER:
        case DataValue::Type::FLOAT:
        case DataValue::Type::DOUBLE:
        case DataValue::Type::BOOLEAN:
        case DataValue::Type::STRING:
            ret << v.asString() << "\n";
            break;
        case DataValue::Type::VECTOR:
            ret << visitVector(v.asVector(), depth);
            break;
        case DataValue::Type::MAP:
            ret << visitMap(v.asMap(), depth);
            break;
        default:
            GAME_ASSERT(false);
            break;
    }

    return ret.str();
}

std::string DataValue::getDescription() const
{
    std::string ret("\n");
    ret += visit(*this, 0);
    return ret;
}

void DataValue::clear()
{
    // Free memory the old value allocated
    switch (_type)
    {
        case Type::BYTE:
            _field.byteVal = 0;
            break;
        case Type::INTEGER:
            _field.intVal = 0;
            break;
        case Type::FLOAT:
            _field.floatVal = 0.0f;
            break;
        case Type::DOUBLE:
            _field.doubleVal = 0.0;
            break;
        case Type::BOOLEAN:
            _field.boolVal = false;
            break;
        case Type::STRING:
			if (_field.strVal)
			{
				delete _field.strVal;
			}
			_field.strVal = nullptr;
            break;
        case Type::VECTOR:
			if (_field.vectorVal)
			{
				delete _field.vectorVal;
			}
			_field.vectorVal = nullptr;
            break;
        case Type::MAP:
			if (_field.mapVal)
			{
				delete _field.mapVal;
			}
			_field.mapVal = nullptr;
            break;
        default:
            break;
    }
    
    _type = Type::NONE;
}

void DataValue::reset(Type type)
{
    if (_type == type)
        return;

    clear();

    // Allocate memory for the new value
    switch (type)
    {
        case Type::STRING:
            _field.strVal = new std::string();
            break;
        case Type::VECTOR:
            _field.vectorVal = new (std::nothrow) std::vector<DataValue>();
            break;
        case Type::MAP:
            _field.mapVal = new (std::nothrow) std::map<std::string, DataValue>();
            break;
        default:
            break;
    }

    _type = type;
}
