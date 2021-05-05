// This file is part of watcher.
//
// watcher is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// watcher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with watcher. If not, see <https://www.gnu.org/licenses/>.

#include <bitset>
#include <random>
#include <vector>

// Needed to include GL.h properly.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <SDL_opengl.h>

#include "network/network.h"


//-----------------------------------------------------------------------------
// Coverage
// Keeps track of which IP blocks have been scanned already and provides 
//-----------------------------------------------------------------------------
class Coverage
{
public:
	Coverage();

	enum class BlockState
	{
		Scanned,
		NotScanned,
		InProgress
	};

	void ClearBlockStates();
	BlockState GetBlockState( const Network::IPAddress& ipAddress ) const;
	void SetBlockState( const Network::IPAddress& ipAddress, BlockState state );
	bool GetNextBlock( Network::IPAddress& ipAddress );

	void Read();
	void Write();

	void DrawUI( bool& isWindowOpen );

private:
	using IndexType = unsigned short;
	IndexType IPAddressToIndex( const Network::IPAddress& ipAddress ) const;
	Network::IPAddress IndexToIPAddress( IndexType index ) const;
	void CreateUserInterfaceTexture();
	void UpdateUserInterfaceTexture();

	static constexpr size_t cCoverageFileSize = 8192;
	static constexpr size_t cBitSetSize = 256 * 256;
	std::bitset< cBitSetSize > m_BitSet;
	std::vector< IndexType > m_FreeIndices;
	bool m_HasInProgressBlock;
	IndexType m_InProgressBlockIndex;
	GLuint m_UITexture;
	GLsizei m_UITextureWidth;
	GLsizei m_UITextureHeight;
	GLubyte* m_pUITextureData;
	bool m_RebuildUITexture;
	std::mt19937 m_MersenneTwister;
	std::uniform_int_distribution<unsigned int> m_Distribution;
};
