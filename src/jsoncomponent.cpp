
#include "jsoncomponent.h"
#include <fstream>
#include <nap/logger.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

RTTI_DEFINE(nap::JsonComponent)

using namespace nap;

template <typename T>
inline bool contains(const std::vector<T> vec, const T& element)
{
	return std::find(vec.begin(), vec.end(), element) != vec.end();
}

namespace nap {

    std::unique_ptr<rapidjson::Document> JsonComponent::loadDocument(const std::string& path)
    {
        auto document = std::make_unique<rapidjson::Document>();

        rapidjson::ParseResult parseResult;

        std::ifstream fileStream;
        fileStream.open(path);
        if (fileStream)
        {
            std::stringstream stringStream;
            stringStream << fileStream.rdbuf();
            parseResult = document->Parse(stringStream.str().c_str());
        }
        else
        {
            Logger::warn("Failed to open " + path);
            return nullptr;
        }

        if (document->HasParseError())
        {
            Logger::warn("JSON parse error: %s (%u) in '%s'", rapidjson::GetParseError_En(parseResult.Code()), parseResult.Offset(), path.c_str());
        }

        return document;
    }


    float JsonComponent::getNumber(rapidjson::Value& root, const std::string& jsonPointer, float defaultValue /*= 0.f*/)
    {
        rapidjson::Value* value = getValue(root, jsonPointer);

        if (!value) return defaultValue;

        if (value->IsFloat())
            return value->GetFloat();

        else if (value->IsDouble())
            return float(value->GetDouble());

        else if (value->GetInt())
            return float(value->GetInt());

        Logger::warn("json value not a number: " + jsonPointer);
        return defaultValue;
    }


    std::string JsonComponent::getString(rapidjson::Value& root, const std::string& jsonPointer, const std::string& defaultValue /*= ""*/)
    {
        rapidjson::Value* value = getValue(root, jsonPointer);

        if (value == nullptr) return defaultValue;

        if (value->IsString()) return value->GetString();

        rapidjson::Type t = value->GetType();

        Logger::debug("Type: %d", (int)t);

        Logger::warn("json value not a string: " + jsonPointer);
        return defaultValue;
    }


    std::vector<std::string> JsonComponent::getStringArray(rapidjson::Value& root, const std::string& jsonPointer)
    {
        std::vector<std::string> arr;
        rapidjson::Value* strings = getValue(root, jsonPointer);
        if (!strings) return arr;

        if (!strings->IsArray())
        {
            Logger::warn("Not an array: " + jsonPointer);
            return arr;
        }

        for (auto it = strings->Begin(); it != strings->End(); ++it)
        {
            rapidjson::Value* elm = it;
            if (!elm->IsString()) continue;
            arr.push_back(elm->GetString());
        }
        return arr;
    }


    std::vector<float> JsonComponent::getNumberArray(rapidjson::Value& root, const std::string& jsonPointer)
    {
        std::vector<float> arr;
        rapidjson::Value* floats = getValue(root, jsonPointer);
        if (!floats) return arr;
        
        if (!floats->IsArray())
        {
            Logger::warn("Not an array: " + jsonPointer);
            return arr;
        }
        
        for (auto it = floats->Begin(); it != floats->End(); ++it)
        {
            rapidjson::Value* elm = it;
            if (elm->IsFloat())
                arr.push_back(elm->GetFloat());
            if (elm->IsInt())
                arr.push_back(elm->GetInt());
        }
        return arr;
    }

    
    std::vector<rapidjson::Value*> JsonComponent::getObjectArray(rapidjson::Value& root, const std::string& jsonPointer)
    {
        std::vector<rapidjson::Value*> result;
        rapidjson::Value* jsonArray = getValue(root, jsonPointer);
        if (!jsonArray)
            return result;
        
        if (!jsonArray->IsArray())
        {
            Logger::warn("Not an array: " + jsonPointer);
            return result;
        }
        
        for (auto it = jsonArray->Begin(); it != jsonArray->End(); ++it)
        {
            rapidjson::Value* elm = it;
            result.emplace_back(elm);
        }
        return result;
    }    


    rapidjson::Value* JsonComponent::getValue(rapidjson::Value& root, const std::string& jsonPointer)
    {
        rapidjson::Value* value = rapidjson::Pointer(jsonPointer.c_str()).Get(root);

        if (!value)
        {
            Logger::warn("json value not found: " + jsonPointer);
        }

        return value;
    }


    bool JsonComponent::exists(rapidjson::Value& root, const std::string& jsonPointer) { return rapidjson::Pointer(jsonPointer.c_str()).Get(root) != nullptr; }


    rapidjson::Value* JsonComponent::getValueByIndex(rapidjson::Value& root, const std::string& jsonPointer, int index)
    {
        rapidjson::Value* object = getValue(root, jsonPointer);

        if (!object) return nullptr;

        if (object->IsArray()) return getValueFromArray(root, index);

        if (object->IsObject())
        {
            auto i = 0;
            for (auto it = object->MemberBegin(); it != object->MemberEnd(); ++it)
            {
                if (i == index) return &it->value;
                i++;
            }
        }
        return nullptr;
    }


