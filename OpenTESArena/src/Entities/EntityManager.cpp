#include <algorithm>

#include "EntityManager.h"
#include "EntityType.h"

#include "components/debug/Debug.h"

template <typename T>
int EntityManager::EntityGroup<T>::getCount() const
{
	return static_cast<int>(this->entities.size());
}

template <typename T>
T *EntityManager::EntityGroup<T>::getEntityAtIndex(int index)
{
	DebugAssert(this->validEntities.size() == this->entities.size());
	DebugAssertIndex(this->validEntities, index);
	const bool isValid = this->validEntities[index];
	return isValid ? &this->entities[index] : nullptr;
}

template <typename T>
const T *EntityManager::EntityGroup<T>::getEntityAtIndex(int index) const
{
	DebugAssert(this->validEntities.size() == this->entities.size());
	DebugAssertIndex(this->validEntities, index);
	const bool isValid = this->validEntities[index];
	return isValid ? &this->entities[index] : nullptr;
}

template <typename T>
int EntityManager::EntityGroup<T>::getEntities(Entity **outEntities, int outSize)
{
	DebugAssert(outEntities != nullptr);
	DebugAssert(outSize >= 0);
	DebugAssert(this->validEntities.size() == this->entities.size());

	int writeIndex = 0;
	const int count = this->getCount();
	for (int i = 0; i < count; i++)
	{
		// Break if the destination buffer is full.
		if (writeIndex == outSize)
		{
			break;
		}

		DebugAssertIndex(this->validEntities, i);
		const bool isValid = this->validEntities[i];

		// Skip if entity is not valid.
		if (!isValid)
		{
			continue;
		}

		T &entity = this->entities[i];
		outEntities[writeIndex] = &entity;
		writeIndex++;
	}

	return writeIndex;
}

template <typename T>
int EntityManager::EntityGroup<T>::getEntities(const Entity **outEntities, int outSize) const
{
	DebugAssert(outEntities != nullptr);
	DebugAssert(outSize >= 0);
	DebugAssert(this->validEntities.size() == this->entities.size());

	int writeIndex = 0;
	const int count = this->getCount();
	for (int i = 0; i < count; i++)
	{
		// Break if the destination buffer is full.
		if (writeIndex == outSize)
		{
			break;
		}

		DebugAssertIndex(this->validEntities, i);
		const bool isValid = this->validEntities[i];

		// Skip if entity is not valid.
		if (!isValid)
		{
			continue;
		}

		const T &entity = this->entities[i];
		outEntities[writeIndex] = &entity;
		writeIndex++;
	}

	return writeIndex;
}

template <typename T>
std::optional<int> EntityManager::EntityGroup<T>::getEntityIndex(int id) const
{
	const auto iter = this->indices.find(id);
	if (iter != this->indices.end())
	{
		return iter->second;
	}
	else
	{
		return std::nullopt;
	}
}

template <typename T>
T *EntityManager::EntityGroup<T>::addEntity(int id)
{
	DebugAssert(id != EntityManager::NO_ID);
	DebugAssert(this->validEntities.size() == this->entities.size());

	// Entity ID must not already be in use.
	DebugAssert(!this->getEntityIndex(id).has_value());

	// Find available position in entities array.
	int index;
	if (this->freeIndices.size() > 0)
	{
		// Reuse a previously-owned entity slot.
		index = this->freeIndices.back();
		this->freeIndices.pop_back();

		DebugAssertIndex(this->validEntities, index);
		this->validEntities[index] = true;
	}
	else
	{
		// Insert new at the end of the entities list.
		index = static_cast<int>(this->entities.size());
		this->entities.push_back(T());
		this->validEntities.push_back(true);
	}

	// Initialize basic entity data.
	DebugAssertIndex(this->entities, index);
	T &entitySlot = this->entities[index];
	entitySlot.reset();
	entitySlot.setID(id);

	// Insert into ID -> entity index table.
	this->indices.insert(std::make_pair(id, index));

	return &entitySlot;
}

template <typename T>
void EntityManager::EntityGroup<T>::remove(int id)
{
	DebugAssert(id != EntityManager::NO_ID);
	DebugAssert(this->validEntities.size() == this->entities.size());

	// Find entity with the given ID.
	const std::optional<int> optIndex = this->getEntityIndex(id);
	if (optIndex.has_value())
	{
		const int index = optIndex.value();

		// Clear entity slot.
		DebugAssertIndex(this->entities, index);
		this->entities[index].reset();
		this->validEntities[index] = false;

		// Clear ID mapping.
		this->indices.erase(id);

		// Add entity index to previously-owned slots list.
		this->freeIndices.push_back(index);
	}
	else
	{
		// Not in entity group.
		DebugLogWarning("Tried to remove missing entity \"" + std::to_string(id) + "\".");
	}
}

template <typename T>
void EntityManager::EntityGroup<T>::clear()
{
	this->entities.clear();
	this->validEntities.clear();
	this->indices.clear();
	this->freeIndices.clear();
}

const int EntityManager::NO_ID = -1;

EntityManager::EntityManager()
{
	this->nextID = 0;
}

int EntityManager::nextFreeID()
{
	// Check if any pre-owned entity IDs are available.
	if (this->freeIDs.size() > 0)
	{
		const int id = this->freeIDs.back();
		this->freeIDs.pop_back();
		return id;
	}
	else
	{
		// Get the next available ID.
		const int id = this->nextID;
		this->nextID++;
		return id;
	}
}

StaticEntity *EntityManager::makeStaticEntity()
{
	const int id = this->nextFreeID();
	StaticEntity *entity = this->staticGroup.addEntity(id);
	DebugAssert(entity->getID() == id);
	return entity;
}

