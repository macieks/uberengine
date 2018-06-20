#include "Base/Containers/ueHashMap.h"
#include "Base/ueRand.h"

typedef ueHashMap<const char*, const char*> SimpleMapType;

void PrintSimpleMap(SimpleMapType& map)
{
	ueHashContainerStats stats;
	map.GetStats(stats);
	ueLogI("Map [%d]: (%d collisions)", map.Size(), stats.m_numCollisions);
	SimpleMapType::Iterator i(map);
	while (i.Next())
	{
		const char* key = *i.Key();
		const char* value = *i.Value();
		ueLogI(" %s -> %s", key, value);
	}
}

void SimpleMapTest(ueAllocator* allocator)
{
	SimpleMapType map;
	UE_ASSERT_FUNC(map.Init(allocator, 16));

	map.Insert("some", "string");
	map.Insert("ala", "ma");
	map.Insert("kota", "psota");
	map.Insert("zupa", "jasio");
	map.Insert("psoci", "stasio");
	map.Insert("something", "funny");
	map.Insert("something", "stupid");
	map.Insert("uber", "engine");
	map.Insert("dragon", "game");
	map.Insert("yellow", "color");
	map.Insert("green", "color");
	map.Insert("red", "color");
	map.Insert("blue", "color");
	map.Insert("black", "color");
	map.Insert("white", "color");
	map.Insert("violet", "color");
	map.Insert("purple", "color");

	PrintSimpleMap(map);

	map.Remove("kota");
	map.Remove("something");

	PrintSimpleMap(map);

	map.Remove("ala");
	map.Remove("some");
	map.Remove("psoci");

	PrintSimpleMap(map);

	map.Deinit();
}

#define ADV_MAP_SIZE (1 << 16)
typedef ueHashMap<const char*, u32> AdvancedMapType;

#define ADV_KEY_LEN 16
typedef char AdvancedKey[ADV_KEY_LEN + 1];

void AdvancedMapTest(ueAllocator* allocator)
{
	ueTimer timer;

	// Generate random string keys

	ueRand* r = ueRand_GetGlobal();
	AdvancedKey* keys = (AdvancedKey*) allocator->Alloc(sizeof(AdvancedKey) * ADV_MAP_SIZE);
	UE_ASSERT(keys);

	timer.Restart();
	for (u32 i = 0; i < ADV_MAP_SIZE; i++)
	{
		char* key = (char*) keys[i];
		for (u32 j = 0; j < ADV_KEY_LEN; j += 4)
		{
			// Generate 4 chars at once

			union u8_4_u32
			{
				u8 m_u8[4];
				u32 m_u32;
			};
			u8_4_u32 u;
			u.m_u32 = ueRand_U32(r);
			for (u32 k = 0; k < 4; k++)
				u.m_u8[k] = 'a' + (char) (u.m_u8[k] % ('z' - 'a' + 1));

			// Copy 4 chars at once

			*(u32*) (key + j) = u.m_u32;
		}
		key[ADV_KEY_LEN] = 0;
	}
	const f32 keyGenTime = timer.GetSecs();

	// Insert keys
	AdvancedMapType map;
	UE_ASSERT_FUNC(map.Init(allocator, ADV_MAP_SIZE, ADV_MAP_SIZE));

	timer.Restart();
	for (u32 i = 0; i < ADV_MAP_SIZE; i++)
		map.Insert(keys[i], i);
	const f32 insertionTime = timer.GetSecs();

	// Verify all keys are present

	timer.Restart();
	for (u32 i = 0; i < ADV_MAP_SIZE; i++)
	{
		const u32* valuePtr = map.Find(keys[i]);
		UE_ASSERT(valuePtr);
		UE_ASSERT(*valuePtr == i);
	}
	const f32 findTime = timer.GetSecs();

	ueHashContainerStats stats;
	map.GetStats(stats);
	ueLogD("Statistics for hashmap with %d random strings, each %d characters:", map.Size(), ADV_KEY_LEN);
	ueLogD("  collisions: %d (%.3f %%)", stats.m_numCollisions, stats.m_collisionsPercentage);
	ueLogD("  longest list: %d", stats.m_longestList);
	ueLogD("  insertion time: %.6f secs", insertionTime);
	ueLogD("  find time: %.6f secs", findTime);
	ueLogD("  random key generation time: %.6f secs", keyGenTime);

	map.Deinit();
	allocator->Free(keys);
}

void UnitTest_HashMap(ueAllocator* allocator)
{
	SimpleMapTest(allocator);
	AdvancedMapTest(allocator);
}
