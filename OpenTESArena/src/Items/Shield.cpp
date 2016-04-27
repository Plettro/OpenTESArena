#include <cassert>
#include <map>

#include "Shield.h"

#include "ArmorMaterial.h"
#include "ArmorType.h"
#include "HeavyArmorMaterial.h"
#include "Metal.h"
#include "ShieldArtifactData.h"
#include "../Entities/BodyPart.h"
#include "../Entities/BodyPartName.h"

const auto ShieldTypeDisplayNames = std::map<ShieldType, std::string>
{
	{ ShieldType::Buckler, "Buckler" },
	{ ShieldType::Round, "Round Shield" },
	{ ShieldType::Kite, "Kite Shield" },
	{ ShieldType::Tower, "Tower Shield" }
};

// Using positive armor ratings here. Negate them for 2nd edition rules.
const auto ShieldRatings = std::map<ShieldType, int>
{
	{ ShieldType::Buckler, 1 },
	{ ShieldType::Round, 2 },
	{ ShieldType::Kite, 3 },
	{ ShieldType::Tower, 4 }
};

// These numbers are based on iron. They are made up and will probably be revised 
// at some point.
const auto ShieldWeights = std::map<ShieldType, double>
{
	{ ShieldType::Buckler, 5.0 },
	{ ShieldType::Round, 6.0 },
	{ ShieldType::Kite, 8.0 },
	{ ShieldType::Tower, 12.0 }
};

// These numbers are based on iron. They are made up and will probably be revised 
// at some point.
const auto ShieldGoldValues = std::map<ShieldType, int>
{
	{ ShieldType::Buckler, 20 },
	{ ShieldType::Round, 30 },
	{ ShieldType::Kite, 45 },
	{ ShieldType::Tower, 60 }
};

// Shields protect multiple body parts, unlike regular body armor pieces.
const auto ShieldProtectedBodyParts = std::map<ShieldType, std::vector<BodyPartName>>
{
	{ ShieldType::Buckler, { BodyPartName::Hands, BodyPartName::LeftShoulder } },
	{ ShieldType::Round, { BodyPartName::Hands, BodyPartName::LeftShoulder } },
	{ ShieldType::Kite, { BodyPartName::Hands, BodyPartName::LeftShoulder, BodyPartName::Legs } },
	{ ShieldType::Tower, { BodyPartName::Chest, BodyPartName::Hands, BodyPartName::Head,
	BodyPartName::LeftShoulder, BodyPartName::Legs } }
};

Shield::Shield(ShieldType shieldType, MetalType metalType,
	const ShieldArtifactData *artifactData)
	: Armor(artifactData)
{
	this->armorMaterial = std::unique_ptr<HeavyArmorMaterial>(
		new HeavyArmorMaterial(metalType));
	this->shieldType = shieldType;

	assert(this->armorMaterial.get() != nullptr);
	assert(this->shieldType == shieldType);
}

Shield::Shield(ShieldType shieldType, MetalType metalType)
	: Shield(shieldType, metalType, nullptr) { }

Shield::Shield(const ShieldArtifactData *artifactData)
	: Shield(artifactData->getShieldType(), artifactData->getMetalType(),
		artifactData) { }

Shield::~Shield()
{

}

std::unique_ptr<Item> Shield::clone() const
{
	return std::unique_ptr<Item>(new Shield(this->getShieldType(),
		this->armorMaterial->getMetal().getMetalType(),
		dynamic_cast<const ShieldArtifactData*>(this->getArtifactData())));
}

double Shield::getWeight() const
{
	auto baseWeight = ShieldWeights.at(this->getShieldType());
	auto metalMultiplier = this->getArmorMaterial()->getWeightMultiplier();
	auto weight = baseWeight * metalMultiplier;
	assert(weight >= 0.0);
	return weight;
}

int Shield::getGoldValue() const
{
	// Refine this method sometime.
	int baseValue = ShieldGoldValues.at(this->getShieldType());
	int ratingModifier = this->getArmorRating();
	auto metalMultiplier = this->getArmorMaterial()->getWeightMultiplier();
	int value = static_cast<int>(static_cast<double>(baseValue + ratingModifier) * 
		metalMultiplier);
	return value;
}

std::string Shield::getDisplayName() const
{
	return this->getArmorMaterial()->toString() + " " + this->typeToString();
}

const ShieldType &Shield::getShieldType() const
{
	return this->shieldType;
}

std::string Shield::typeToString() const
{
	auto displayName = ShieldTypeDisplayNames.at(this->getShieldType());
	assert(displayName.size() > 0);
	return displayName;
}

ArmorType Shield::getArmorType() const
{
	return ArmorType::Shield;
}

const ArmorMaterial *Shield::getArmorMaterial() const
{
	return static_cast<ArmorMaterial*>(this->armorMaterial.get());
}

std::vector<BodyPartName> Shield::getProtectedBodyParts() const
{
	auto partNames = ShieldProtectedBodyParts.at(this->getShieldType());
	return partNames;
}

int Shield::getArmorRating() const
{
	int rating = ShieldRatings.at(this->getShieldType());
	return rating;
}
