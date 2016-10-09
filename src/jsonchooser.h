//
//  jsonchooser.hpp
//  Soundlab
//
//  Created by Stijn van Beek on 10/8/16.
//
//

#ifndef jsonchooser_hpp
#define jsonchooser_hpp

#include <stdio.h>

#include <rtti/rtti.h>
#include <nap/component.h>
#include <nap/componentdependency.h>
#include <nap/link.h>
#include <jsoncomponent.h>

namespace nap {
    
    // Class that applies the values in a json entry to attributes in a nap object and it's children
    class JsonChooser : public Component {
        RTTI_ENABLE_DERIVED_FROM(Component)
        
    public:
        // json path where the options entries are specified
        Attribute<std::string> optionsJsonPtr = { this, "optionsJsonPtr", "", &JsonChooser::optionsChanged };
        
        NumericAttribute<int> choice = { this, "choice", 0, 0, 0, &JsonChooser::choiceChanged };
        
        // TODO use some sort of serialiable Object link
        void setTarget(Object& object);
        
        // TODO use some sort of more flexible component dependency here
        void setJsonComponent(JsonComponent& component) { mJsonComponent = &component; }
        
    private:
        void choiceChanged(const int& value) { selectChoice(value); }
        void optionsChanged(const std::string& jsonPtr);
        
        void selectChoice(int index);
        
        JsonComponent* mJsonComponent = nullptr;
        Object* mTarget = nullptr;
    };
    
}

RTTI_DECLARE(nap::JsonChooser)

#endif /* jsonchooser_hpp */
