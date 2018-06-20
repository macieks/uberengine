#ifndef NS_IN_APPS_H
#define NS_IN_APPS_H

#include "Base/ueBase.h"

struct ueValue;

typedef void (*ntInAppPurchaseCallback)(u32 inAppIndex, ueBool success, void* userData);
typedef void (*ntInAppRestoreCallback)(ueBool success, void* userData);
typedef void (*ntInAppsCancelPurchaseCallback)(u32 inAppIndex, void* userData);

void ntInApps_Startup(ueAllocator* allocator, u32 numInApps, const char** inAppIds, ntInAppsCancelPurchaseCallback cancelCallback);
void ntInApps_Shutdown();

ueBool ntInApps_IsSupported();

void ntInApps_RestoreAll(ntInAppRestoreCallback callback, void* userData);
void ntInApps_Purchase(u32 inAppIndex, ntInAppPurchaseCallback callback, void* userData);
ueBool ntInApps_IsPurchased(u32 inAppIndex);

#ifdef UE_MARMALADE

ntInAppLib* ntiOSInApps_Register(const char** inAppIds);
void ntiOSInApps_Unregister();

ntInAppLib* ntAndroidMarketInApps_Register(const char** inAppIds);
void ntAndroidMarketInApps_Unregister();

#endif

#endif // NS_IN_APPS_H