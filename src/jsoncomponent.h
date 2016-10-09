#pragma once
#include <rtti/rtti.h>
#include <nap/logger.h>
#include <nap/component.h>
#include <nap/coremodule.h>

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <string>

namespace nap {
    
    class JsonComponent : public nap::Component
    {
        RTTI_ENABLE_DERIVED_FROM(nap::Component)
    public:
        // downloads a json document from a host server. Returns a nullptr if the path is invalid or the document failed to
        // parse properly.
        std::unique_ptr<rapidjson::Document> loadDocument(const std::string& path);

        // changes the path to the json file containing info on all the assets on a host
        nap::Attribute<std::string> jsonPath = {this, "jsonPath", "", &JsonComponent::jsonPathChanged};

        // Checks wether a json entry exists at the location where the jsonPOinter points to
        bool exists(rapidjson::Value& root, const std::string& jsonPointer);

        // Return a generic json object
        rapidjson::Value* getValue(rapidjson::Value& root, const std::string& jsonPointer);
        
        // Return a numeric type
        template <typename T>
        T getNumber(rapidjson::Value& root, const std::string& jsonPointer, T defaultValue);
        
        // Reeturn a string
        std::string getString(rapidjson::Value& root, const std::string& jsonPointer, const std::string& defaultValue = "");

        // Return a numeric array
        template <typename T>
        std::vector<T> getNumberArray(rapidjson::Value& root, const std::string& jsonPointer);
        
        // Return a string array
        std::vector<std::string> getStringArray(rapidjson::Value& root, const std::string& jsonPointer);
        
        // Return an array of generic json objects
        std::vector<rapidjson::Value*> getObjectArray(rapidjson::Value& root, const std::string& jsonPointer);
        
        // Return a generic json object using an index within a parent object
        rapidjson::Value* getValueByIndex(rapidjson::Value& root, const std::string& jsonPointer, int index);
        
        // Return a generic json object using an index into an array
        rapidjson::Value* getValueFromArray(rapidjson::Value& root, int inIndex);
        
        // Return the number of entries withis this object
        int getSize(rapidjson::Value& root, const std::string& jsonPointer);
        
        // Return a generic json object using an index into an array
        rapidjson::Value* getValue(const std::string& jsonPointer);
        
        // Same as other @getNumber but using the document as root
        template <typename T>
        T getNumber(const std::string& jsonPointer, T defaultValue);
        
        // Same as other @getNumberArray but using the document as root
        template <typename T>
        std::vector<T> getNumberArray(const std::string& jsonPointer);
        
        // Same as other @getString but using the document as root
        std::string getString(const std::string& jsonPointer, const std::string& defaultValue = "");
        
        // Same as other @getStringArray but using the document as root
        std::vector<std::string> getStringArray(const std::string& jsonPointer);
        
        // Same as other @getObjectArray but using the document as root
        std::vector<rapidjson::Value*> getObjectArray(const std::string& jsonPointer);
        
        // Return a generic json object using an index within a parent object
        rapidjson::Value* getValueByIndex(const std::string& jsonPointer, int index);
        
        // Return the number of entries withis this object
        int getSize(const std::string& jsonPointer);
        
        rapidjson::Value* getValueFromArray(int inIndex);
        
        // Maps the content of a generic json object to the values of attributes in the nap Object tree
        void mapToAttributes(rapidjson::Value& json, Object& object);
        
        // returns wether the json document containing info on all the assets is loaded successfully
        bool isLoaded() const { return mDocument != nullptr; }

        const std::string& getRawDocumentContent() const {
            return mRawDocumentContent;
        }

        // Find and return an array as an actual json string
        std::string getJSONStringArray(const std::string& jsonPointer);
        
    private:
        void jsonPathChanged(const std::string& path) { mDocument = loadDocument(path); }
        
        // the json document containing info on all the assets
        std::unique_ptr<rapidjson::Document> mDocument = nullptr;
        std::string mRawDocumentContent;
    };
    
    
    template <typename T>
    T JsonComponent::getNumber(rapidjson::Value& root, const std::string& jsonPointer, T defaultValue)
    {
        rapidjson::Value* value = getValue(root, jsonPointer);
        
        if (!value) return defaultValue;
        
        if (value->IsFloat())
            return T(value->GetFloat());
        
        else if (value->IsDouble())
            return T(value->GetDouble());
        
        else if (value->IsInt())
            return T(value->GetInt());
        
        else if (value->IsInt64())
            return T(value->GetInt64());
        
        Logger::warn("json value not a number: " + jsonPointer);
        return defaultValue;
    }
    
    
    template <typename T>
    std::vector<T> JsonComponent::getNumberArray(rapidjson::Value& root, const std::string& jsonPointer)
    {
        std::vector<T> arr;
        rapidjson::Value* json = getValue(root, jsonPointer);
        if (!json)
            return arr;
        
        if (!json->IsArray())
        {
            Logger::warn("Not an array: " + jsonPointer);
            return arr;
        }
        
        for (auto it = json->Begin(); it != json->End(); ++it)
        {
            rapidjson::Value* elm = it;
            if (elm->IsFloat())
                arr.emplace_back(elm->GetFloat());
            if (elm->IsInt())
                arr.emplace_back(elm->GetInt());
        }
        return arr;
    }
    
    
    template <typename T>
    T JsonComponent::getNumber(const std::string& jsonPointer, T defaultValue)
    {
        if (!mDocument) return defaultValue;
        return getNumber<T>(*mDocument, jsonPointer, defaultValue);
    }
    
    
    template <typename T>
    std::vector<T> JsonComponent::getNumberArray(const std::string& jsonPointer)
    {
        if (!mDocument)
        {
            std::vector<T> emptyResult;
            return emptyResult;
        }
        return getNumberArray<T>(*mDocument, jsonPointer);
    }
    
    
    
}

RTTI_DECLARE(nap::JsonComponent)