    rapidjson::Value* JsonComponent::getValueFromArray(rapidjson::Value& root, int inIndex)
    {
        bool is_array = root.IsArray();
        if (!is_array || inIndex >= (int)root.GetArray().Size()) return nullptr;

        return &root[inIndex];
    }
    
    
    void JsonComponent::mapToAttributes(rapidjson::Value& json, Object& object)
    {
        if (object.getTypeInfo().isKindOf<AttributeBase>())
        {
            if (json.IsInt())
            {
                if (object.getTypeInfo().isKindOf<Attribute<int>>())
                    static_cast<Attribute<int>*>(&object)->setValue(json.GetInt());
                if (object.getTypeInfo().isKindOf<Attribute<float>>())
                    static_cast<Attribute<float>*>(&object)->setValue(json.GetInt());
            }
            if (json.IsFloat())
            {
                if (object.getTypeInfo().isKindOf<Attribute<int>>())
                    static_cast<Attribute<int>*>(&object)->setValue(json.GetFloat());
                if (object.getTypeInfo().isKindOf<Attribute<float>>())
                    static_cast<Attribute<float>*>(&object)->setValue(json.GetFloat());
            }
            if (json.IsString())
            {
                if (object.getTypeInfo().isKindOf<Attribute<std::string>>())
                    static_cast<Attribute<std::string>*>(&object)->setValue(json.GetString());
            }
            if (json.IsBool())
            {
                if (object.getTypeInfo().isKindOf<Attribute<bool>>())
                    static_cast<Attribute<bool>*>(&object)->setValue(json.GetBool());
            }
            if (json.IsArray())
            {
                if (object.getTypeInfo().isKindOf<Attribute<FloatArray>>())
                    static_cast<Attribute<FloatArray>*>(&object)->setValue(getNumberArray(json, ""));
                if (object.getTypeInfo().isKindOf<Attribute<IntArray>>())
                {
                    FloatArray floats = getNumberArray(json, "");
                    IntArray ints;
                    for (auto& number : floats)
                        ints.emplace_back(number);
                    
                    static_cast<Attribute<IntArray>*>(&object)->setValue(ints);
                }
                if (object.getTypeInfo().isKindOf<Attribute<StringArray>>())
                    static_cast<Attribute<StringArray>*>(&object)->setValue(getStringArray(json, ""));
                if (object.getTypeInfo().isKindOf<ArrayAttribute<float>>())
                {
                    
                }
            }
            
        }
        else {
            if (json.IsObject())
            {
                for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it)
                {
                    std::string name = it->name.GetString();
                    Object* child = object.getChild(name);
                    if (child)
                    {
                        mapToAttributes(it->value, *child);
                    }
                }
                
            }
        }
    }


    std::string JsonComponent::getJSONStringArray(const std::string& jsonPointer)
    {
        // TODO: Replace with rapidjson
        auto strings = getValue(jsonPointer)->GetArray();
        std::ostringstream ss;
        int i = 0;
        ss << "[";
        for (auto it = strings.Begin(); it != strings.End(); ++it)
        {
            if (i > 0) ss << ",";
            ss << "\"" << it->GetString() << "\"";
            i++;
        }
        ss << "]";
        return ss.str();
    }


    float JsonComponent::getNumber(const std::string& jsonPointer, float defaultValue)
    {
        if (!mDocument) return defaultValue;
        return getNumber(*mDocument, jsonPointer, defaultValue);
    }


    float JsonComponent::getFloat(const std::string& jsonPointer, float defaultValue) { return getNumber(jsonPointer, defaultValue); }


    int JsonComponent::getInt(const std::string& jsonPointer, float defaultValue) { return (int)getNumber(jsonPointer, defaultValue); }


    std::string JsonComponent::getString(const std::string& jsonPointer, const std::string& defaultValue)
    {
        if (!mDocument)
            return defaultValue;
        return
            getString(*mDocument, jsonPointer, defaultValue);
    }
    
    
    std::vector<std::string> JsonComponent::getStringArray(const std::string& jsonPointer)
    {
        std::vector<std::string> result;
        if (!mDocument)
            return result;
        return getStringArray(*mDocument, jsonPointer);
    }
    
    
    std::vector<rapidjson::Value*> JsonComponent::getObjectArray(const std::string& jsonPointer)
    {
        std::vector<rapidjson::Value*> result;
        if (!mDocument)
            return result;
        return getObjectArray(*mDocument, jsonPointer);
    }    


    rapidjson::Value* JsonComponent::getValue(const std::string& jsonPointer)
    {
        if (!mDocument)
            return nullptr;
        return getValue(*mDocument, jsonPointer);
    }
    
}
