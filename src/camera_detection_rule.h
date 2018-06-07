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
	bool Match( const std::string& title ) const;
	
private:
	StringVector m_InTitle;
	StringVector m_InText;
};
