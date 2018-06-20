#include "Utils/utValueInTime.h"
#include "IO/ioXml.h"

#define MAX_KEYS		100
#define MAX_COMPONENTS	4

void utValueInTime_Resample(u32 numSrcKeys, u32 numComponents, const f32* srcKeys, const f32* src, f32 dstKeyLength, u32 numDstKeys, f32* dst)
{
	u32 srcIndex = 0;

	u32 dstIndex = 0;
	f32 dstTime = 0.0f;

	while (dstIndex < numDstKeys)
	{
		// Determine lerp parameters

		u32 a, b;
		f32 scale;

		while (srcIndex + 1 < numSrcKeys && srcKeys[srcIndex] < dstTime)
			srcIndex++;
		const f32 srcTime = srcKeys[srcIndex];

		u32 prevSrcIndex;
		f32 prevSrcTime;
		if (srcIndex == 0)
		{
			prevSrcIndex = srcIndex;
			prevSrcTime = srcTime;
		}
		else
		{
			prevSrcIndex = srcIndex - 1;
			prevSrcTime = srcKeys[prevSrcIndex];
		}

		if (dstTime <= prevSrcTime)
		{
			a = b = prevSrcIndex;
			scale = 0.0f;
		}
		else if (srcTime <= dstTime)
		{
			a = b = srcIndex;
			scale = 0.0f;
		}
		else
		{
			a = prevSrcIndex;
			b = srcIndex;
			scale = (dstTime - prevSrcTime) / (srcTime - prevSrcTime);
		}

		// Calculate output

		a *= numComponents;
		b *= numComponents;

		f32* dstPtr = dst + dstIndex * numComponents;
		for (u32 i = 0; i < numComponents; i++)
			dstPtr[i] = ueLerp(src[a + i], src[b + i], scale);

		// Move to next destination key

		dstIndex++;
		dstTime += dstKeyLength;
	}
}

void utValueInTime_ReadValues(const char* valueString, u32 numComponents, f32* dst)
{
	f32 tmp[4];
	const u32 numParsed = ueStrScanf(valueString, "%f %f %f %f", &tmp[0], &tmp[1], &tmp[2], &tmp[3]);
	UE_ASSERT(numParsed > 0);

	const u32 numToCopy = ueMin(numParsed, numComponents);
	for (u32 i = 0; i < numToCopy; i++)
		dst[i] = tmp[i];

	for (u32 i = numToCopy; i < numComponents; i++)
		dst[i] = 0.0f;
}

