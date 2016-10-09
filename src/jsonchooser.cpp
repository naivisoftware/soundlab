//
//  jsonchooser.cpp
//  Soundlab
//
//  Created by Stijn van Beek on 10/8/16.
//
//

#include "jsonchooser.h"
#include <nap/logger.h>

RTTI_DEFINE(nap::JsonChooser)

namespace nap {
    
    void JsonChooser::selectChoice(int value)
    {
        if (mJsonComponent && mTarget)
        {
            auto json = mJsonComponent->getValueByIndex(optionsJsonPtr.getValue(), value);
            if (!json)
            {
                Logger::warn("JsonChooser::choiceChanged(): json entry not found: " + std::to_string(value));
                return;
            }
            
            mJsonComponent->mapToAttributes(*json, *mTarget);
        }
    }
    
    
    void JsonChooser::optionsChanged(const std::string& jsonPtr)
    {
        if (mJsonComponent);
        {
            auto* json = mJsonComponent->getValue(optionsJsonPtr.getValue());
            if (!json)
            {
                Logger::warn("JsonChooser::choiceChanged(): json entry not found");
                return;
            }
            
            int numberOfOptions = mJsonComponent->getSize(jsonPtr);
            choice.setRange(0, numberOfOptions - 1);
        }
        selectChoice(choice.getValue());
    }
    
    
    void JsonChooser::setTarget(Object& object)
    {
        mTarget = &object;
        selectChoice(choice.getValue());
    }
    
}
