#include <bitset>
#include <vector>
#include "render.h"
#include "network/network.h"

namespace PortScanner
{


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
};

}