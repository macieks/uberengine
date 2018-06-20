#include "Base/ueRand.h"
#include "Base/Containers/ueGenericPool.h"

// MersenneTwister.h
// Mersenne Twister random number generator -- a C++ class MTRand
// Based on code by Makoto Matsumoto, Takuji Nishimura, and Shawn Cokus
// Richard J. Wagner  v1.1  28 September 2009  wagnerr@umich.edu

// The Mersenne Twister is an algorithm for generating random numbers.  It
// was designed with consideration of the flaws in various other generators.
// The period, 2^19937-1, and the order of equidistribution, 623 dimensions,
// are far greater.  The generator is also fast; it avoids multiplication and
// division, and it benefits from caches and pipelines.  For more information
// see the inventors' web page at
// http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html

// Reference
// M. Matsumoto and T. Nishimura, "Mersenne Twister: A 623-Dimensionally
// Equidistributed Uniform Pseudo-Random Number Generator", ACM Transactions on
// Modeling and Computer Simulation, Vol. 8, No. 1, January 1998, pp 3-30.

// Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
// Copyright (C) 2000 - 2009, Richard J. Wagner
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//   3. The names of its contributors may not be used to endorse or promote
//      products derived from this software without specific prior written
//      permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// The original code included the following notice:
//
//     When you use this, send an email to: m-mat@math.sci.hiroshima-u.ac.jp
//     with an appropriate reference to your work.
//
// It would be nice to CC: wagnerr@umich.edu and Cokus@math.washington.edu
// when you write.

#ifndef MERSENNETWISTER_H
#define MERSENNETWISTER_H

// Not thread safe (unless auto-initialization is avoided and each thread has
// its own MTRand object)

struct ueRand /* originally MTRand */ {
// Data
public:
	typedef unsigned long u32;  // unsigned integer type, at least 32 bits

	enum { N = 624 };       // length of state vector
	enum { SAVE = N + 1 };  // length of array for save()

protected:
	enum { M = 397 };  // period parameter

	u32 state[N];   // internal state
	u32 *pNext;     // next value to get from state
	s32 left;          // number of values left before reload needed

// Methods
public:
	ueRand( const u32 oneSeed );  // initialize with a simple u32
	ueRand( u32 *const bigSeed, u32 const seedLength = N );  // or array

	// Do NOT use for CRYPTOGRAPHY without securely hashing several returned
	// values together, otherwise the generator state can be learned after
	// reading 624 consecutive values.

	// Access to 32-bit random numbers
	u32 randInt();                     // integer in [0,2^32-1]
	u32 randInt( const u32 n );     // integer in [0,n] for n < 2^32
	f64 rand();                        // real number in [0,1]
	f64 rand( const f64 n );        // real number in [0,n]
	f64 randExc();                     // real number in [0,1)
	f64 randExc( const f64 n );     // real number in [0,n)
	f64 randDblExc();                  // real number in (0,1)
	f64 randDblExc( const f64 n );  // real number in (0,n)
	f64 operator()();                  // same as rand()

	// Access to 53-bit random numbers (capacity of IEEE f64 precision)
	f64 rand53();  // real number in [0,1)

	// Access to nonuniform random number distributions
	f64 randNorm( const f64 mean = 0.0, const f64 stddev = 1.0 );

	// Re-seeding functions with same behavior as initializers
	void seed( const u32 oneSeed );
	void seed( u32 *const bigSeed, const u32 seedLength = N );

protected:
	void initialize( const u32 oneSeed );
	void reload();
	u32 hiBit( const u32 u ) const { return u & 0x80000000UL; }
	u32 loBit( const u32 u ) const { return u & 0x00000001UL; }
	u32 loBits( const u32 u ) const { return u & 0x7fffffffUL; }
	u32 mixBits( const u32 u, const u32 v ) const
		{ return hiBit(u) | loBits(v); }
	u32 magic( const u32 u ) const
		{ return loBit(u) ? 0x9908b0dfUL : 0x0UL; }
	u32 twist( const u32 m, const u32 s0, const u32 s1 ) const
		{ return m ^ (mixBits(s0,s1)>>1) ^ magic(s1); }
};

// Functions are defined in order of usage to assist inlining

inline void ueRand::initialize( const u32 seed )
{
	// Initialize generator state with seed
	// See Knuth TAOCP Vol 2, 3rd Ed, p.106 for multiplier.
	// In previous versions, most significant bits (MSBs) of the seed affect
	// only MSBs of the state array.  Modified 9 Jan 2002 by Makoto Matsumoto.
	register u32 *s = state;
	register u32 *r = state;
	register s32 i = 1;
	*s++ = seed & 0xffffffffUL;
	for( ; i < N; ++i )
	{
		*s++ = ( 1812433253UL * ( *r ^ (*r >> 30) ) + i ) & 0xffffffffUL;
		r++;
	}
}

inline void ueRand::reload()
{
	// Generate N new values in state
	// Made clearer and faster by Matthew Bellew (matthew.bellew@home.com)
	static const s32 MmN = s32(M) - s32(N);  // in case enums are unsigned
	register u32 *p = state;
	register s32 i;
	for( i = N - M; i--; ++p )
		*p = twist( p[M], p[0], p[1] );
	for( i = M; --i; ++p )
		*p = twist( p[MmN], p[0], p[1] );
	*p = twist( p[MmN], p[0], state[0] );

	left = N, pNext = state;
}

inline void ueRand::seed( const u32 oneSeed )
{
	// Seed the generator with a simple u32
	initialize(oneSeed);
	reload();
}

