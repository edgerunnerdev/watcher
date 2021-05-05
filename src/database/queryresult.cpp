///////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////

#include "database/queryresult.h"

namespace Watcher
{

QueryResultType::QueryResultType( int value ) :
m_Int( value ),
m_Double( 0.0 )
{

}

QueryResultType::QueryResultType( double value ) :
m_Int( 0 ),
m_Double( value )
{

}

QueryResultType::QueryResultType( const std::string& value ) :
m_Int( 0 ),
m_Double( 0.0 )
{
	m_String = value;
}

int QueryResultType::GetInt() const
{
	return m_Int;
}

double QueryResultType::GetDouble() const
{
	return m_Double;
}

const std::string& QueryResultType::GetString() const
{
	return m_String;
}

} // namespace Watcher