DynamicEntity *EntityManager::makeDynamicEntity()
{
	const int id = this->nextFreeID();
	DynamicEntity *entity = this->dynamicGroup.addEntity(id);
	DebugAssert(entity->getID() == id);
	return entity;
}

Entity *EntityManager::get(int id)
{
	// Find which entity group the given ID is in.
	std::optional<int> entityIndex = this->staticGroup.getEntityIndex(id);
	if (entityIndex.has_value())
	{
		// Static entity.
		return this->staticGroup.getEntityAtIndex(entityIndex.value());
	}

	entityIndex = this->dynamicGroup.getEntityIndex(id);
	if (entityIndex.has_value())
	{
		// Dynamic entity.
		return this->dynamicGroup.getEntityAtIndex(entityIndex.value());
	}

	// Not in any entity group.
	return nullptr;
}

const Entity *EntityManager::get(int id) const
{
	// Find which entity group the given ID is in.
	std::optional<int> entityIndex = this->staticGroup.getEntityIndex(id);
	if (entityIndex.has_value())
	{
		// Static entity.
		return this->staticGroup.getEntityAtIndex(entityIndex.value());
	}

	entityIndex = this->dynamicGroup.getEntityIndex(id);
	if (entityIndex.has_value())
	{
		// Dynamic entity.
		return this->dynamicGroup.getEntityAtIndex(entityIndex.value());
	}

	// Not in any entity group.
	return nullptr;
}

int EntityManager::getCount(EntityType entityType) const
{
	switch (entityType)
	{
	case EntityType::Static:
		return this->staticGroup.getCount();
	case EntityType::Dynamic:
		return this->dynamicGroup.getCount();
	default:
		DebugUnhandledReturnMsg(int, std::to_string(static_cast<int>(entityType)));
	}
}

int EntityManager::getTotalCount() const
{
	return this->staticGroup.getCount() + this->dynamicGroup.getCount();
}

int EntityManager::getEntities(EntityType entityType, Entity **outEntities, int outSize)
{
	DebugAssert(outEntities != nullptr);
	DebugAssert(outSize >= 0);

	// Get entities from the desired type.
	switch (entityType)
	{
	case EntityType::Static:
		return this->staticGroup.getEntities(outEntities, outSize);
	case EntityType::Dynamic:
		return this->dynamicGroup.getEntities(outEntities, outSize);
	default:
		DebugUnhandledReturnMsg(int, std::to_string(static_cast<int>(entityType)));
	}
}

int EntityManager::getEntities(EntityType entityType, const Entity **outEntities, int outSize) const
{
	DebugAssert(outEntities != nullptr);
	DebugAssert(outSize >= 0);

	// Get entities from the desired type.
	switch (entityType)
	{
	case EntityType::Static:
		return this->staticGroup.getEntities(outEntities, outSize);
	case EntityType::Dynamic:
		return this->dynamicGroup.getEntities(outEntities, outSize);
	default:
		DebugUnhandledReturnMsg(int, std::to_string(static_cast<int>(entityType)));
	}
}

int EntityManager::getTotalEntities(const Entity **outEntities, int outSize) const
{
	DebugAssert(outEntities != nullptr);
	DebugAssert(outSize >= 0);

	// Fill the output buffer with as many entities as will fit.
	int writeIndex = 0;
	auto tryWriteEntities = [outEntities, outSize, &writeIndex](const auto &entityGroup)
	{
		const int entityCount = entityGroup.getCount();

		for (int i = 0; i < entityCount; i++)
		{
			// Break if the output buffer is full.
			if (writeIndex == outSize)
			{
				break;
			}

			outEntities[writeIndex] = entityGroup.getEntityAtIndex(i);
			writeIndex++;
		}
	};

	tryWriteEntities(this->staticGroup);
	tryWriteEntities(this->dynamicGroup);

	return writeIndex;
}

const EntityData *EntityManager::getEntityData(int flatIndex) const
{
	const auto iter = std::find_if(this->entityData.begin(), this->entityData.end(),
		[flatIndex](const EntityData &data)
	{
		return data.getFlatIndex() == flatIndex;
	});

	return (iter != this->entityData.end()) ? &(*iter) : nullptr;
}

EntityData *EntityManager::addEntityData(EntityData &&data)
{
	this->entityData.push_back(std::move(data));
	return &this->entityData.back();
}

void EntityManager::remove(int id)
{
	// Find which entity group the given ID is in.
	std::optional<int> entityIndex = this->staticGroup.getEntityIndex(id);
	if (entityIndex.has_value())
	{
		// Static entity.
		this->staticGroup.remove(id);

		// Insert entity ID into the free list.
		this->freeIDs.push_back(id);
		return;
	}

	entityIndex = this->dynamicGroup.getEntityIndex(id);
	if (entityIndex.has_value())
	{
		// Dynamic entity.
		this->dynamicGroup.remove(id);

		// Insert entity ID into the free list.
		this->freeIDs.push_back(id);
		return;
	}

	// Not in any entity group.
	DebugLogWarning("Tried to remove missing entity \"" + std::to_string(id) + "\".");
}

void EntityManager::clear()
{
	this->staticGroup.clear();
	this->dynamicGroup.clear();

	this->entityData.clear();

	this->freeIDs.clear();
	this->nextID = 0;
}

void EntityManager::tick(Game &game, double dt)
{
	auto tickEntityGroup = [&game, dt](auto &entityGroup)
	{
		const int entityCount = entityGroup.getCount();

		for (int i = 0; i < entityCount; i++)
		{
			auto *entity = entityGroup.getEntityAtIndex(i);
			if (entity != nullptr)
			{
				entity->tick(game, dt);
			}
		}
	};

	tickEntityGroup(this->staticGroup);
	tickEntityGroup(this->dynamicGroup);
}
