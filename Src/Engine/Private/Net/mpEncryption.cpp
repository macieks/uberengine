#include "Net/mpEncryption.h"
#include "Base/ueRand.h"

#include <openssl/rand.h>
#include <openssl/dh.h>
#include <openssl/blowfish.h>

struct mpEncryptionHelper
{
	ueAllocator* m_allocator;

	ueBool m_isAlice;				// Indicates whether we're Alice; otherwise we're Bob

	DH* m_dh;						// Diffi-Helman OpenSSL context; valid only during key exchange
	ueBool m_isExchangeDataSent;	// Indicates whether exchange data was successfully sent to Bob; valid for Alice only
	ueBool m_isExchangeDataRecved;	// Indicates whether exchange data was successfully received from Alice; valid for Bob only

	BF_KEY m_blowfish;				// Blowfish key; valid once key exchange is finalized
};

static ueAllocator* s_allocator = NULL;

void* mpEncryptionHelper_Malloc(ueSize size)
{
	return s_allocator->Alloc(size);
}

void* mpEncryptionHelper_Realloc(void* ptr, ueSize size)
{
	return s_allocator->Realloc(ptr, size);
}

void mpEncryptionHelper_Free(void* ptr)
{
	return s_allocator->Free(ptr);
}

void mpEncryptionHelper_Startup(ueAllocator* allocator)
{
	UE_ASSERT(!s_allocator);
	s_allocator = allocator;

	CRYPTO_set_mem_functions(mpEncryptionHelper_Malloc, mpEncryptionHelper_Realloc, mpEncryptionHelper_Free);
	const u32 randSeed = ueRandG_U32();
	RAND_seed(&randSeed, sizeof(randSeed));
}

void mpEncryptionHelper_Shutdown()
{
	UE_ASSERT(s_allocator);
	s_allocator = NULL;
}

mpEncryptionHelper* mpEncryptionHelper_Create(ueAllocator* allocator, ueBool isAlice)
{
	mpEncryptionHelper* helper = new(allocator) mpEncryptionHelper;
	if (!helper)
		return NULL;
	helper->m_allocator = allocator;
	helper->m_dh = NULL;
	helper->m_isAlice = isAlice;
	helper->m_isExchangeDataSent = UE_FALSE;
	helper->m_isExchangeDataRecved = UE_FALSE;
	return helper;
}

void mpEncryptionHelper_Destroy(mpEncryptionHelper* helper)
{
	if (helper->m_dh)
		DH_free(helper->m_dh);
	ueDelete(helper, helper->m_allocator);
}

ueBool mpEncryptionHelper_IsAlice(mpEncryptionHelper* helper)
{
	return helper->m_isAlice;
}

ueBool mpEncryptionHelper_VerifyDiffiHelman(mpEncryptionHelper* helper)
{
	s32 flags;
	if (!DH_check(helper->m_dh, &flags))
		return UE_FALSE;
	return flags ? UE_FALSE : UE_TRUE;
}

ueBool mpEncryptionHelper_WriteNum(const BIGNUM* num, u8* buffer, u32 bufferCapacity, u32* bufferSize)
{
	if (*bufferSize + 1 >= bufferCapacity)
		return UE_FALSE;

	// Write number length followed by the number data

	u8* numLengthPtr = buffer + *bufferSize;
	u8* dataPtr = buffer + *bufferSize + 1;
	*numLengthPtr = (u8) BN_bn2bin(num, dataPtr);

	*bufferSize += 1 + *numLengthPtr;
	UE_ASSERT(*bufferSize <= bufferCapacity);

	return UE_TRUE;
}

BIGNUM* mpEncryptionHelper_ReadNum(const u8*& currPtr, const void* endPtr)
{
	// Read number length

	if ((u8*) currPtr + 1 >= endPtr)
		return NULL;

	const u8* numLengthPtr = currPtr;
	currPtr++;

	// Read data and create number

	if (currPtr + *numLengthPtr > endPtr)
		return NULL;

	BIGNUM* num = BN_bin2bn(currPtr, *numLengthPtr, NULL);
	if (!num)
		return NULL;
	currPtr += *numLengthPtr;

	return num;
}

