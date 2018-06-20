#include "AnimationCompiler_Common/ueToolAnimation.h"
#include "IO/ioFile.h"

#include <set>

// Collada

#include "FCollada.h"
#include "FCDocument/FCDocument.h"
#include "FCDocument/FCDSceneNode.h"
#include "FCDocument/FCDSceneNodeTools.h"
#include "FCDocument/FCDGeometry.h"
#include "FCDocument/FCDGeometrySource.h"
#include "FCDocument/FCDGeometryMesh.h"
#include "FCDocument/FCDGeometryPolygons.h"
#include "FCDocument/FCDGeometryPolygonsTools.h"
#include "FCDocument/FCDTexture.h"
#include "FCDocument/FCDMaterial.h"
#include "FCDocument/FCDMaterialInstance.h"
#include "FCDocument/FCDGeometryPolygonsInput.h"
#include "FCDocument/FCDEffectParameter.h"
#include "FCDocument/FCDEffectParameterSurface.h"
#include "FCDocument/FCDEffect.h"
#include "FCDocument/FCDEffectStandard.h"
#include "FCDocument/FCDEffectProfile.h"
#include "FCDocument/FCDEffectParameterSampler.h"
#include "FCDocument/FCDImage.h"
#include "FCDocument/FCDController.h"
#include "FCDocument/FCDControllerInstance.h"
#include "FCDocument/FCDSkinController.h"
#include "FCDocument/FCDocumentTools.h"
#include "FCDocument/FCDEffectTools.h"
#include "FCDocument/FCDLibrary.h"

static FCDocument* s_document = NULL;
static ueToolAnimation* s_dstAnim = NULL;
static const ueToolAnimation::ImportSettings* s_settings = NULL;

ueQuat FromColladaQuaternion(const FMQuaternion& q)
{
	return ueQuat(q.x, q.y, q.z, q.w);
}

ueVec3 FromColladaVec3(const FMVector3& v)
{
	return ueVec3(v.x, v.y, v.z);
}

void ProcessColladaBone(FCDSceneNode* srcNode)
{
	// Generate sampled keys and transforms

	FCDSceneNodeTools::ClearSampledAnimation();
	FCDSceneNodeTools::GenerateSampledAnimation(srcNode);

	const FloatList& keys = FCDSceneNodeTools::GetSampledAnimationKeys();
	const FMMatrix44List& transforms = FCDSceneNodeTools::GetSampledAnimationMatrices();
	if (keys.size() == 0)
		return;

	// Get node by name

	const char* nodeName = srcNode->GetName().c_str();
	ueToolAnimation::Node* dstNode = s_dstAnim->GetAddNode(nodeName);

	// Copy transforms to our bone

	for (u32 i = 0; i < keys.size(); i++)
	{
		const f32 time = keys[i];
		const ueMat44 transform((const f32*) transforms[i]);

		ueVec3 scale, translation;
		ueQuat rotation;
		transform.Decompose(scale, rotation, translation);

		dstNode->m_rotationKeys.push_back(ueToolAnimation::QuatKey(time, rotation));
		dstNode->m_translationKeys.push_back(ueToolAnimation::Vec3Key(time, translation));
		dstNode->m_scaleKeys.push_back(ueToolAnimation::Vec3Key(time, scale));
	}
}

void ProcessColladaSceneNode(FCDSceneNode* sceneNode)
{
	ProcessColladaBone(sceneNode);

	// Process child nodes

	for (u32 i = 0; i < sceneNode->GetChildrenCount(); i++)
		ProcessColladaSceneNode(sceneNode->GetChild(i));
}

void ConvertAnimation()
{
	// Process scene nodes

	FCDVisualSceneNodeLibrary* sceneLibrary = s_document->GetVisualSceneLibrary();
	for (u32 i = 0; i < sceneLibrary->GetEntityCount(); i++)
		ProcessColladaSceneNode(sceneLibrary->GetEntity(i));
}

bool ImportFromFile(ueToolAnimation& anim, const ueToolAnimation::ImportSettings& settings, const ueAssetParams& assetParams)
{
	s_dstAnim = &anim;
	s_settings = &settings;

	// Load Collada scene

	FCollada::Initialize();
	s_document = FCollada::NewTopDocument();
	ueLogI("Loading Collada document from '%s'...", anim.m_sourcePath.c_str());
	if (!FCollada::LoadDocumentFromFile(s_document, anim.m_sourcePath.c_str()))
	{
		ueLogE("Failed to load source animation file (path = '%s').", anim.m_sourcePath.c_str());

		s_document->Release();
		s_document = NULL;

		FCollada::Release();
		return false;
	}

	// Make Y axis up axis

	FCDocumentTools::StandardizeUpAxisAndLength(s_document, FMVector3(0, 1, 0));

	// Convert

	ConvertAnimation();

	// Release the scene

	s_document->Release();
	s_document = NULL;
	FCollada::Release();

	return true;
}