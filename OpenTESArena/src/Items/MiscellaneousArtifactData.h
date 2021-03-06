#ifndef MISCELLANEOUS_ARTIFACT_DATA_H
#define MISCELLANEOUS_ARTIFACT_DATA_H

#include <string>

#include "ArtifactData.h"

enum class ItemType;
enum class MiscellaneousItemType;

class MiscellaneousArtifactData : public ArtifactData
{
private:
	MiscellaneousItemType miscItemType;
public:
	MiscellaneousArtifactData(const std::string &displayName,
		const std::string &flavorText, const std::vector<int> &provinceIDs, 
		MiscellaneousItemType miscItemType);
	virtual ~MiscellaneousArtifactData() = default;

	virtual std::unique_ptr<ArtifactData> clone() const override;

	MiscellaneousItemType getMiscellaneousItemType() const;

	virtual ItemType getItemType() const override;
};

#endif