ueBool mpEncryptionHelper_GetExchangeData(mpEncryptionHelper* helper, u8* buffer, u32 bufferCapacity, u32* bufferSize)
{
	UE_ASSERT((helper->m_isAlice && !helper->m_isExchangeDataSent) || (!helper->m_isAlice && helper->m_isExchangeDataRecved));

	*bufferSize = 0;

	if (helper->m_isAlice) // Bob initializes Diffi-Helman when exchange data gets received
	{
		// Initialize Diffi-Helman if not done before

		if (!helper->m_dh)
		{
			helper->m_dh = DH_new();
			if (!helper->m_dh)
				return UE_FALSE;
			helper->m_dh->flags &= ~DH_FLAG_NO_EXP_CONSTTIME;

			// Generate P and G

			if (!DH_generate_parameters_ex(helper->m_dh, 256, DH_GENERATOR_5, NULL))
			{
				DH_free(helper->m_dh);
				helper->m_dh = NULL;
				return UE_FALSE;
			}

			// Verify P and G

			if (!mpEncryptionHelper_VerifyDiffiHelman(helper))
			{
				DH_free(helper->m_dh);
				helper->m_dh = NULL;
				return UE_FALSE;
			}

			// Generate public and private keys

			if (!DH_generate_key(helper->m_dh))
			{
				DH_free(helper->m_dh);
				helper->m_dh = NULL;
				return UE_FALSE;
			}
		}

		// Write P and G numbers

		if (!mpEncryptionHelper_WriteNum(helper->m_dh->p, buffer, bufferCapacity, bufferSize) ||
			!mpEncryptionHelper_WriteNum(helper->m_dh->g, buffer, bufferCapacity, bufferSize))
			return UE_FALSE;
	}

	// Write public key

	return mpEncryptionHelper_WriteNum(helper->m_dh->pub_key, buffer, bufferCapacity, bufferSize);
}

void mpEncryptionHelper_MarkExchangeDataSent(mpEncryptionHelper* helper)
{
	UE_ASSERT(!helper->m_isExchangeDataSent);
	helper->m_isExchangeDataSent = UE_TRUE;

	if (!helper->m_isAlice)
	{
		UE_ASSERT(helper->m_dh);
		DH_free(helper->m_dh);
		helper->m_dh = NULL;
	}
}

ueBool mpEncryptionHelper_IsExchangeDataSent(mpEncryptionHelper* helper)
{
	return helper->m_isExchangeDataSent;
}

ueBool mpEncryptionHelper_ReceiveExchangeData(mpEncryptionHelper* helper, const u8* buffer, u32 bufferSize)
{
	UE_ASSERT(!helper->m_isExchangeDataRecved);
	UE_ASSERT(!helper->m_isAlice || helper->m_isExchangeDataSent);

	const u8* bufferEnd = buffer + bufferSize;

	// Read P and G if Bob (Alice already did it)

	if (!helper->m_isAlice)
	{
		UE_ASSERT(!helper->m_dh);

		helper->m_dh = DH_new();
		if (!helper->m_dh)
			return UE_FALSE;
		helper->m_dh->flags |= DH_FLAG_NO_EXP_CONSTTIME;

		// Read P and G

		helper->m_dh->p = mpEncryptionHelper_ReadNum(buffer, bufferEnd);
		if (!helper->m_dh->p)
		{
			DH_free(helper->m_dh);
			helper->m_dh = NULL;
			return UE_FALSE;
		}

		helper->m_dh->g = mpEncryptionHelper_ReadNum(buffer, bufferEnd);
		if (!helper->m_dh->g)
		{
			DH_free(helper->m_dh);
			helper->m_dh = NULL;
			return UE_FALSE;
		}

		// Verify P and G

		if (!mpEncryptionHelper_VerifyDiffiHelman(helper))
		{
			DH_free(helper->m_dh);
			helper->m_dh = NULL;
			return UE_FALSE;
		}

		// Generate public and private keys

		if (!DH_generate_key(helper->m_dh))
		{
			DH_free(helper->m_dh);
			helper->m_dh = NULL;
			return UE_FALSE;
		}
	}

	// Read public key

	BIGNUM* publicKey = mpEncryptionHelper_ReadNum(buffer, bufferEnd);
	if (!publicKey)
		return UE_FALSE;

	// Check end of buffer

	if (buffer != bufferEnd)
	{
		BN_free(publicKey);
		return UE_FALSE;
	}

	// Generate symmetric key to be used by Blowfish algorithm

	const u32 MAX_BLOWFISH_KEY_BITS = 1024;
	u8 symmetricKey[MAX_BLOWFISH_KEY_BITS / 8];

	const u32 symmetricKeyLength = DH_size(helper->m_dh);
	if (MAX_BLOWFISH_KEY_BITS < symmetricKeyLength)
	{
		BN_free(publicKey);
		return UE_FALSE;
	}

	const u32 computeResult = DH_compute_key(symmetricKey, publicKey, helper->m_dh);
	if (computeResult != symmetricKeyLength)
	{
		BN_free(publicKey);
		return UE_FALSE;
	}

	// Deinitialize Diffi-Helman if Alice

	if (helper->m_isAlice)
	{
		DH_free(helper->m_dh);
		helper->m_dh = NULL;
	}

	// Public key isn't needed anymore

	BN_free(publicKey);

	// Initialize Blowfish with symmetric key

	BF_set_key(&helper->m_blowfish, symmetricKeyLength, symmetricKey);

	helper->m_isExchangeDataRecved = UE_TRUE;
	return UE_TRUE;
}

