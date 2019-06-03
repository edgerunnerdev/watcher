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

#include <algorithm>
#include <array>
#include <fstream>
#include <sstream>
#include <SDL.h>
#include <imgui/imgui.h>
#include "coverage.h"

static const std::string sCoverageFilePath( "plugins/portscanner/coverage" );

Coverage::Coverage() :
m_UITexture( 0 ),
m_UITextureWidth( 256 ),
m_UITextureHeight( 256 ),
m_pUITextureData( nullptr ),
m_RebuildUITexture( true )
{
	// At the worst case, all indices in the bitmap will be free.
	m_FreeIndices.reserve( cBitSetSize );
	ClearBlockStates();

	CreateUserInterfaceTexture();
}

void Coverage::ClearBlockStates() 
{
	m_BitSet.reset();
	m_HasInProgressBlock = false;
	m_InProgressBlockIndex = 0;
	m_RebuildUITexture = true;
	for ( int i = 0; i < cBitSetSize; i++ )
	{
		m_FreeIndices.push_back( i );
	}
}

void Coverage::CreateUserInterfaceTexture()
{
	m_pUITextureData = new GLubyte[ m_UITextureWidth * m_UITextureHeight * 3 ];
	memset( m_pUITextureData, 0xFF, m_UITextureWidth * m_UITextureHeight * 3 * sizeof( GLubyte ) );

	glGenTextures( 1, &m_UITexture );
	glBindTexture( GL_TEXTURE_2D, m_UITexture );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, m_UITextureWidth, m_UITextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pUITextureData );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
}

void Coverage::UpdateUserInterfaceTexture()
{
	if ( m_RebuildUITexture == false )
	{
		return; 
	}

	for ( int y = 0; y < m_UITextureHeight; ++y )
	{
		for ( int x = 0; x < m_UITextureWidth; ++x )
		{
			IndexType bitSetIndex = y * m_UITextureWidth + x;
			int textureDataIndex = bitSetIndex * 3;
			if ( m_HasInProgressBlock && m_InProgressBlockIndex == bitSetIndex )
			{
				// Green
				m_pUITextureData[ textureDataIndex     ] = 0x00;
				m_pUITextureData[ textureDataIndex + 1 ] = 0xFF;
				m_pUITextureData[ textureDataIndex + 2 ] = 0x00;
			}

			else if ( m_BitSet.test( bitSetIndex ) )
			{
				// Cyan
				m_pUITextureData[ textureDataIndex     ] = 0x88;
				m_pUITextureData[ textureDataIndex + 1 ] = 0xFF;
				m_pUITextureData[ textureDataIndex + 2 ] = 0xD7;
			}
			else
			{
				// Dark blue
				m_pUITextureData[ textureDataIndex     ] = 0x0E;
				m_pUITextureData[ textureDataIndex + 1 ] = 0x11;
				m_pUITextureData[ textureDataIndex + 2 ] = 0x18;
			}
		}
	}

	glBindTexture( GL_TEXTURE_2D, m_UITexture );
	glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, m_UITextureWidth, m_UITextureHeight, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)m_pUITextureData );

	m_RebuildUITexture = false;
}

Coverage::BlockState Coverage::GetBlockState( const Network::IPAddress& ipAddress ) const
{
	IndexType index = IPAddressToIndex( ipAddress );
	if ( m_HasInProgressBlock && m_InProgressBlockIndex == index )
	{
		return Coverage::BlockState::InProgress;
	}
	return m_BitSet.test( index ) ? Coverage::BlockState::Scanned : Coverage::BlockState::NotScanned;
}

void Coverage::SetBlockState( const Network::IPAddress& ipAddress, Coverage::BlockState state )
{
	IndexType index = IPAddressToIndex( ipAddress );
	if ( state == Coverage::BlockState::InProgress )
	{
		m_HasInProgressBlock = true;
		m_InProgressBlockIndex = index;
	}
	else
	{
		if ( m_HasInProgressBlock && m_InProgressBlockIndex == index )
		{
			m_HasInProgressBlock = false;
			m_InProgressBlockIndex = 0;
		}
		
		if ( state == Coverage::BlockState::Scanned )
		{
			m_BitSet.set( index, true );
			std::vector< IndexType >::iterator it = std::find( m_FreeIndices.begin(), m_FreeIndices.end(), index );
			if ( it != m_FreeIndices.end() )
			{
				std::iter_swap( it, m_FreeIndices.end() - 1 );
				m_FreeIndices.pop_back();
			}
		}
		else if ( state == Coverage::BlockState::NotScanned )
		{
			m_BitSet.set( index, false );
			std::vector< IndexType >::iterator it = std::find( m_FreeIndices.begin(), m_FreeIndices.end(), index );
			if ( it == m_FreeIndices.end() )
			{
				m_FreeIndices.push_back( index );
			}
		}
	}

	m_RebuildUITexture = true;
}

