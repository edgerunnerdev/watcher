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
		InText,
		PageExists
	};

	void AddFilter( FilterType type, const std::string& filter );
	bool Match( const std::string& url, const std::string& title ) const;
	
private:
	int ProcessInTitleRules( const std::string& title ) const;
	int ProcessInTextRules() const;
	int ProcessPageExistsRules( const std::string& url ) const;

	StringVector m_InTitle;
	StringVector m_InText;
	StringVector m_PageExists;
};