ueBool mpEncryptionHelper_IsExchangeDataReceived(mpEncryptionHelper* helper)
{
	return helper->m_isExchangeDataRecved;
}

ueBool mpEncryptionHelper_CanReceiveData(mpEncryptionHelper* helper)
{
	return helper->m_isExchangeDataRecved; // Remote peer did all it was supposed to do
}

ueBool mpEncryptionHelper_CanSendData(mpEncryptionHelper* helper)
{
	return helper->m_isExchangeDataSent; // We did all we were supposed to do
}

ueBool mpEncryptionHelper_Encrypt(mpEncryptionHelper* helper, u8* buffer, u32 bufferCapacity, u32 inputBufferSize, u32* outputBufferSize)
{
	UE_ASSERT(mpEncryptionHelper_CanSendData(helper));

	// Must align data to 64 bits (8 bytes) for Blowfish algorithm

	const u32 remainderLength = inputBufferSize & 7;
	if (remainderLength) // We'll fit remainder length in the last 8 bits (there's at least 1 byte unused!)
	{
		*outputBufferSize = inputBufferSize - remainderLength + 8;
		UE_ASSERT(*outputBufferSize <= bufferCapacity);
		buffer[*outputBufferSize - 1] = remainderLength;
	}
	else // Must add extra 8 bytes
	{
		*outputBufferSize = inputBufferSize + 8;
		UE_ASSERT(*outputBufferSize <= bufferCapacity);
		buffer[*outputBufferSize - 1] = 0; // No remainder
	}

	// Encrypt the data

	for (u32 i = 0; i < *outputBufferSize; i += 8)
		BF_encrypt((BF_LONG*) (buffer + i), &helper->m_blowfish);

	return UE_TRUE;
}

ueBool mpEncryptionHelper_Decrypt(mpEncryptionHelper* helper, u8* buffer, u32 inputBufferSize, u32* outputBufferSize)
{
	UE_ASSERT(mpEncryptionHelper_CanReceiveData(helper));

	// Properly encrypted data is always 8-byte (64 bits) aligned

	if (inputBufferSize & 7)
		return UE_FALSE;

	// Decrypt the data

	for (u32 i = 0; i < inputBufferSize; i += 8)
		BF_decrypt((BF_LONG*) (buffer + i), &helper->m_blowfish);

	// Reconstruct original buffer length and fix up buffer

	const u8 remainderLength = buffer[inputBufferSize - 1];
	if (remainderLength)
	{
		if (remainderLength > 7)
			return UE_FALSE;
		*outputBufferSize = inputBufferSize - (8 - remainderLength);
	}
	else
		*outputBufferSize = inputBufferSize - 8;

	return UE_TRUE;
}