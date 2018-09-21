#include <SDL.h>
#include "imgui/imgui.h"
#include "coverage.h"

namespace PortScanner
{

Coverage::Coverage() :
m_UI( true ),
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
				m_pUITextureData[ textureDataIndex     ] = 0;
				m_pUITextureData[ textureDataIndex + 1 ] = 0;
				m_pUITextureData[ textureDataIndex + 2 ] = 255;
			}
			else if ( m_BitSet.test( bitSetIndex ) )
			{
				m_pUITextureData[ textureDataIndex     ] = 0;
				m_pUITextureData[ textureDataIndex + 1 ] = 255;
				m_pUITextureData[ textureDataIndex + 2 ] = 0;
			}
			else
			{
				m_pUITextureData[ textureDataIndex     ] = 128;
				m_pUITextureData[ textureDataIndex + 1 ] = 0;
				m_pUITextureData[ textureDataIndex + 2 ] = 0;
			}
		}
	}

	glBindTexture( GL_TEXTURE_2D, m_UITexture );
	glTexSubImage2D( GL_TEXTURE_2D, 0 ,0, 0, m_UITextureWidth, m_UITextureHeight, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)m_pUITextureData );

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
	// TODO
	return false;
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
	m_FreeIndices.clear();
	// TODO
}

void Coverage::Write()
{
	// TODO
}

void Coverage::DrawUI()
{
	UpdateUserInterfaceTexture();

	ImGui::SetNextWindowSize( ImVec2( 512, 512 ), ImGuiCond_FirstUseEver );
	ImGui::Begin( "Port scanner coverage", &m_UI );

	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
	pDrawList->AddImage( reinterpret_cast< ImTextureID >( m_UITexture ), ImVec2( 0, 0 ), ImVec2( 512, 512 ) );

	ImGui::End();

}


}