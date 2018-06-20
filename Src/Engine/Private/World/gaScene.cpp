#include "World/gaScene.h"
#include "World/gaActor.h"
#include "World/gaEffectComponent.h"
#include "World/gaLightComponent.h"
#include "World/gaModelComponent.h"

ueClassRegistry s_builtinGameplayModule[] =
{
	UE_CLASS_REGISTRY(gaActor),
	UE_CLASS_REGISTRY(gaComponent),

	UE_CLASS_REGISTRY(gaEffectComponent),
	UE_CLASS_REGISTRY(gaLightComponent),
	UE_CLASS_REGISTRY(gaModelComponent)
};