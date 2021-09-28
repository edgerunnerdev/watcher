//// This file is part of watcher.
////
//// watcher is free software: you can redistribute it and/or modify
//// it under the terms of the GNU General Public License as published by
//// the Free Software Foundation, either version 3 of the License, or
//// (at your option) any later version.
////
//// watcher is distributed in the hope that it will be useful,
//// but WITHOUT ANY WARRANTY; without even the implied warranty of
//// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//// GNU General Public License for more details.
////
//// You should have received a copy of the GNU General Public License
//// along with watcher. If not, see <https://www.gnu.org/licenses/>.
//
//#include "geolocation.h"
//
//#include <iostream>
//
//#include <curl/curl.h>
//#include <imgui/imgui.h>
//
//#include <stdio.h>
//
//IMPLEMENT_PLUGIN( Geolocation )
//
//// This needs to be a static function as libcurl is a C library and will segfault if passed
//// a local lambda.
//static size_t WriteMemoryCallback( void* pContents, size_t size, size_t nmemb, void* pUserData )
//{
//	size_t realSize = size * nmemb;
//	std::string& data = *reinterpret_cast< std::string* >( pUserData );
//	size_t curDataSize = data.size();
//	data.resize( curDataSize + realSize );
//	memcpy( &data[ curDataSize ], pContents, realSize );
//	return realSize;
//}
//
//Geolocation::Geolocation()
//{
//	m_RateLimitExceeded = false;
//}
//
//Geolocation::~Geolocation()
//{
//	if ( m_QueryThread.joinable() )
//	{
//		m_QueryThread.join();
//	}
//}
//
//bool Geolocation::Initialise( PluginMessageCallback pMessageCallback )
//{
//	m_pMessageCallback = pMessageCallback;
//	return true;
//}
//
//void Geolocation::OnMessageReceived( const nlohmann::json& message )
//{
//	if ( message[ "type" ] == "geolocation_request" )
//	{
//		{
//			std::lock_guard< std::mutex > lock( m_AccessMutex );
//			std::string address = message[ "ip_address" ];
//			m_Queue.emplace_back( address );
//		}
//
//		ConsumeQueue();
//	}
//}
//
//void Geolocation::DrawUI( ImGuiContext* pContext )
//{
//	ImGui::SetCurrentContext( pContext );
//
//	if ( ImGui::CollapsingHeader( "Geolocation", ImGuiTreeNodeFlags_DefaultOpen ) )
//	{
//		std::lock_guard< std::mutex > lock( m_AccessMutex );
//		ImGui::Text( "Provider: ipinfo.io" );
//		
//		if ( m_RateLimitExceeded )
//		{
//			ImGui::Text( "Rate limit exceeded." );
//		}
//		else
//		{
//			std::stringstream ss;
//			ss << "Queue size: " << m_Queue.size();
//			ImGui::Text( ss.str().c_str() );
//		}
//	}
//}
//
//// If required, create a thread which will keep consuming the queue until it is
//// empty. The thread is responsible for resolving the IP addresses into actual
//// locations and for sending back the "geolocation_result" message.
//void Geolocation::ConsumeQueue()
//{
//	if ( m_Queue.empty() || m_QueryThreadActive )
//	{
//		return;
//	}
//
//	auto threadMain = []( Geolocation* pGeolocation )
//	{
//		pGeolocation->m_QueryThreadActive = true;
//		while ( 1 )
//		{
//			Network::IPAddress address;
//			{
//				std::lock_guard< std::mutex > lock( pGeolocation->m_AccessMutex );
//				if ( pGeolocation->m_Queue.empty() )
//				{
//					break;
//				}
//				else
//				{
//					address = pGeolocation->m_Queue.back();
//					pGeolocation->m_Queue.pop_back();
//				}
//			}
//
//			pGeolocation->m_Data.clear();
//
//			std::stringstream url;
//			url << "https://ipinfo.io/" << address.GetHostAsString() << "/json"; 
//
//			CURL* pCurlHandle = curl_easy_init();
//			char pErrorBuffer[ CURL_ERROR_SIZE ];
//			curl_easy_setopt( pCurlHandle, CURLOPT_ERRORBUFFER, pErrorBuffer );
//			curl_easy_setopt( pCurlHandle, CURLOPT_URL, url.str().c_str() );
//			curl_easy_setopt( pCurlHandle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback );
//			curl_easy_setopt( pCurlHandle, CURLOPT_WRITEDATA, &pGeolocation->m_Data );
//			curl_easy_setopt( pCurlHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0" );
//			curl_easy_setopt( pCurlHandle, CURLOPT_TIMEOUT, 10L );
//
//			if ( curl_easy_perform( pCurlHandle ) != CURLE_OK )
//			{
//				json message = 
//				{
//					{ "type", "log" },
//					{ "level", "error" }, 
//					{ "plugin", "geolocation" },
//					{ "message", pErrorBuffer }
//				};
//				pGeolocation->m_pMessageCallback( message );
//			}
//			else
//			{
//				json message;
//				if ( pGeolocation->m_Data.find( "Rate limit exceeded." ) != std::string::npos )
//				{
//					message =
//					{
//						{ "type", "log" },
//						{ "level", "warning" }, 
//						{ "plugin", "geolocation" },
//						{ "message", "Rate limit exceeded." }
//					};
//					pGeolocation->m_RateLimitExceeded = true;
//				}
//				else
//				{
//					json data = json::parse( pGeolocation->m_Data );
//					if ( data.find( "city" ) != data.end() && 
//						 data.find( "region" ) != data.end() &&
//						 data.find( "country" ) != data.end() &&
//						 data.find( "org" ) != data.end() &&
//						 data.find( "loc" ) != data.end() )
//					{
//						message = 
//						{
//							{ "type", "geolocation_result" },
//							{ "address", address.ToString() },
//							{ "city", data[ "city" ] },
//							{ "region", data[ "region" ] },
//							{ "country", data[ "country" ] },
//							{ "org", data[ "org" ] },
//							{ "loc", data[ "loc" ] }
//						};
//					}
//					else
//					{
//						message =
//						{
//							{ "type", "log" },
//							{ "level", "error" }, 
//							{ "plugin", "geolocation" },
//							{ "message", "Error processing JSON response." }
//						};
//					}
//				}
//				pGeolocation->m_pMessageCallback( message );
//			}
//
//			curl_easy_cleanup( pCurlHandle );
//		}
//		pGeolocation->m_QueryThreadActive = false;
//	};
//
//	// Ensure any previous thread is fully terminated before starting a new one.
//	if ( m_QueryThread.joinable() )
//	{
//		m_QueryThread.join();
//	}
//
//	m_QueryThread = std::thread( threadMain, this );
//}