utValueInTime* utValueInTime_CreateFromXmlNode(ueAllocator* allocator, ioXmlNode* node)
{
#ifdef UE_ENABLE_ASSERTION
	const char* nodeName = ioXmlNode_GetName(node);
	UE_ASSERT(!ueStrCmp(nodeName, "param"));
#endif

	u32 numComponents = 0;

	const char* type = ioXmlNode_GetAttrValue(node, "type");
	if (!ueStrCmp(type, "float")) numComponents = 1;
	else if (!ueStrCmp(type, "float2")) numComponents = 2;
	else if (!ueStrCmp(type, "float3")) numComponents = 3;
	else if (!ueStrCmp(type, "float4")) numComponents = 4;
	else
	{
		ueLogE("Unsupported value-in-time parameter type '%s'", type);
		return NULL;
	}

	// Constant?

	if (const char* valueString = ioXmlNode_GetAttrValue(node, "value"))
	{
		const u32 memorySize = sizeof(utValueInTime_Constant) + sizeof(f32) * numComponents;
		u8* memory = (u8*) allocator->Alloc(memorySize);
		UE_ASSERT(memory);

		utValueInTime_Constant* param = new(memory) utValueInTime_Constant();
		memory += sizeof(utValueInTime_Constant);
		param->m_numComponents = numComponents;
		param->m_value = (f32*) memory;

		utValueInTime_ReadValues(valueString, numComponents, param->m_value);

		return param;
	}

	// Constant with min/max range?

	if (const char* minValueString = ioXmlNode_GetAttrValue(node, "minValue"))
	{
		const char* maxValueString = ioXmlNode_GetAttrValue(node, "maxValue");
		UE_ASSERT(maxValueString);

		const u32 memorySize = sizeof(utValueInTime_ConstantRange) + sizeof(f32) * 2 * numComponents;
		u8* memory = (u8*) allocator->Alloc(memorySize);
		UE_ASSERT(memory);

		utValueInTime_ConstantRange* param = new(memory) utValueInTime_ConstantRange();
		memory += sizeof(utValueInTime_ConstantRange);
		param->m_numComponents = numComponents;
		param->m_minValue = (f32*) memory;
		memory += sizeof(f32) * numComponents;
		param->m_maxValue = (f32*) memory;

		utValueInTime_ReadValues(minValueString, numComponents, param->m_minValue);
		utValueInTime_ReadValues(maxValueString, numComponents, param->m_maxValue);

		return param;
	}

	// Must be timeline

	ioXmlNode* firstKey = ioXmlNode_GetFirstNode(node, "key");
	if (!firstKey)
	{
		ueLogE("Expected param's key elements not found");
		return NULL;
	}
	ueBool isRange;
	if (ioXmlNode_GetFirstAttr(firstKey, "value"))
		isRange = UE_FALSE;
	else if (ioXmlNode_GetFirstAttr(firstKey, "minValue") && ioXmlNode_GetFirstAttr(firstKey, "maxValue"))
		isRange = UE_TRUE;
	else
	{
		ueLogE("Expected 'value' or ('minValue' or 'maxValue') within param's key element");
		return NULL;
	}

	// Determine resampling parameters

	const u32 numSrcKeys = ioXmlNode_CalcNumNodes(node, "key");

	f32 length = 0.0f;
	for (ioXmlNode* keyNode = ioXmlNode_GetFirstNode(node, "key"); keyNode; keyNode = ioXmlNode_GetNext(keyNode, "key"))
	{
		f32 keyTime;
		UE_ASSERT_FUNC(ioXmlNode_GetAttrValueF32(keyNode, "time", keyTime));
		length = ueMax(length, keyTime);
	}

	f32 resampledKeyTime;
	ioXmlNode_GetAttrValueF32(node, "resampledKeyTime", resampledKeyTime, 0.1f);

	const u32 numDstKeys = (u32) ueCeil(length / resampledKeyTime) + 1;
	length = (numDstKeys - 1) * resampledKeyTime;

	// Read keys and resample

	f32 tmpKeys[MAX_KEYS];
	f32 tmpValues[MAX_KEYS * MAX_COMPONENTS];
	f32 tmpMaxValues[MAX_KEYS * MAX_COMPONENTS];
	UE_ASSERT(numSrcKeys <= MAX_KEYS);
	UE_ASSERT(numComponents <= MAX_COMPONENTS);

	if (isRange)
	{
		const u32 memorySize = sizeof(utValueInTime_TimeLineRange) + sizeof(f32) * 2 * numComponents * numDstKeys;
		u8* memory = (u8*) allocator->Alloc(memorySize);
		UE_ASSERT(memory);

		utValueInTime_TimeLineRange* param = new(memory) utValueInTime_TimeLineRange();
		memory += sizeof(utValueInTime_TimeLineRange);
		param->m_numComponents = numComponents;
		param->m_sampleTime = resampledKeyTime;
		param->m_numSamples = numDstKeys;
		param->m_minValues = (f32*) memory;
		memory += sizeof(f32) * numComponents * numDstKeys;
		param->m_maxValues = (f32*) memory;

		// Read source samples

		u32 srcKeyIndex = 0;
		for (ioXmlNode* keyNode = ioXmlNode_GetFirstNode(node, "key"); keyNode; keyNode = ioXmlNode_GetNext(keyNode, "key"))
		{
			UE_ASSERT_FUNC(ioXmlNode_GetAttrValueF32(keyNode, "time", tmpKeys[srcKeyIndex]));

			const char* minValueString = ioXmlNode_GetAttrValue(keyNode, "minValue");
			UE_ASSERT(minValueString);
			utValueInTime_ReadValues(minValueString, numComponents, &tmpValues[srcKeyIndex * numComponents]);

			const char* maxValueString = ioXmlNode_GetAttrValue(keyNode, "maxValue");
			UE_ASSERT(maxValueString);
			utValueInTime_ReadValues(maxValueString, numComponents, &tmpMaxValues[srcKeyIndex * numComponents]);

			srcKeyIndex++;
		}

		// Resample

		utValueInTime_Resample(numSrcKeys, numComponents, tmpKeys, tmpValues, resampledKeyTime, numDstKeys, param->m_minValues);
		utValueInTime_Resample(numSrcKeys, numComponents, tmpKeys, tmpMaxValues, resampledKeyTime, numDstKeys, param->m_maxValues);

		return param;
	}
	else
	{
		const u32 memorySize = sizeof(utValueInTime_TimeLine) + sizeof(f32) * numComponents * numDstKeys;
		u8* memory = (u8*) allocator->Alloc(memorySize);
		UE_ASSERT(memory);

		utValueInTime_TimeLine* param = new(memory) utValueInTime_TimeLine();
		memory += sizeof(utValueInTime_TimeLine);
		param->m_numComponents = numComponents;
		param->m_sampleTime = resampledKeyTime;
		param->m_numSamples = numDstKeys;
		param->m_values = (f32*) memory;

		// Read source samples

		u32 srcKeyIndex = 0;
		for (ioXmlNode* keyNode = ioXmlNode_GetFirstNode(node, "key"); keyNode; keyNode = ioXmlNode_GetNext(keyNode, "key"))
		{
			UE_ASSERT_FUNC(ioXmlNode_GetAttrValueF32(keyNode, "time", tmpKeys[srcKeyIndex]));

			const char* valueString = ioXmlNode_GetAttrValue(keyNode, "value");
			UE_ASSERT(valueString);
			utValueInTime_ReadValues(valueString, numComponents, &tmpValues[srcKeyIndex * numComponents]);

			srcKeyIndex++;
		}

		// Resample

		utValueInTime_Resample(numSrcKeys, numComponents, tmpKeys, tmpValues, resampledKeyTime, numDstKeys, param->m_values);

		return param;
	}

	return NULL;
}