#ifndef GA_PREFAB_H
#define GA_PREFAB_H

#include "Base/ueResource.h"
#include "Base/Containers/ueVector.h"

class gaActor;

UE_DEFINE_RESOURCE_TYPE(gaPrefab, UE_BE_4BYTE_SYMBOL('p','r','f','b'));

ueBool gaPrefab_Create(const char* name, ueVector<gaActor*>& actors);

#endif // GA_PREFAB_H