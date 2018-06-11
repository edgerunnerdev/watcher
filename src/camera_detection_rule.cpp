#include <algorithm>
#include <cctype>
#include "camera_detection_rule.h"

bool CameraDetectionRule::Match( const std::string& title ) const
{
	if ( title.size() == 0 )
	{
		return false;
	}

	std::string loweredTitle;
	loweredTitle.resize( title.length() );
	std::transform( title.begin(), title.end(), loweredTitle.begin(), [](unsigned char c) -> unsigned char { return std::tolower(c); });

	int titleRulesPassed = 0;
	for ( const std::string& inTitleRule : m_InTitle )
	{
		if ( loweredTitle.find( inTitleRule ) != std::string::npos )
		{
			titleRulesPassed++;
		}
	}

	return ( titleRulesPassed == m_InTitle.size() );
}

void CameraDetectionRule::AddFilter( FilterType type, const std::string& filter )
{
	std::string loweredFilter;
	loweredFilter.resize( filter.length() );
	std::transform( filter.begin(), filter.end(), loweredFilter.begin(), [](unsigned char c) -> unsigned char { return std::tolower(c); });

	if ( type == FilterType::InTitle )
	{
		m_InTitle.push_back( loweredFilter );
	}
	else if ( type == FilterType::InText )
	{
		m_InText.push_back( loweredFilter );
	}
}