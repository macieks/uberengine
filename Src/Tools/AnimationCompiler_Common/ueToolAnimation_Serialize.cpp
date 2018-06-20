#include "AnimationCompiler_Common/ueToolAnimation.h"
#include "IO/ioPackageWriter.h"
#include "IO/ioPackageWriterUtils.h"

bool ueToolAnimation::Serialize(ioPackageWriter* pw)
{
	ioSegmentParams segmentParams;
	segmentParams.m_symbol = UE_BE_4BYTE_SYMBOL('s','k','e','l');
	ioSegmentWriter sw;
	pw->BeginSegment(sw, segmentParams);

	sw.WriteNumber<f32>(m_length);
	sw.WriteNumber<u32>((u32) m_nodes.size());

	sw.BeginPtr(sw.WritePtr());

	std::vector<ioPtr> namePtrs;

#define DECL_TIMELINE_PTRS(name) \
	std::vector<ioPtr> name##TimePtrs; \
	std::vector<ioPtr> name##KeyPtrs;

	DECL_TIMELINE_PTRS(rotation);
	DECL_TIMELINE_PTRS(translation);
	DECL_TIMELINE_PTRS(scale);

	for (u32 i = 0; i < m_nodes.size(); i++)
	{
		const Node* node = &m_nodes[i];

		namePtrs.push_back(sw.WritePtr());

#define WRITE_TIMELINE_INFO(name) \
		sw.WriteNumber<u32>((u32) node->m_##name##Keys.size()); \
		name##TimePtrs.push_back(sw.WritePtr()); \
		name##KeyPtrs.push_back(sw.WritePtr()); \
		sw.WriteNumber<f32>((f32) node->m_##name##Keys.size() / m_length);

		WRITE_TIMELINE_INFO(rotation);
		WRITE_TIMELINE_INFO(translation);
		WRITE_TIMELINE_INFO(scale);
	}

	for (u32 i = 0; i < m_nodes.size(); i++)
	{
		const Node* node = &m_nodes[i];

		sw.BeginPtr(namePtrs[i]);
		sw.WriteData(node->m_name.c_str(), node->m_name.length() + 1);
	}

	for (u32 i = 0; i < m_nodes.size(); i++)
	{
		const Node* node = &m_nodes[i];

#define WRITE_TIMELINE(name, typeName) \
	sw.BeginPtr(name##TimePtrs[i]); \
	for (u32 j = 0; j < node->m_##name##Keys.size(); j++) \
		sw.WriteNumber<f32>(node->m_##name##Keys[j].m_time); \
	sw.WriteAlignVec(); \
	sw.BeginPtr(name##KeyPtrs[i]); \
	for (u32 j = 0; j < node->m_##name##Keys.size(); j++) \
		Write##typeName(&sw, node->m_##name##Keys[j].m_value);

		WRITE_TIMELINE(rotation, Quat);
		WRITE_TIMELINE(translation, Vec3);
		WRITE_TIMELINE(scale, Vec3);
	}

	return true;
}