bool Coverage::GetNextBlock( Network::IPAddress& ipAddress )
{
	if ( m_FreeIndices.empty() )
	{
		return false;
	}

	IndexType idx = m_FreeIndices[ rand() % m_FreeIndices.size() ];
	ipAddress = IndexToIPAddress( idx );
	ipAddress.SetBlock(16);
	return true;
}

Coverage::IndexType Coverage::IPAddressToIndex( const Network::IPAddress& ipAddress ) const
{
	unsigned int addr = ipAddress.GetHost();
	SDL_assert( ( addr & 0xFFFF ) == 0 ); // We expect a /16 CIDR block.
	return ( addr >> 16 ) & 0xFFFF;
}

Network::IPAddress Coverage::IndexToIPAddress( Coverage::IndexType index ) const
{
	return Network::IPAddress( static_cast< unsigned int >( index ) << 16, 0u );
}

void Coverage::Read()
{
	ClearBlockStates();
	
	std::ifstream fs( sCoverageFilePath, std::ios_base::in | std::ios_base::binary );
	if ( fs.good() )
	{
		fs.seekg( 0, fs.end );
		std::streamoff fileSize = fs.tellg();
		fs.seekg( 0, fs.beg );
		SDL_assert( fileSize == cCoverageFileSize );
		if ( fileSize == cCoverageFileSize )
		{
			std::array< unsigned char, cCoverageFileSize > buffer;
			fs.read( reinterpret_cast< char* >( buffer.data() ), cCoverageFileSize );

			unsigned char v = 0u;
			unsigned int bitSetIndex = 0u;
			for ( size_t s = 0u; s < cCoverageFileSize; ++s )
			{
				unsigned char v = static_cast< unsigned char >( buffer[ s ] );
				for ( int i = 0; i < 8; ++i )
				{
					const bool isSet = v & ( 0x80 >> i );
					m_BitSet.set( s * 8 + i, isSet );
				}
			}

		}
		fs.close();
	}
}

//-----------------------------------------------------------------------------
// Coverage::Write()
// Writes the coverage bitset to disk in binary format.
// There's no easy way to write out a std::bitset so we have to populate bytes
// individually before writing the resulting buffer to disk.
//-----------------------------------------------------------------------------
void Coverage::Write()
{
	static_assert( cCoverageFileSize == cBitSetSize / 8, "Size mismatch" );

	std::ofstream fs( sCoverageFilePath, std::ios_base::out | std::ios_base::binary );
	if ( fs.good() )
	{
		constexpr size_t cNumBytes = cBitSetSize / 8;
		std::array< char, cNumBytes > buffer;
		buffer.fill( 0 );

		unsigned char v = 0u;
		unsigned int bufferIndex = 0u;
		for ( size_t s = 0u; s < cBitSetSize; s++ )
		{
			if ( s != 0 && s % 8 == 0 )
			{
				buffer[ bufferIndex++ ] = v;
				v = 0x0;
			}

			if ( m_BitSet[ s ] )
			{
				int bitPosition = s % 8;
				v |= 0x80 >> bitPosition;
			}
		}

		fs.write( buffer.data(), cNumBytes * sizeof( char ));
		fs.close();
	}
	else
	{
		SDL_assert(false);
	}
}

void Coverage::DrawUI( bool& isWindowOpen )
{
	if ( !isWindowOpen )
	{
		return;
	}

	ImGui::SetNextWindowSize( ImVec2( 512, 512 ), ImGuiCond_FirstUseEver );
	if (!ImGui::Begin( "Port scanner coverage", &isWindowOpen ))
	{
		ImGui::End();
		return;
	}

	if (ImGui::Button( "Random address") )
	{
		Network::IPAddress addr;
		GetNextBlock(addr);
		SetBlockState( addr, BlockState::Scanned );
	}

	if ( ImGui::Button( "Write" ) )
	{
		Write();
	}

	if ( ImGui::Button( "Read" ) )
	{
		Read();
	}

	UpdateUserInterfaceTexture();

	ImGui::Image( reinterpret_cast< ImTextureID >( m_UITexture ), ImVec2( 512, 512 ) );
	ImGui::End();
}
