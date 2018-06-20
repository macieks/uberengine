#include "Utils/utCompression.h"

void UnitTest_Compression(ueAllocator* allocator)
{
    // Generate input data

    u32 data[1024];
    for (u32 i = 0; i < UE_ARRAY_SIZE(data); i++)
        data[i] = i;

    const ueSize size = sizeof(data);

    // Compress

    u32 compressionLibSymbol = UT_BEST_COMPRESSION_SYMBOL;
    u32 compressed[1024];
    ueSize compressedSize;
    UE_ASSERT_FUNC( utCompression_Compress(compressionLibSymbol, compressed, compressedSize, data, size) );

    UE_ASSERT(compressedSize < size);

    // Reset input data

    ueMemZeroS(data);

    // Decompress

    UE_ASSERT_FUNC( utCompression_Decompress(compressionLibSymbol, data, size, compressed, compressedSize) );

    // Verify result

    for (u32 i = 0; i < UE_ARRAY_SIZE(data); i++)
        UE_ASSERT(data[i] == i);
}
