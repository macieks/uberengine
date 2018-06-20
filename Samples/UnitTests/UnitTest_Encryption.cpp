#include "Multiplayer/mpEncryption.h"

#define check(op) { ueBool result = op; UE_ASSERT(result); }

void UnitTest_Encryption(ueAllocator* allocator)
{
	u32 length;

	const u32 UNIT_TEST_BUFFER_SIZE = 256;
	u8 buffer[UNIT_TEST_BUFFER_SIZE];

	const u32 UNIT_TEST_MSG0_SIZE = UNIT_TEST_BUFFER_SIZE - 8; // Keep it 8-byte aligned
	const u32 UNIT_TEST_MSG1_SIZE = UNIT_TEST_BUFFER_SIZE - 9; // Make it non 8-byte aligned

	// Initialize OpenSSL

	mpEncryptionHelper_StartupOpenSSL();

	// Initialize Alice and Bob

	mpEncryptionHelper* alice = mpEncryptionHelper_Create(allocator, UE_TRUE);
	mpEncryptionHelper* bob = mpEncryptionHelper_Create(allocator, UE_FALSE);
	UE_ASSERT(alice && bob);
	check(mpEncryptionHelper_IsAlice(alice));
	check(!mpEncryptionHelper_IsAlice(bob));

	// Alice generates exchange data and sends it to Bob

	check(mpEncryptionHelper_GetExchangeData(alice, buffer, UNIT_TEST_BUFFER_SIZE, &length));
	// [ sending... ]
	mpEncryptionHelper_MarkExchangeDataSent(alice);
	check(mpEncryptionHelper_IsExchangeDataSent(alice));

	// Bob receives exchange data and sends another exchange data to Alice

	check(mpEncryptionHelper_ReceiveExchangeData(bob, buffer, length));
	check(mpEncryptionHelper_IsExchangeDataReceived(bob));
	check(mpEncryptionHelper_GetExchangeData(bob, buffer, UNIT_TEST_BUFFER_SIZE, &length));
	// [ sending... ]
	mpEncryptionHelper_MarkExchangeDataSent(bob);
	check(mpEncryptionHelper_IsExchangeDataSent(bob));

	// Alice receives exchange data from Bob

	check(mpEncryptionHelper_ReceiveExchangeData(alice, buffer, length));

	// Authentication done!

	// Communication begins...

	// Bob encrypts data for Alice; Alice decrypts the data

	for (u32 i = 0; i < UNIT_TEST_MSG0_SIZE; i++)
		buffer[i] = i;
	check(mpEncryptionHelper_Encrypt(bob, buffer, UNIT_TEST_BUFFER_SIZE, UNIT_TEST_MSG0_SIZE, &length));
	check(mpEncryptionHelper_Decrypt(alice, buffer, length, &length));
	for (u32 i = 0; i < length; i++)
		check(buffer[i] == i);

	// Alice encrypts data for Bob; Bob decrypts the data

	for (u32 i = 0; i < UNIT_TEST_MSG1_SIZE; i++)
		buffer[i] = i;
	check(mpEncryptionHelper_Encrypt(alice, buffer, UNIT_TEST_BUFFER_SIZE, UNIT_TEST_MSG1_SIZE, &length));
	check(mpEncryptionHelper_Decrypt(bob, buffer, length, &length));
	for (u32 i = 0; i < length; i++)
		check(buffer[i] == i);

	// Shut down Alice and Bob

	mpEncryptionHelper_Destroy(alice);
	mpEncryptionHelper_Destroy(bob);
}