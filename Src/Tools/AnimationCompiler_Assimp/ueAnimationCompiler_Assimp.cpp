#include "AnimationCompiler_Common/ueToolAnimation.h"
#include "IO/ioFile.h"

#include <set>

// Assimp headers

#include "assimp.h"
#include "aiAssert.h"
#include "aiFileIO.h"
#include "aiPostProcess.h"
#include "aiScene.h"

static aiScene* s_scene = NULL;
static ueToolAnimation* s_dstAnim = NULL;
static const ueToolAnimation::ImportSettings* s_settings = NULL;

void ConvertAnimation()
{
	// Process single animation only (match by name or choose first if name not specified)

	for (u32 i = 0; i < s_scene->mNumAnimations; i++)
	{
		const aiAnimation* srcAnimation = s_scene->mAnimations[i];

		if (s_settings->m_sourceAnimationName.length() > 0 &&
			s_settings->m_sourceAnimationName != srcAnimation->mName.data)
			continue;

		s_dstAnim->m_sourceAnimationName = srcAnimation->mName.data;

		// Determine all unique times (among all key frames in all bones)

		std::set<f32> timesSet;
		for (u32 j = 0; j < srcAnimation->mNumChannels; j++)
		{
			const aiNodeAnim* srcBone = srcAnimation->mChannels[j];

			for (u32 k = 0; k < srcBone->mNumPositionKeys; k++)
				timesSet.insert((f32) srcBone->mPositionKeys[k].mTime);
			for (u32 k = 0; k < srcBone->mNumRotationKeys; k++)
				timesSet.insert((f32) srcBone->mRotationKeys[k].mTime);
			for (u32 k = 0; k < srcBone->mNumScalingKeys; k++)
				timesSet.insert((f32) srcBone->mScalingKeys[k].mTime);
		}

		std::vector<f32> times;
		for (std::set<f32>::iterator j = timesSet.begin(); j != timesSet.end(); j++)
			times.push_back(*j);

		std::sort(times.begin(), times.end());
		const f32 startFrameTime = s_settings->m_startFrame < times.size() ? times[s_settings->m_startFrame] : vector_last(times);
		const u32 endFrame = s_settings->m_startFrame + s_settings->m_numFrames - 1;
		const f32 endFrameTime = endFrame < times.size() ? times[endFrame] : vector_last(times);

		// Process all bones in animation

		for (u32 j = 0; j < srcAnimation->mNumChannels; j++)
		{
			const aiNodeAnim* srcBone = srcAnimation->mChannels[j];

			ueToolAnimation::Node* dstBone = s_dstAnim->GetAddNode(srcBone->mNodeName.data);

			dstBone->m_translationKeys.clear();
			dstBone->m_rotationKeys.clear();
			dstBone->m_scaleKeys.clear();

			u32 k;

#define ITER_KEYS(keys, count) \
			for (k = 0; (k < count && keys[k].mTime < startFrameTime); k++) {} \
			for (; (k < count && keys[k].mTime <= endFrameTime); k++)

			ITER_KEYS(srcBone->mPositionKeys, srcBone->mNumPositionKeys)
			{
				const aiVectorKey& srcKey = srcBone->mPositionKeys[k];
				dstBone->m_translationKeys.push_back(ueToolAnimation::Vec3Key((f32) srcKey.mTime, *(ueVec3*) &srcKey.mValue));
			}

			ITER_KEYS(srcBone->mRotationKeys, srcBone->mNumRotationKeys)
			{
				const aiQuatKey& srcKey = srcBone->mRotationKeys[k];
				const aiQuaternion& srcQuat = srcKey.mValue;
				const ueQuat dstQuat(srcQuat.x, srcQuat.y, srcQuat.z, srcQuat.w);
				dstBone->m_rotationKeys.push_back(ueToolAnimation::QuatKey((f32) srcKey.mTime, dstQuat));
			}

			ITER_KEYS(srcBone->mScalingKeys, srcBone->mNumScalingKeys)
			{
				const aiVectorKey& srcKey = srcBone->mScalingKeys[k];
				dstBone->m_scaleKeys.push_back(ueToolAnimation::Vec3Key((f32) srcKey.mTime, *(ueVec3*) &srcKey.mValue));
			}
		}

		// Do not process other animations

		break;
	}
}

bool ImportFromFile(ueToolAnimation& anim, const ueToolAnimation::ImportSettings& settings, const ueAssetParams& assetParams)
{
	s_dstAnim = &anim;
	s_settings = &settings;

	// Load Assimp scene and perform initial transformations on it

	s_scene = (aiScene*) aiImportFile(
		anim.m_sourcePath.c_str(),
		aiProcess_CalcTangentSpace		| // calculate tangents and bitangents
		aiProcess_JoinIdenticalVertices | // join identical vertices
		aiProcess_Triangulate			| // triangulate n-polygons
		aiProcess_GenSmoothNormals		| // generate smooth normal vectors if not existing
		aiProcess_ConvertToLeftHanded	| // convert everything to D3D left handed space
		aiProcess_LimitBoneWeights		| // limit per-vertex influences to max 4
		aiProcess_ValidateDataStructure | aiProcess_ImproveCacheLocality 
		| aiProcess_RemoveRedundantMaterials ); // validate the output data structure
	if (!s_scene)
	{
		ueLogE("Failed to load source animation file (path = '%s').", anim.m_sourcePath.c_str());
		return false;
	}

	// Convert

	ConvertAnimation();

	// Release Assimp scene

	aiReleaseImport(s_scene);
	s_scene = NULL;

	return true;
}