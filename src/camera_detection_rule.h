#pragma once

#include <string>
#include <vector>

using StringVector = std::vector< std::string >;

class CameraDetectionRule
{
public:
	enum class FilterType
	{
		InTitle,
		InText
	};

	void AddFilter( FilterType type, const std::string& filter );
	
private:
	StringVector m_InTitle;
	StringVector m_InText;
};

inline void CameraDetectionRule::AddFilter( FilterType type, const std::string& filter )
{
	if ( type == FilterType::InTitle )
	{
		m_InTitle.push_back( filter );
	}
	else if ( type == FilterType::InText )
	{
		m_InText.push_back( filter );
	}
}