inline void ueRand::seed( u32 *const bigSeed, const u32 seedLength )
{
	// Seed the generator with an array of u32's
	// There are 2^19937-1 possible initial states.  This function allows
	// all of those to be accessed by providing at least 19937 bits (with a
	// default seed length of N = 624 u32's).  Any bits above the lower 32
	// in each element are discarded.
	// Just call seed() if you want to get array from /dev/urandom
	initialize(19650218UL);
	register s32 i = 1;
	register u32 j = 0;
	register s32 k = ( N > seedLength ? (u32) N : seedLength );
	for( ; k; --k )
	{
		state[i] =
		state[i] ^ ( (state[i-1] ^ (state[i-1] >> 30)) * 1664525UL );
		state[i] += ( bigSeed[j] & 0xffffffffUL ) + j;
		state[i] &= 0xffffffffUL;
		++i;  ++j;
		if( i >= N ) { state[0] = state[N-1];  i = 1; }
		if( j >= seedLength ) j = 0;
	}
	for( k = N - 1; k; --k )
	{
		state[i] =
		state[i] ^ ( (state[i-1] ^ (state[i-1] >> 30)) * 1566083941UL );
		state[i] -= i;
		state[i] &= 0xffffffffUL;
		++i;
		if( i >= N ) { state[0] = state[N-1];  i = 1; }
	}
	state[0] = 0x80000000UL;  // MSB is 1, assuring non-zero initial array
	reload();
}

inline ueRand::ueRand( const u32 oneSeed )
	{ seed(oneSeed); }

inline ueRand::ueRand( u32 *const bigSeed, const u32 seedLength )
	{ seed(bigSeed,seedLength); }

inline ueRand::u32 ueRand::randInt()
{
	// Pull a 32-bit integer from the generator state
	// Every other access function simply transforms the numbers extracted here

	if( left == 0 ) reload();
	--left;

	register u32 s1;
	s1 = *pNext++;
	s1 ^= (s1 >> 11);
	s1 ^= (s1 <<  7) & 0x9d2c5680UL;
	s1 ^= (s1 << 15) & 0xefc60000UL;
	return ( s1 ^ (s1 >> 18) );
}

inline ueRand::u32 ueRand::randInt( const u32 n )
{
	// Find which bits are used in n
	// Optimized by Magnus Jonsson (magnus@smartelectronix.com)
	u32 used = n;
	used |= used >> 1;
	used |= used >> 2;
	used |= used >> 4;
	used |= used >> 8;
	used |= used >> 16;

	// Draw numbers until one is found in [0,n]
	u32 i;
	do
		i = randInt() & used;  // toss unused bits to shorten search
	while( i > n );
	return i;
}

inline f64 ueRand::rand()
	{ return f64(randInt()) * (1.0/4294967295.0); }

inline f64 ueRand::rand( const f64 n )
	{ return rand() * n; }

inline f64 ueRand::randExc()
	{ return f64(randInt()) * (1.0/4294967296.0); }

inline f64 ueRand::randExc( const f64 n )
	{ return randExc() * n; }

inline f64 ueRand::randDblExc()
	{ return ( f64(randInt()) + 0.5 ) * (1.0/4294967296.0); }

inline f64 ueRand::randDblExc( const f64 n )
	{ return randDblExc() * n; }

inline f64 ueRand::rand53()
{
	u32 a = randInt() >> 5, b = randInt() >> 6;
	return ( a * 67108864.0 + b ) * (1.0/9007199254740992.0);  // by Isaku Wada
}

inline f64 ueRand::randNorm( const f64 mean, const f64 stddev )
{
	// Return a real number from a normal (Gaussian) distribution with given
	// mean and standard deviation by polar form of Box-Muller transformation
	f64 x, y, r;
	do
	{
		x = 2.0 * rand() - 1.0;
		y = 2.0 * rand() - 1.0;
		r = x * x + y * y;
	}
	while ( r >= 1.0 || r == 0.0 );
	f64 s = sqrt( -2.0 * log(r) / r );
	return mean + x * s * stddev;
}

inline f64 ueRand::operator()()
{
	return rand();
}

#endif  // MERSENNETWISTER_H

struct ueRandMgrData
{
	ueGenericPool m_pool;
	ueRand* m_global;
};

static ueRandMgrData s_data;

void ueRandMgr_Startup(ueAllocator* stackAllocator, u32 maxRandoms)
{
	UE_ASSERT_FUNC(s_data.m_pool.Init(stackAllocator, sizeof(ueRand), maxRandoms + 1 /* global rand */));
	s_data.m_global = ueRand_Create();
}

void ueRandMgr_Shutdown()
{
	ueRand_Destroy(s_data.m_global);
	s_data.m_global = NULL;

	s_data.m_pool.Deinit();
}

ueRand*	ueRand_GetGlobal()
{
	return s_data.m_global;
}

ueRand*	ueRand_Create(u32 seed)
{
	return new(s_data.m_pool) ueRand(seed);
}

void ueRand_Destroy(ueRand* r)
{
	s_data.m_pool.Free(r);
}

void ueRand_Seed(ueRand* r, u32 seed)
{
	r->seed(seed);
}

u32 ueRand_U32(ueRand* r)
{
	return r->randInt();
}

u32 ueRand_U32(ueRand* r, u32 maxValue)
{
	return r->randInt(maxValue);
}

u32 ueRand_U32(ueRand* r, u32 minValue, u32 maxValue)
{
	return minValue + r->randInt(maxValue - minValue);
}

f32 ueRand_F32(ueRand* r)
{
	return (f32) r->rand();
}

f32 ueRand_F32(ueRand* r, f32 maxValue)
{
	return (f32) r->rand((f64) maxValue);
}

f32 ueRand_F32(ueRand* r, f32 minValue, f32 maxValue)
{
	return minValue + (f32) r->rand((f64) (maxValue - minValue));
}
