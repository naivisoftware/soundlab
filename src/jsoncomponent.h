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

        bool exists(rapidjson::Value& root, const std::string& jsonPointer);

        float getNumber(rapidjson::Value& root, const std::string& jsonPointer, float defaultValue = 0.f);
        std::string getString(rapidjson::Value& root, const std::string& jsonPointer, const std::string& defaultValue = "");
        rapidjson::Value* getValue(rapidjson::Value& root, const std::string& jsonPointer);

        std::vector<std::string> getStringArray(rapidjson::Value& root, const std::string& jsonPointer);
        std::vector<float> getNumberArray(rapidjson::Value& root, const std::string& jsonPointer);
        std::vector<rapidjson::Value*> getObjectArray(rapidjson::Value& root, const std::string& jsonPointer);
        
        rapidjson::Value* getValueByIndex(rapidjson::Value& root, const std::string& jsonPointer, int index);
        rapidjson::Value* getValueFromArray(rapidjson::Value& root, int inIndex);
        
        float getNumber(const std::string& jsonPointer, float defaultValue = 0.f);
        float getFloat(const std::string& jsonPointer, float defaultValue = 0);
        int getInt(const std::string& jsonPointer, float defaultValue = 0);
        std::string getString(const std::string& jsonPointer, const std::string& defaultValue = "");
        std::vector<std::string> getStringArray(const std::string& jsonPointer);
        std::vector<rapidjson::Value*> getObjectArray(const std::string& jsonPointer);
        
        void mapToAttributes(rapidjson::Value& json, Object& object);
        
        // Find and return an array as an actual json string
        std::string getJSONStringArray(const std::string& jsonPointer);

        rapidjson::Value* getValue(const std::string& jsonPointer);

        // returns wether the json document containing info on all the assets is loaded successfully
        bool isLoaded() const { return mDocument != nullptr; }

        const std::string& getRawDocumentContent() const {
            return mRawDocumentContent;
        }

    private:
        void jsonPathChanged(const std::string& path) { mDocument = loadDocument(path); }
        
        // the json document containing info on all the assets
        std::unique_ptr<rapidjson::Document> mDocument = nullptr;
        std::string mRawDocumentContent;
    };
    
}

RTTI_DECLARE(nap::JsonComponent)