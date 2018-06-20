#ifndef MP_ENCRYPTION_H
#define MP_ENCRYPTION_H

#include "Base/ueBase.h"

/**
 *  @struct mpEncryptionHelper
 *  @brief Encryption helper to be used for communication between 2 peers (Alice and Bob).
 *    Internally uses Diffi-Helman to first establish shared key which is then used by Blowfish
 *    algorithm for regular data encryption/decryption. Uses OpenSSL for both algorithms.
 */
struct mpEncryptionHelper;

//! Starts up encryption helper including all dependant libraries used
void mpEncryptionHelper_Startup();
//! Shuts down encryption helper
void mpEncryptionHelper_Shutdown();

//! Creates encryption helper (either for Alice or Bob); it is up to user to decide who is Alice and who is Bob
mpEncryptionHelper* mpEncryptionHelper_Create(ueAllocator* allocator, ueBool isAlice);
//! Destroys encryption helper
void mpEncryptionHelper_Destroy(mpEncryptionHelper* helper);
//! Gets whether helper is used for Alice; otherwise it's used for Bob
ueBool mpEncryptionHelper_IsAlice(mpEncryptionHelper* helper);

//! Gets exchange data to be sent (first Alice to Bob, then, once received, Bob to Alice)
ueBool mpEncryptionHelper_GetExchangeData(mpEncryptionHelper* helper, u8* buffer, u32 bufferCapacity, u32* bufferSize);
//! Marks exchange data as successfully sent; this is so we can u32ernally deinitialize Diffi-Helman for Bob
void mpEncryptionHelper_MarkExchangeDataSent(mpEncryptionHelper* helper);
//! Gets whether exchange data was successfully sent
ueBool mpEncryptionHelper_IsExchangeDataSent(mpEncryptionHelper* helper);

//! To be invoked when exchange data is received
ueBool mpEncryptionHelper_ReceiveExchangeData(mpEncryptionHelper* helper, const u8* buffer, u32 bufferSize);
//! Gets whether exchange data has been successfully received
ueBool mpEncryptionHelper_IsExchangeDataReceived(mpEncryptionHelper* helper);

//! Gets whether regular data can now be received
ueBool mpEncryptionHelper_CanReceiveData(mpEncryptionHelper* helper);
//! Gets whether regular data can now be sent
ueBool mpEncryptionHelper_CanSendData(mpEncryptionHelper* helper);
//! Encrypts data in place
ueBool mpEncryptionHelper_Encrypt(mpEncryptionHelper* helper, u8* buffer, u32 bufferCapacity, u32 inputBufferSize, u32* outputBufferSize);
//! Decrypts data in place
ueBool mpEncryptionHelper_Decrypt(mpEncryptionHelper* helper, u8* buffer, u32 inputBufferSize, u32* outputBufferSize);

#endif // MP_ENCRYPTION_H