#include "AnimationCompiler_Common/ueToolAnimation.h"

void ueToolAnimation::RemoveRedundantBones()
{
	u32 i = 0;
	while (i < m_nodes.size())
		if (m_nodes[i].m_rotationKeys.size() == 0 &&
			m_nodes[i].m_translationKeys.size() == 0 &&
			m_nodes[i].m_scaleKeys.size() == 0)
		{
			vector_remove_at(m_nodes, i);
		}
		else
			i++;
}

template <class TYPE>
void RemoveTrivialKeysT(TYPE& timeLine)
{
	for (u32 i = 0; i < timeLine.size(); i++)
	{
		if (timeLine.size() == 1)
			break;

		const bool equalBefore = (i == 0) || timeLine[i].m_value == timeLine[i - 1].m_value;
		if (!equalBefore)
			continue;
		const bool equalNext = i == timeLine.size() - 1 || timeLine[i].m_value == timeLine[i + 1].m_value;
		if (!equalNext)
			continue;

		vector_remove_at_preserve_order(timeLine, i);
		i--;
	}
}

void ueToolAnimation::RemoveTrivialKeys()
{
	for (u32 i = 0; i < m_nodes.size(); i++)
	{
		Node& node = m_nodes[i];
		RemoveTrivialKeysT(node.m_rotationKeys);
		RemoveTrivialKeysT(node.m_translationKeys);
		RemoveTrivialKeysT(node.m_scaleKeys);
	}
}

template <class TYPE>
void OffsetTimeLineT(TYPE& timeLine, f32 time)
{
	for (u32 i = 0; i < timeLine.size(); i++)
		timeLine[i].m_time += time;
}

void ueToolAnimation::CalculateAnimationLengthFromKeyFrames()
{
	// Determine min and max key-frame times

	f32 minTime = F32_MAX;
	f32 maxTime = F32_MIN;

	for (u32 i = 0; i < m_nodes.size(); i++)
	{
		const Node& node = m_nodes[i];

#define UPDATE_MIN_MAX_TIME(timeLine) \
		if (timeLine.size() > 0) \
		{ \
			minTime = ueMin(minTime, timeLine[0].m_time); \
			maxTime = ueMax(maxTime, timeLine[timeLine.size() - 1].m_time); \
		}

		UPDATE_MIN_MAX_TIME(node.m_rotationKeys);
		UPDATE_MIN_MAX_TIME(node.m_translationKeys);
		UPDATE_MIN_MAX_TIME(node.m_scaleKeys);
	}

	if (minTime > maxTime)
	{
		// This simply means there's no single key frame in any boneInfo

		m_length = 0;
		return;
	}

	m_length = maxTime - minTime;

	// Set track length appropriately for all bones and make all tracks start at 0

	if (minTime != 0.0f)
		for (u32 i = 0; i < m_nodes.size(); i++)
		{
			Node& node = m_nodes[i];
			OffsetTimeLineT(node.m_rotationKeys, -minTime);
			OffsetTimeLineT(node.m_translationKeys, -minTime);
			OffsetTimeLineT(node.m_scaleKeys, -minTime);
		}
}

template <class TYPE>
void ScaleTimeLineT(std::vector<TYPE>& timeLine, f32 scale)
{
	for (u32 i = 0; i < timeLine.size(); i++)
		timeLine[i].m_time *= scale;
}

void ueToolAnimation::ScaleTimeLines(f32 scale)
{
	for (u32 i = 0; i < m_nodes.size(); i++)
	{
		Node& node = m_nodes[i];
		ScaleTimeLineT(node.m_rotationKeys, scale);
		ScaleTimeLineT(node.m_translationKeys, scale);
		ScaleTimeLineT(node.m_scaleKeys, scale);
	}
}

template <typename TYPE, typename LERPER_TYPE>
void ResampleT(std::vector<TYPE>& timeLine, f32 length, u32 numDstSamples)
{
	std::vector<TYPE> newTimeLine(numDstSamples);
	u32 index = 0;
	f32 time = 0.0f;
	const f32 timeStep = length / (numDstSamples - 1);
	for (u32 i = 0; i < numDstSamples; i++)
	{
		if (time <= timeLine[0].m_time)
			newTimeLine[i].m_value = timeLine[0].m_value;
		else if (vector_last(timeLine).m_time <= time || index == timeLine.size())
			newTimeLine[i].m_value = vector_last(timeLine).m_value;
		else
		{
			const TYPE& a = timeLine[index - 1];
			const TYPE& b = timeLine[index];
			const f32 scale = (time - a.m_time) / (b.m_time - a.m_time);
			LERPER_TYPE::Lerp(newTimeLine[i].m_value, a.m_value, b.m_value, scale);
		}

		newTimeLine[i].m_time = time;

		time += timeStep;
		time = ueMin(time, length);

		while (index < timeLine.size() && timeLine[index].m_time < time)
			index++;
	}

	timeLine = newTimeLine;
}

template <typename TYPE, typename LERPER_TYPE>
void ResampleT(std::vector<TYPE>& timeLine, f32 length, f32 minSampleFrequency)
{
	if (timeLine.size() <= 1)
		return;

	// Determine reasonable sampling frequency for this node

	f32 frequency = F32_MAX;
	for (u32 j = 0; j < timeLine.size() - 1; j++)
		frequency = ueMin(frequency, timeLine[j + 1].m_time - timeLine[j].m_time);
	frequency = ueMax(frequency, minSampleFrequency);

	// Fix up the frequency to produce integer number of samples with first at time 0 and last at time ueToolAnimation::m_length

	const u32 numSamples = ueMax((u32) 1, (u32) ueCeil(length / frequency + 1.0f));

	// Generate final samples

	ResampleT<TYPE, LERPER_TYPE>(timeLine, length, numSamples);
}

void ueToolAnimation::Resample(f32 minSampleFrequency)
{
	UE_ASSERT(m_length > 0);

	for (u32 i = 0; i < m_nodes.size(); i++)
	{
		Node& node = m_nodes[i];
		ResampleT<QuatKey, ueQuatLerper>(node.m_rotationKeys, m_length, minSampleFrequency);
		ResampleT<Vec3Key, ueVec3Lerper>(node.m_translationKeys, m_length, minSampleFrequency);
		ResampleT<Vec3Key, ueVec3Lerper>(node.m_scaleKeys, m_length, minSampleFrequency);
	}
}