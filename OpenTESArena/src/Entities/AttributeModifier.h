#ifndef ATTRIBUTE_MODIFIER_H
#define ATTRIBUTE_MODIFIER_H

#include <string>

// This static class is just for toString() purposes. Obtaining the modifier value is a 
// simple function, and can be done in the PrimaryAttribute class because it's the same 
// for all modifiers.

enum class AttributeModifierName;

class AttributeModifier
{
private:
	AttributeModifier() = delete;
	~AttributeModifier() = delete;
public:
	static const std::string &toString(AttributeModifierName modifierName);
};

#endif
