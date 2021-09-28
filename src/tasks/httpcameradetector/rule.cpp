//#include <algorithm>
//#include <cctype>
//#include "rule.h"
//
//bool Rule::Match(const std::string& url, const std::string& title) const
//{
//	if (title.size() == 0)
//	{
//		return false;
//	}
//
//	const int inTitleRulesPassed = ProcessInTitleRules(title);
//	const int inTextRulesPassed = ProcessInTextRules();
//
//	return (inTitleRulesPassed == m_InTitle.size() && inTextRulesPassed == m_InText.size());
//}
//
//void Rule::AddFilter(FilterType type, const std::string& filter)
//{
//	std::string loweredFilter;
//	loweredFilter.resize(filter.length());
//	std::transform(filter.begin(), filter.end(), loweredFilter.begin(), [](unsigned char c) -> unsigned char { return std::tolower(c); });
//
//	if (type == FilterType::InTitle)
//	{
//		m_InTitle.push_back(loweredFilter);
//	}
//	else if (type == FilterType::InText)
//	{
//		m_InText.push_back(loweredFilter);
//	}
//}
//
//int Rule::ProcessInTitleRules(const std::string& title) const
//{
//	std::string loweredTitle;
//	loweredTitle.resize(title.length());
//	std::transform(title.begin(), title.end(), loweredTitle.begin(), [](unsigned char c) -> unsigned char { return std::tolower(c); });
//
//	int rulesPassed = 0;
//	for (const std::string& inTitleRule : m_InTitle)
//	{
//		if (loweredTitle.find(inTitleRule) != std::string::npos)
//		{
//			rulesPassed++;
//		}
//	}
//	return rulesPassed;
//}
//
//int Rule::ProcessInTextRules() const
//{
//	// Not implemented.
//	return 0;
//}
