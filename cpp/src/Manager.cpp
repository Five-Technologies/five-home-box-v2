//-----------------------------------------------------------------------------
//
//	Manager.h
//
//	Communicates with a Z-Wave network
//
//	Copyright (c) 2010 Mal Lansell <openzwave@lansell.org>
//
//	SOFTWARE NOTICE AND LICENSE
//
//	This file is part of OpenZWave.
//
//	OpenZWave is free software: you can redistribute it and/or modify
//	it under the terms of the GNU Lesser General Public License as published
//	by the Free Software Foundation, either version 3 of the License,
//	or (at your option) any later version.
//
//	OpenZWave is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public License
//	along with OpenZWave.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------------

#include "Defs.h"
#include "Manager.h"
#include "Driver.h"
#include "Node.h"
#include "Notification.h"

#include "Mutex.h"
#include "Event.h"
#include "Log.h"

#include "CommandClasses.h"
#include "CommandClass.h"

#include "ValueID.h"
#include "ValueBool.h"
#include "ValueButton.h"
#include "ValueByte.h"
#include "ValueDecimal.h"
#include "ValueInt.h"
#include "ValueList.h"
#include "ValueSchedule.h"
#include "ValueShort.h"
#include "ValueString.h"

using namespace OpenZWave;

Manager* Manager::s_instance = NULL;


//-----------------------------------------------------------------------------
//	Construction
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//	<Manager::Create>
//	Static creation of the singleton
//-----------------------------------------------------------------------------
Manager* Manager::Create
(
	string const& _configPath,
	string const& _userPath
)
{
	if( NULL == s_instance )
	{
		s_instance = new Manager( _configPath, _userPath );
	}

	return s_instance;
}

//-----------------------------------------------------------------------------
//	<Manager::Destroy>
//	Static method to destroy the singleton.
//-----------------------------------------------------------------------------
void Manager::Destroy
(
)
{
	delete s_instance;
	s_instance = NULL;
}

//-----------------------------------------------------------------------------
// <Manager::Manager>
// Constructor
//-----------------------------------------------------------------------------
Manager::Manager
( 
	string const& _configPath,
	string const& _userPath
):
	m_configPath( _configPath ),
	m_userPath( _userPath ),
	m_exitEvent( new Event() ),
	m_notificationMutex( new Mutex() )
{
	// Create the log file
	string logFilename = _userPath + string( "OZW_Log.txt" );
	Log::Create( logFilename );

	CommandClasses::RegisterCommandClasses();

	// Ensure the singleton instance is set
	s_instance = this;
}

//-----------------------------------------------------------------------------
// <Manager::Manager>
// Destructor
//-----------------------------------------------------------------------------
Manager::~Manager
(
)
{
	// Clear the pending list
	list<Driver*>::iterator pit = m_pendingDrivers.begin();
	while( !m_pendingDrivers.empty() )
	{
		delete *pit;
		m_pendingDrivers.erase( pit );
		pit = m_pendingDrivers.begin();
	}

	// Clear the ready map
	map<uint32,Driver*>::iterator rit = m_readyDrivers.begin();
	while( !m_readyDrivers.empty() )
	{
		delete rit->second;
		m_readyDrivers.erase( rit );
		rit = m_readyDrivers.begin();
	}
	
	delete m_exitEvent;
	delete m_notificationMutex;
	Log::Destroy();
}

//-----------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// <Manager::WriteConfig>
// Save the configuration of a driver to a file
//-----------------------------------------------------------------------------
void Manager::WriteConfig
(
	uint32 const _homeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		driver->WriteConfig();
		Log::Write( "Manager::WriteConfig completed for driver with home ID of 0x%.8x", _homeId );
	}
	else
	{
		Log::Write( "Manager::WriteConfig failed - _homeId %d not found", _homeId );
	}
}

//-----------------------------------------------------------------------------
//	Drivers
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// <Manager::AddDriver>
// Add a new Z-Wave PC Interface
//-----------------------------------------------------------------------------
bool Manager::AddDriver
(
	string const& _serialPortName
)
{
	// Make sure we don't already have a driver for this serial port
	
	// Search the pending list
	for( list<Driver*>::iterator pit = m_pendingDrivers.begin(); pit != m_pendingDrivers.end(); ++pit )
	{
		if( _serialPortName == (*pit)->GetSerialPortName() )
		{
			Log::Write( "Cannot add driver for serial port %s - driver already exists", _serialPortName.c_str() );
			return false;
		}
	}

	// Search the ready map
	for( map<uint32,Driver*>::iterator rit = m_readyDrivers.begin(); rit != m_readyDrivers.end(); ++rit )
	{
		if( _serialPortName == rit->second->GetSerialPortName() )
		{
			Log::Write( "Cannot add driver for serial port %s - driver already exists", _serialPortName.c_str() );
			return false;
		}
	}

	Driver* driver = new Driver( _serialPortName );
	m_pendingDrivers.push_back( driver );
	driver->Start();

	Log::Write( "Added driver for serial port %s", _serialPortName.c_str() );
	return true;
}

//-----------------------------------------------------------------------------
// <Manager::RemoveDriver>
// Remove a Z-Wave PC Interface
//-----------------------------------------------------------------------------
bool Manager::RemoveDriver
(
	string const& _serialPortName
)
{
	// Search the pending list
	for( list<Driver*>::iterator pit = m_pendingDrivers.begin(); pit != m_pendingDrivers.end(); ++pit )
	{
		if( _serialPortName == (*pit)->GetSerialPortName() )
		{
			delete *pit;
			m_pendingDrivers.erase( pit );
			Log::Write( "Driver for serial port %s removed", _serialPortName.c_str() );
			return true;
		}
	}

	// Search the ready map
	for( map<uint32,Driver*>::iterator rit = m_readyDrivers.begin(); rit != m_readyDrivers.end(); ++rit )
	{
		if( _serialPortName == rit->second->GetSerialPortName() )
		{
			delete rit->second;
			m_readyDrivers.erase( rit );
			Log::Write( "Driver for serial port %s removed", _serialPortName.c_str() );
			return true;
		}
	}

	Log::Write( "Failed to remove driver for serial port %s", _serialPortName.c_str() );
	return false;
}

//-----------------------------------------------------------------------------
// <Manager::GetDriver>
// Get a pointer to the driver for a Z-Wave PC Interface
//-----------------------------------------------------------------------------
Driver* Manager::GetDriver
(
	uint32 const _homeId
)
{
	map<uint32,Driver*>::iterator it = m_readyDrivers.find( _homeId );
	if( it != m_readyDrivers.end() )
	{
		return it->second;
	}

	assert(0);
	Log::Write( "Manager::GetDriver failed - Home ID 0x%.8x is unknown", _homeId );
	return NULL;
}

//-----------------------------------------------------------------------------
// <Manager::SetDriverReady>
// Move a driver from pending to ready, and notify any watchers
//-----------------------------------------------------------------------------
void Manager::SetDriverReady
(
	Driver* _driver
)
{
	// Search the pending list
	bool found = false;
	for( list<Driver*>::iterator it = m_pendingDrivers.begin(); it != m_pendingDrivers.end(); ++it )
	{
		if( (*it) == _driver )
		{
			// Remove the driver from the pending list
			m_pendingDrivers.erase( it );
			found = true;
			break;
		}
	}

	if( found )
	{
		Log::Write( "Driver with Home ID of 0x%.8x is now ready.", _driver->GetHomeId() );

		// Add the driver to the ready map
		m_readyDrivers[_driver->GetHomeId()] = _driver;

		// Notify the watchers
		Notification* notification = new Notification( Notification::Type_DriverReady );
		notification->SetHomeAndNodeIds( _driver->GetHomeId(), 0 );
		_driver->QueueNotification( notification ); 
	}
}

//-----------------------------------------------------------------------------
// <Manager::IsPrimaryController>
// 
//-----------------------------------------------------------------------------
bool Manager::IsPrimaryController
(
	uint32 const _homeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->IsPrimaryController();
	}

	Log::Write( "IsPrimaryController() failed - _homeId %d not found", _homeId );
	return false;
}

//-----------------------------------------------------------------------------
// <Manager::IsStaticUpdateController>
// 
//-----------------------------------------------------------------------------
bool Manager::IsStaticUpdateController
(
	uint32 const _homeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->IsStaticUpdateController();
	}

	Log::Write( "IsStaticUpdateController() failed - _homeId %d not found", _homeId );
	return false;
}

//-----------------------------------------------------------------------------
// <Manager::IsBridgeController>
// 
//-----------------------------------------------------------------------------
bool Manager::IsBridgeController
(
	uint32 const _homeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->IsBridgeController();
	}

	Log::Write( "IsBridgeController() failed - _homeId %d not found", _homeId );
	return false;
}

//-----------------------------------------------------------------------------
// <Manager::GetLibraryVersion>
// 
//-----------------------------------------------------------------------------
string Manager::GetLibraryVersion
(
	uint32 const _homeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->GetLibraryVersion();
	}

	Log::Write( "GetLibraryVersion() failed - _homeId %d not found", _homeId );
	return "";
}

//-----------------------------------------------------------------------------
// <Manager::GetLibraryTypeName>
// 
//-----------------------------------------------------------------------------
string Manager::GetLibraryTypeName
(
	uint32 const _homeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->GetLibraryTypeName();
	}

	Log::Write( "GetLibraryTypeName() failed - _homeId %d not found", _homeId );
	return "";
}

//-----------------------------------------------------------------------------
//	Polling Z-Wave values
//-----------------------------------------------------------------------------
				  		
//-----------------------------------------------------------------------------
// <Manager::SetPollInterval>
// Set the polling interval on all drivers
//-----------------------------------------------------------------------------
void Manager::SetPollInterval
(
	int32 _seconds
)
{
	for( list<Driver*>::iterator pit = m_pendingDrivers.begin(); pit != m_pendingDrivers.end(); ++pit )
	{
		(*pit)->SetPollInterval( _seconds );
	}

	for( map<uint32,Driver*>::iterator rit = m_readyDrivers.begin(); rit != m_readyDrivers.end(); ++rit )
	{
		rit->second->SetPollInterval( _seconds );
	}
}

//-----------------------------------------------------------------------------
// <Manager::EnablePoll>
// Enable polling of a value
//-----------------------------------------------------------------------------
bool Manager::EnablePoll
( 
	uint32 const _homeId,
	uint8 const _nodeId 
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return( driver->EnablePoll( _nodeId ) );
	}

	Log::Write( "EnablePoll failed - Driver with Home ID 0x%.8x is not available", _homeId );
	return false;
}

//-----------------------------------------------------------------------------
// <Manager::DisablePoll>
// Disable polling of a value
//-----------------------------------------------------------------------------
bool Manager::DisablePoll
( 
	uint32 const _homeId,
	uint8 const _nodeId 
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return( driver->DisablePoll( _nodeId ) );
	}

	Log::Write( "DisablePoll failed - Driver with Home ID 0x%.8x is not available", _homeId );
	return false;
}

//-----------------------------------------------------------------------------
//	Retrieving Node information
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// <Manager::RefreshNodeInfo>
// Fetch the data for a node from the Z-Wave network
//-----------------------------------------------------------------------------
bool Manager::RefreshNodeInfo
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		// Cause the node's data to be obtained from the Z-Wave network
		// in the same way as if it had just been added.
		driver->AddNodeInfoRequest( _nodeId );
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// <Manager::RequestNodeState>
// Fetch the command class data for a node from the Z-Wave network
//-----------------------------------------------------------------------------
void Manager::RequestNodeState
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		// Retreive the Node's Session and/or Dynamic data
		driver->RequestNodeState( _nodeId, CommandClass::RequestFlag_Session | CommandClass::RequestFlag_Dynamic );
	}
}

//-----------------------------------------------------------------------------
// <Manager::IsNodeListeningDevice>
// Get whether the node is a listening device that does not go to sleep
//-----------------------------------------------------------------------------
bool Manager::IsNodeListeningDevice
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	bool res = false;
	if( Driver* driver = GetDriver( _homeId ) )
	{
		res = driver->IsNodeListeningDevice( _nodeId );
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::IsNodeRoutingDevice>
// Get whether the node is a routing device that passes messages to other nodes
//-----------------------------------------------------------------------------
bool Manager::IsNodeRoutingDevice
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	bool res = false;
	if( Driver* driver = GetDriver( _homeId ) )
	{
		res = driver->IsNodeRoutingDevice( _nodeId );
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::GetNodeMaxBaudRate>
// Get the maximum baud rate of a node's communications
//-----------------------------------------------------------------------------
uint32 Manager::GetNodeMaxBaudRate
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	uint32 baud = 0;
	if( Driver* driver = GetDriver( _homeId ) )
	{
		baud = driver->GetNodeMaxBaudRate( _nodeId );
	}

	return baud;
}

//-----------------------------------------------------------------------------
// <Manager::GetNodeVersion>
// Get the version number of a node
//-----------------------------------------------------------------------------
uint8 Manager::GetNodeVersion
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	uint8 version = 0;
	if( Driver* driver = GetDriver( _homeId ) )
	{
		version = driver->GetNodeVersion( _nodeId );
	}

	return version;
}

//-----------------------------------------------------------------------------
// <Manager::GetNodeSecurity>
// Get the security byte for a node (bit meanings still to be determined)
//-----------------------------------------------------------------------------
uint8 Manager::GetNodeSecurity
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	uint8 security = 0;
	if( Driver* driver = GetDriver( _homeId ) )
	{
		security = driver->GetNodeSecurity( _nodeId );
	}

	return security;
}

//-----------------------------------------------------------------------------
// <Manager::GetNodeBasic>
// Get the basic type of a node
//-----------------------------------------------------------------------------
uint8 Manager::GetNodeBasic
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	uint8 basic = 0;
	if( Driver* driver = GetDriver( _homeId ) )
	{
		basic = driver->GetNodeBasic( _nodeId );
	}

	return basic;
}

//-----------------------------------------------------------------------------
// <Manager::GetNodeGeneric>
// Get the generic type of a node
//-----------------------------------------------------------------------------
uint8 Manager::GetNodeGeneric
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	uint8 genericType = 0;
	if( Driver* driver = GetDriver( _homeId ) )
	{
		genericType = driver->GetNodeGeneric( _nodeId );
	}

	return genericType;
}

//-----------------------------------------------------------------------------
// <Manager::GetNodeSpecific>
// Get the specific type of a node
//-----------------------------------------------------------------------------
uint8 Manager::GetNodeSpecific
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	uint8 specific = 0;
	if( Driver* driver = GetDriver( _homeId ) )
	{
		specific = driver->GetNodeSpecific( _nodeId );
	}

	return specific;
}

//-----------------------------------------------------------------------------
// <Manager::GetNodeType>
// Get a string describing the type of a node
//-----------------------------------------------------------------------------
string Manager::GetNodeType
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->GetNodeType( _nodeId );
	}

	return "Unknown";
}

//-----------------------------------------------------------------------------
// <Manager::GetNodeManufacturerName>
// Get the manufacturer name of a node
//-----------------------------------------------------------------------------
string Manager::GetNodeManufacturerName
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->GetNodeManufacturerName( _nodeId );
	}

	return "Unknown";
}

//-----------------------------------------------------------------------------
// <Manager::GetNodeProductName>
// Get the product name of a node
//-----------------------------------------------------------------------------
string Manager::GetNodeProductName
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->GetNodeProductName( _nodeId );
	}

	return "Unknown";
}

//-----------------------------------------------------------------------------
// <Manager::GetNodeName>
// Get the user-editable name of a node
//-----------------------------------------------------------------------------
string Manager::GetNodeName
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->GetNodeName( _nodeId );
	}

	return "Unknown";
}

//-----------------------------------------------------------------------------
// <Manager::GetNodeLocation>
// Get the location of a node
//-----------------------------------------------------------------------------
string Manager::GetNodeLocation
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->GetNodeLocation( _nodeId );
	}

	return "Unknown";
}

//-----------------------------------------------------------------------------
// <Manager::SetNodeManufacturerName>
// Set the manufacturer name a node
//-----------------------------------------------------------------------------
void Manager::SetNodeManufacturerName
(
	uint32 const _homeId,
	uint8 const _nodeId,
	string const& _manufacturerName
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		driver->SetNodeManufacturerName( _nodeId, _manufacturerName );
	}
}

//-----------------------------------------------------------------------------
// <Manager::SetNodeProductName>
// Set the product name of a node
//-----------------------------------------------------------------------------
void Manager::SetNodeProductName
(
	uint32 const _homeId,
	uint8 const _nodeId,
	string const& _productName
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		driver->SetNodeProductName( _nodeId, _productName );
	}
}

//-----------------------------------------------------------------------------
// <Manager::SetNodeName>
// Set the node name value with the specified ID
//-----------------------------------------------------------------------------
void Manager::SetNodeName
(
	uint32 const _homeId,
	uint8 const _nodeId,
	string const& _nodeName
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		driver->SetNodeName( _nodeId, _nodeName );
	}
}

//-----------------------------------------------------------------------------
// <Manager::SetNodeLocation>
// Set a string describing the location of a node
//-----------------------------------------------------------------------------
void Manager::SetNodeLocation
(
	uint32 const _homeId,
	uint8 const _nodeId,
	string const& _location

)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		driver->SetNodeLocation( _nodeId, _location );
	}
}

//-----------------------------------------------------------------------------
// <Manager::GetNodeManufacturerId>
// Get the manufacturer ID value of a node
//-----------------------------------------------------------------------------
string Manager::GetNodeManufacturerId
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->GetNodeManufacturerId( _nodeId );
	}

	return "Unknown";
}

//-----------------------------------------------------------------------------
// <Manager::GetNodeProductType>
// Get the product type value of a node
//-----------------------------------------------------------------------------
string Manager::GetNodeProductType
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->GetNodeProductType( _nodeId );
	}

	return "Unknown";
}

//-----------------------------------------------------------------------------
// <Manager::GetNodeProductId>
// Get the product Id value with the specified ID
//-----------------------------------------------------------------------------
string Manager::GetNodeProductId
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->GetNodeProductId( _nodeId );
	}

	return "Unknown";
}

//-----------------------------------------------------------------------------
// <Manager::SetNodeOn>
// Helper method to turn a node on
//-----------------------------------------------------------------------------
void Manager::SetNodeOn
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	SetNodeLevel( _homeId, _nodeId, 0xff );
}

//-----------------------------------------------------------------------------
// <Manager::SetNodeOff>
// Helper method to turn a node off
//-----------------------------------------------------------------------------
void Manager::SetNodeOff
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	SetNodeLevel( _homeId, _nodeId, 0 );
}

//-----------------------------------------------------------------------------
// <Manager::SetNodeLevel>
// Helper method to set the basic level of a node
//-----------------------------------------------------------------------------
void Manager::SetNodeLevel
(
	uint32 const _homeId,
	uint8 const _nodeId,
	uint8 const _level
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->SetNodeLevel( _nodeId, _level );
	}
}

//-----------------------------------------------------------------------------
//	Values
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// <Manager::GetValueLabel>
// Gets the user-friendly label for the value
//-----------------------------------------------------------------------------
string Manager::GetValueLabel
( 
	ValueID const& _id
)
{
	string label;
	if( Driver* driver = GetDriver( _id.GetHomeId() ) )
	{
		driver->LockNodes();
		if( Value* value = driver->GetValue( _id ) )
		{
			label = value->GetLabel();
		}
		driver->ReleaseNodes();
	}

	return label;
}

//-----------------------------------------------------------------------------
// <Manager::GetValueUnits>
// Gets the units that the value is measured in
//-----------------------------------------------------------------------------
string Manager::GetValueUnits
( 
	ValueID const& _id
)
{
	string units;
	if( Driver* driver = GetDriver( _id.GetHomeId() ) )
	{
		driver->LockNodes();
		if( Value* value = driver->GetValue( _id ) )
		{
			units = value->GetUnits();
		}
		driver->ReleaseNodes();
	}

	return units;
}

//-----------------------------------------------------------------------------
// <Manager::GetValueHelp>
// Gets a help string describing the value's purpose and usage
//-----------------------------------------------------------------------------
string Manager::GetValueHelp
( 
	ValueID const& _id
)
{
	string help;
	if( Driver* driver = GetDriver( _id.GetHomeId() ) )
	{
		driver->LockNodes();
		if( Value* value = driver->GetValue( _id ) )
		{
			help = value->GetHelp();
		}
		driver->ReleaseNodes();
	}

	return help;
}

//-----------------------------------------------------------------------------
// <Manager::GetValueMin>
// Gets the minimum for a value
//-----------------------------------------------------------------------------
int32 Manager::GetValueMin
( 
	ValueID const& _id
)
{
	int32 limit = 0;
	if( Driver* driver = GetDriver( _id.GetHomeId() ) )
	{
		driver->LockNodes();
		if( Value* value = driver->GetValue( _id ) )
		{
			limit = value->GetMin();
		}
		driver->ReleaseNodes();
	}

	return limit;
}

//-----------------------------------------------------------------------------
// <Manager::GetValueMax>
// Gets the maximum for a value
//-----------------------------------------------------------------------------
int32 Manager::GetValueMax
( 
	ValueID const& _id
)
{
	int32 limit = 0;
	if( Driver* driver = GetDriver( _id.GetHomeId() ) )
	{
		driver->LockNodes();
		if( Value* value = driver->GetValue( _id ) )
		{
			limit = value->GetMax();
		}
		driver->ReleaseNodes();
	}

	return limit;
}

//-----------------------------------------------------------------------------
// <Manager::IsValueReadOnly>
// Test whether the value is read-only
//-----------------------------------------------------------------------------
bool Manager::IsValueReadOnly
( 
	ValueID const& _id
)
{
	bool res = false;
	if( Driver* driver = GetDriver( _id.GetHomeId() ) )
	{
		driver->LockNodes();
		if( Value* value = driver->GetValue( _id ) )
		{
			res = value->IsReadOnly();
		}
		driver->ReleaseNodes();
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::IsValueSet>
// Test whether the value has been set by a status message from the device
//-----------------------------------------------------------------------------
bool Manager::IsValueSet
( 
	ValueID const& _id
)
{
	bool res = false;
	if( Driver* driver = GetDriver( _id.GetHomeId() ) )
	{
		driver->LockNodes();
		if( Value* value = driver->GetValue( _id ) )
		{
			res = value->IsSet();
		}
		driver->ReleaseNodes();
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::GetValueAsBool>
// Gets a value as a bool
//-----------------------------------------------------------------------------
bool Manager::GetValueAsBool
(
	ValueID const& _id,
	bool* o_value
)
{
	bool res = false;

	if( o_value )
	{
		if( ValueID::ValueType_Bool == _id.GetType() )
		{
			if( Driver* driver = GetDriver( _id.GetHomeId() ) )
			{
				driver->LockNodes();
				if( ValueBool* value = static_cast<ValueBool*>( driver->GetValue( _id ) ) )
				{
					*o_value = value->GetValue();
					res = true;
				}
				driver->ReleaseNodes();
			}
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::GetValueAsByte>
// Gets a value as an 8-bit unsigned integer
//-----------------------------------------------------------------------------
bool Manager::GetValueAsByte
(
	ValueID const& _id,
	uint8* o_value
)
{
	bool res = false;

	if( o_value )
	{
		if( ValueID::ValueType_Byte == _id.GetType() )
		{
			if( Driver* driver = GetDriver( _id.GetHomeId() ) )
			{
				driver->LockNodes();
				if( ValueByte* value = static_cast<ValueByte*>( driver->GetValue( _id ) ) )
				{
					*o_value = value->GetValue();
					res = true;
				}
				driver->ReleaseNodes();
			}
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::GetValueAsFloat>
// Gets a value as a floating point number
//-----------------------------------------------------------------------------
bool Manager::GetValueAsFloat
(
	ValueID const& _id,
	float* o_value
)
{
	bool res = false;

	if( o_value )
	{
		if( ValueID::ValueType_Decimal == _id.GetType() )
		{
			if( Driver* driver = GetDriver( _id.GetHomeId() ) )
			{
				driver->LockNodes();
				if( ValueDecimal* value = static_cast<ValueDecimal*>( driver->GetValue( _id ) ) )
				{
					string str = value->GetValue();
					*o_value = (float)atof( str.c_str() );
					res = true;
				}
				driver->ReleaseNodes();
			}
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::GetValueAsInt>
// Gets a value as a 32-bit signed integer
//-----------------------------------------------------------------------------
bool Manager::GetValueAsInt
(
	ValueID const& _id,
	int32* o_value
)
{
	bool res = false;

	if( o_value )
	{
		if( ValueID::ValueType_Int == _id.GetType() )
		{
			if( Driver* driver = GetDriver( _id.GetHomeId() ) )
			{
				driver->LockNodes();
				if( ValueInt* value = static_cast<ValueInt*>( driver->GetValue( _id ) ) )
				{
					*o_value = value->GetValue();
					res = true;
				}
				driver->ReleaseNodes();
			}
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::GetValueAsShort>
// Gets a value as a 16-bit signed integer
//-----------------------------------------------------------------------------
bool Manager::GetValueAsShort
(
	ValueID const& _id,
	int16* o_value
)
{
	bool res = false;

	if( o_value )
	{
		if( ValueID::ValueType_Short == _id.GetType() )
		{
			if( Driver* driver = GetDriver( _id.GetHomeId() ) )
			{
				driver->LockNodes();
				if( ValueShort* value = static_cast<ValueShort*>( driver->GetValue( _id ) ) )
				{
					*o_value = value->GetValue();
					res = true;
				}
				driver->ReleaseNodes();
			}
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::GetValueAsString>
// Creates a string representation of the value, regardless of type
//-----------------------------------------------------------------------------
bool Manager::GetValueAsString
(
	ValueID const& _id,
	string* o_value
)
{
	bool res = false;
	char str[256];

	if( o_value )
	{
		if( Driver* driver = GetDriver( _id.GetHomeId() ) )
		{
			driver->LockNodes();
		
			switch( _id.GetType() )
			{
				case ValueID::ValueType_Bool:
				{
					if( ValueBool* value = static_cast<ValueBool*>( driver->GetValue( _id ) ) )
					{
						*o_value = value->GetValue() ? "True" : "False";
						res = true;
					}
					break;
				}
				case ValueID::ValueType_Byte:
				{
					if( ValueByte* value = static_cast<ValueByte*>( driver->GetValue( _id ) ) )
					{
						snprintf( str, sizeof(str), "%u", value->GetValue() );
						*o_value = str;
						res = true;
					}
					break;
				}
				case ValueID::ValueType_Decimal:
				{
					if( ValueDecimal* value = static_cast<ValueDecimal*>( driver->GetValue( _id ) ) )
					{
						*o_value = value->GetValue();
						res = true;
					}
					break;
				}
				case ValueID::ValueType_Int:
				{
					if( ValueInt* value = static_cast<ValueInt*>( driver->GetValue( _id ) ) )
					{
						snprintf( str, sizeof(str), "%d", value->GetValue() );
						*o_value = str;
						res = true;
					}
					break;
				}
				case ValueID::ValueType_List:
				{
					if( ValueList* value = static_cast<ValueList*>( driver->GetValue( _id ) ) )
					{
						ValueList::Item const& item = value->GetItem();
						*o_value = item.m_label;
						res = true;
					}
					break;
				}
				case ValueID::ValueType_Short:
				{
					if( ValueShort* value = static_cast<ValueShort*>( driver->GetValue( _id ) ) )
					{
						snprintf( str, sizeof(str), "%d", value->GetValue() );
						*o_value = str;
						res = true;
					}
					break;
				}
				case ValueID::ValueType_String:
				{
					if( ValueString* value = static_cast<ValueString*>( driver->GetValue( _id ) ) )
					{
						*o_value = value->GetValue();
						res = true;
					}
					break;
				}
			}

			driver->ReleaseNodes();
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::GetValueListSelection>
// Gets the selected item from a list value
//-----------------------------------------------------------------------------
bool Manager::GetValueListSelection
(
	ValueID const& _id,
	string* o_value
)
{
	bool res = false;

	if( o_value )
	{
		if( ValueID::ValueType_Int == _id.GetType() )
		{
			if( Driver* driver = GetDriver( _id.GetHomeId() ) )
			{
				driver->LockNodes();
				if( ValueList* value = static_cast<ValueList*>( driver->GetValue( _id ) ) )
				{
					ValueList::Item const& item = value->GetItem();
					*o_value = item.m_label;
					res = true;
				}
				driver->ReleaseNodes();
			}
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::GetValueListItems>
// Gets the list of items from a list value
//-----------------------------------------------------------------------------
bool Manager::GetValueListItems
(
	ValueID const& _id,
	vector<string>* o_value
)
{
	bool res = false;

	if( o_value )
	{
		if( ValueID::ValueType_Int == _id.GetType() )
		{
			if( Driver* driver = GetDriver( _id.GetHomeId() ) )
			{
				driver->LockNodes();
				if( ValueList* value = static_cast<ValueList*>( driver->GetValue( _id ) ) )
				{
					res = value->GetItemLabels( o_value );
				}
				driver->ReleaseNodes();
			}
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::SetValue>
// Sets the value from a bool
//-----------------------------------------------------------------------------
bool Manager::SetValue
( 
	ValueID const& _id, 
	bool const _value
)
{
	bool res = false;

	if( ValueID::ValueType_Bool == _id.GetType() )
	{
		if( Driver* driver = GetDriver( _id.GetHomeId() ) )
		{
			driver->LockNodes();
			if( ValueBool* value = static_cast<ValueBool*>( driver->GetValue( _id ) ) )
			{
				res = value->Set( _value );
			}
			driver->ReleaseNodes();
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::SetValue>
// Sets the value from a byte
//-----------------------------------------------------------------------------
bool Manager::SetValue
( 
	ValueID const& _id, 
	uint8 const _value
)
{
	bool res = false;

	if( ValueID::ValueType_Byte == _id.GetType() )
	{
		if( Driver* driver = GetDriver( _id.GetHomeId() ) )
		{
			driver->LockNodes();
			if( ValueByte* value = static_cast<ValueByte*>( driver->GetValue( _id ) ) )
			{
				res = value->Set( _value );
			}
			driver->ReleaseNodes();
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::SetValue>
// Sets the value from a floating point number
//-----------------------------------------------------------------------------
bool Manager::SetValue
( 
	ValueID const& _id, 
	float const _value
)
{
	bool res = false;

	if( ValueID::ValueType_Decimal == _id.GetType() )
	{
		if( Driver* driver = GetDriver( _id.GetHomeId() ) )
		{
			driver->LockNodes();
			if( ValueDecimal* value = static_cast<ValueDecimal*>( driver->GetValue( _id ) ) )
			{
				char str[256];
				snprintf( str, sizeof(str), "%f", _value );
				res = value->Set( str );
			}
			driver->ReleaseNodes();
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::SetValue>
// Sets the value from a 32-bit signed integer
//-----------------------------------------------------------------------------
bool Manager::SetValue
( 
	ValueID const& _id, 
	int32 const _value
)
{
	bool res = false;

	if( ValueID::ValueType_Int == _id.GetType() )
	{
		if( Driver* driver = GetDriver( _id.GetHomeId() ) )
		{
			driver->LockNodes();
			if( ValueInt* value = static_cast<ValueInt*>( driver->GetValue( _id ) ) )
			{
				res = value->Set( _value );
			}
			driver->ReleaseNodes();
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::SetValue>
// Sets the value from a 16-bit signed integer
//-----------------------------------------------------------------------------
bool Manager::SetValue
( 
	ValueID const& _id, 
	int16 const _value
)
{
	bool res = false;

	if( ValueID::ValueType_Short == _id.GetType() )
	{
		if( Driver* driver = GetDriver( _id.GetHomeId() ) )
		{
			driver->LockNodes();
			if( ValueShort* value = static_cast<ValueShort*>( driver->GetValue( _id ) ) )
			{
				res = value->Set( _value );
			}
			driver->ReleaseNodes();
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::SetValueListSelection>
// Sets the selected item in a list by value
//-----------------------------------------------------------------------------
bool Manager::SetValueListSelection
(
	ValueID const& _id,
	string const& _selectedItem
)
{
	bool res = false;

	if( ValueID::ValueType_Int == _id.GetType() )
	{
		if( Driver* driver = GetDriver( _id.GetHomeId() ) )
		{
			driver->LockNodes();
			if( ValueList* value = static_cast<ValueList*>( driver->GetValue( _id ) ) )
			{
				res = value->SetByLabel( _selectedItem );
			}
			driver->ReleaseNodes();
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::SetValue>
// Sets the value from a string
//-----------------------------------------------------------------------------
bool Manager::SetValue
( 
	ValueID const& _id, 
	string const& _value
)
{
	bool res = false;

	if( Driver* driver = GetDriver( _id.GetHomeId() ) )
	{
		driver->LockNodes();
		
		switch( _id.GetType() )
		{
			case ValueID::ValueType_Bool:
			{
				if( ValueBool* value = static_cast<ValueBool*>( driver->GetValue( _id ) ) )
				{
					if( !strcasecmp( "true", _value.c_str() ) )
					{
						res = value->Set( true );
					}
					else if( !strcasecmp( "false", _value.c_str() ) )
					{
						res = value->Set( false );
					}
				}
				break;
			}
			case ValueID::ValueType_Byte:
			{
				if( ValueByte* value = static_cast<ValueByte*>( driver->GetValue( _id ) ) )
				{
					uint32 val = (uint32)atoi( _value.c_str() );
					if( val < 256 )
					{
						res = value->Set( (uint8)val );
					}
				}
				break;
			}
			case ValueID::ValueType_Decimal:
			{
				if( ValueDecimal* value = static_cast<ValueDecimal*>( driver->GetValue( _id ) ) )
				{
					res = value->Set( _value );
				}
				break;
			}
			case ValueID::ValueType_Int:
			{
				if( ValueInt* value = static_cast<ValueInt*>( driver->GetValue( _id ) ) )
				{
					int32 val = atoi( _value.c_str() );
					res = value->Set( val );
				}
				break;
			}
			case ValueID::ValueType_List:
			{
				if( ValueList* value = static_cast<ValueList*>( driver->GetValue( _id ) ) )
				{
					res = value->SetByLabel( _value );
				}
				break;
			}
			case ValueID::ValueType_Short:
			{
				if( ValueShort* value = static_cast<ValueShort*>( driver->GetValue( _id ) ) )
				{
					int32 val = (uint32)atoi( _value.c_str() );
					if( ( val < 32768 ) && ( val >= -32768 ) )
					{
						res = value->Set( (int16)val );
					}
				}
				break;
			}
			case ValueID::ValueType_String:
			{
				if( ValueString* value = static_cast<ValueString*>( driver->GetValue( _id ) ) )
				{
					res = value->Set( _value );
				}
				break;
			}
		}

		driver->ReleaseNodes();
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::PressButton>
// Starts an activity in a device.
//-----------------------------------------------------------------------------
bool Manager::PressButton
( 
	ValueID const& _id
)
{
	bool res = false;

	if( ValueID::ValueType_Button == _id.GetType() )
	{
		if( Driver* driver = GetDriver( _id.GetHomeId() ) )
		{
			driver->LockNodes();
			if( ValueButton* value = static_cast<ValueButton*>( driver->GetValue( _id ) ) )
			{
				res = value->PressButton();
			}
			driver->ReleaseNodes();
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::ReleaseButton>
// Stops an activity in a device.
//-----------------------------------------------------------------------------
bool Manager::ReleaseButton
( 
	ValueID const& _id
)
{
	bool res = false;

	if( ValueID::ValueType_Button == _id.GetType() )
	{
		if( Driver* driver = GetDriver( _id.GetHomeId() ) )
		{
			driver->LockNodes();
			if( ValueButton* value = static_cast<ValueButton*>( driver->GetValue( _id ) ) )
			{
				res = value->ReleaseButton();
			}
			driver->ReleaseNodes();
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// Climate Control Schedules
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// <Manager::GetNumSwitchPoints>
// Get the number of switch points defined in a schedule
//-----------------------------------------------------------------------------
uint8 Manager::GetNumSwitchPoints
(
	ValueID const& _id
)
{
	bool res = false;

	uint8 numSwitchPoints = 0;
	if( ValueID::ValueType_Schedule == _id.GetType() )
	{
		if( Driver* driver = GetDriver( _id.GetHomeId() ) )
		{
			driver->LockNodes();
			if( ValueSchedule* value = static_cast<ValueSchedule*>( driver->GetValue( _id ) ) )
			{
				numSwitchPoints = value->GetNumSwitchPoints();
			}
			driver->ReleaseNodes();
		}
	}

	return numSwitchPoints;
}

//-----------------------------------------------------------------------------
// <Manager::SetSwitchPoint>
// Set a switch point in the schedule
//-----------------------------------------------------------------------------
bool Manager::SetSwitchPoint
(
	ValueID const& _id,
	uint8 const _hours,
	uint8 const _minutes,
	int8 const _setback
)
{
	bool res = false;

	if( ValueID::ValueType_Schedule == _id.GetType() )
	{
		if( Driver* driver = GetDriver( _id.GetHomeId() ) )
		{
			driver->LockNodes();
			if( ValueSchedule* value = static_cast<ValueSchedule*>( driver->GetValue( _id ) ) )
			{
				res = value->SetSwitchPoint( _hours, _minutes, _setback );
			}
			driver->ReleaseNodes();
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::RemoveSwitchPoint>
// Remove a switch point from the schedule
//-----------------------------------------------------------------------------
bool Manager::RemoveSwitchPoint
(
	ValueID const& _id,
	uint8 const _hours,
	uint8 const _minutes
)
{
	bool res = false;

	if( ValueID::ValueType_Schedule == _id.GetType() )
	{
		if( Driver* driver = GetDriver( _id.GetHomeId() ) )
		{
			driver->LockNodes();
			if( ValueSchedule* value = static_cast<ValueSchedule*>( driver->GetValue( _id ) ) )
			{
				uint8 idx;
				res = value->FindSwitchPoint( _hours, _minutes, &idx );

				if( res )
				{
					res = value->RemoveSwitchPoint( idx );
				}
			}
			driver->ReleaseNodes();
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
// <Manager::ClearSwitchPoints>
// Clears all switch points from the schedule
//-----------------------------------------------------------------------------
void Manager::ClearSwitchPoints
(
	ValueID const& _id
)
{
	if( ValueID::ValueType_Schedule == _id.GetType() )
	{
		if( Driver* driver = GetDriver( _id.GetHomeId() ) )
		{
			driver->LockNodes();
			if( ValueSchedule* value = static_cast<ValueSchedule*>( driver->GetValue( _id ) ) )
			{
				value->ClearSwitchPoints();
			}
			driver->ReleaseNodes();
		}
	}
}
		
//-----------------------------------------------------------------------------
// <Manager::GetSwitchPoint>
// Gets switch point data from the schedule
//-----------------------------------------------------------------------------
bool Manager::GetSwitchPoint
( 
	ValueID const& _id,
	uint8 const _idx,
	uint8* o_hours,
	uint8* o_minutes,
	int8* o_setback
)
{
	bool res = false;

	if( ValueID::ValueType_Schedule == _id.GetType() )
	{
		if( Driver* driver = GetDriver( _id.GetHomeId() ) )
		{
			driver->LockNodes();
			if( ValueSchedule* value = static_cast<ValueSchedule*>( driver->GetValue( _id ) ) )
			{
				res = value->GetSwitchPoint( _idx, o_hours, o_minutes, o_setback );
			}
			driver->ReleaseNodes();
		}
	}

	return res;
}

//-----------------------------------------------------------------------------
//	Configuration Parameters
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// <Manager::SetConfigParam>
// Set the value of one of the configuration parameters of a device
//-----------------------------------------------------------------------------
bool Manager::SetConfigParam
(
	uint32 const _homeId, 
	uint8 const _nodeId,
	uint8 const _param,
	int32 _value
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->SetConfigParam( _nodeId, _param, _value );
	}

	return false;
}

//-----------------------------------------------------------------------------
// <Manager::RequestConfigParam>
// Request the value of one of the configuration parameters of a device
//-----------------------------------------------------------------------------
void Manager::RequestConfigParam
(
	uint32 const _homeId, 
	uint8 const _nodeId,
	uint8 const _param
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		driver->RequestConfigParam( _nodeId, _param );
	}
}


//-----------------------------------------------------------------------------
//	Groups
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// <Manager::GetNumGroups>
// Gets the number of association groups reported by this node
//-----------------------------------------------------------------------------
uint8 Manager::GetNumGroups
(
	uint32 const _homeId, 
	uint8 const _nodeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->GetNumGroups( _nodeId );
	}

	return 0;
}

//-----------------------------------------------------------------------------
// <Manager::GetAssociations>
// Gets the associations for a group
//-----------------------------------------------------------------------------
uint32 Manager::GetAssociations
( 
	uint32 const _homeId,
	uint8 const _nodeId,
	uint8 const _groupIdx,
	uint8** o_associations
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return driver->GetAssociations( _nodeId, _groupIdx, o_associations );
	}

	return 0;
}

//-----------------------------------------------------------------------------
// <Manager::AddAssociation>
// Adds a node to an association group
//-----------------------------------------------------------------------------
void Manager::AddAssociation
(
	uint32 const _homeId,
	uint8 const _nodeId,
	uint8 const _groupIdx,
	uint8 const _targetNodeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		driver->AddAssociation( _nodeId, _groupIdx, _targetNodeId );
	}
}

//-----------------------------------------------------------------------------
// <Manager::RemoveAssociation>
// Removes a node from an association group
//-----------------------------------------------------------------------------
void Manager::RemoveAssociation
(
	uint32 const _homeId,
	uint8 const _nodeId,
	uint8 const _groupIdx,
	uint8 const _targetNodeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		driver->RemoveAssociation( _nodeId, _groupIdx, _targetNodeId );
	}
}


//-----------------------------------------------------------------------------
//	Notifications
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// <Manager::AddWatcher>
// Add a watcher to the list
//-----------------------------------------------------------------------------
bool Manager::AddWatcher
(
	pfnOnNotification_t _watcher,
	void* _context
)
{
	// Ensure this watcher is not already on the list
	m_notificationMutex->Lock();
	for( list<Watcher*>::iterator it = m_watchers.begin(); it != m_watchers.end(); ++it )
	{
		if( ((*it)->m_callback == _watcher ) && ( (*it)->m_context == _context ) )
		{
			// Already in the list
			m_notificationMutex->Release();
			return false;
		}
	}

	m_watchers.push_back( new Watcher( _watcher, _context ) );
	m_notificationMutex->Release();
	return true;
}

//-----------------------------------------------------------------------------
// <Manager::RemoveWatcher>
// Remove a watcher from the list
//-----------------------------------------------------------------------------
bool Manager::RemoveWatcher
(
	pfnOnNotification_t _watcher,
	void* _context
)
{
	m_notificationMutex->Lock();
	list<Watcher*>::iterator it = m_watchers.begin();
	while( it != m_watchers.end() )
	{
		if( ((*it)->m_callback == _watcher ) && ( (*it)->m_context == _context ) )
		{
			delete (*it);
			m_watchers.erase( it );
			m_notificationMutex->Release();
			return true;
		}
	}

	m_notificationMutex->Release();
	return false;
}

//-----------------------------------------------------------------------------
// <Manager::NotifyWatchers>
// Notify any watching objects of a value change
//-----------------------------------------------------------------------------
void Manager::NotifyWatchers
(
	Notification* _notification
)
{
	m_notificationMutex->Lock();
	for( list<Watcher*>::iterator it = m_watchers.begin(); it != m_watchers.end(); ++it )
	{
		Watcher* pWatcher = *it;
		pWatcher->m_callback( _notification, pWatcher->m_context );
	}
	m_notificationMutex->Release();
}

//-----------------------------------------------------------------------------
//	Controller commands
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// <Manager::ResetController>
// Reset controller and erase all node information
//-----------------------------------------------------------------------------
void Manager::ResetController
(
	uint32 const _homeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		driver->ResetController();
	}
}

//-----------------------------------------------------------------------------
// <Manager::SoftReset>
// Soft-reset the Z-Wave controller chip
//-----------------------------------------------------------------------------
void Manager::SoftReset
(
	uint32 const _homeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		driver->SoftReset();
	}
}

//-----------------------------------------------------------------------------
// <Manager::RequestNodeNeighborUpdate>
// 
//-----------------------------------------------------------------------------
void Manager::RequestNodeNeighborUpdate
(
	uint32 const _homeId,
	uint8 const _nodeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		driver->RequestNodeNeighborUpdate( _nodeId );
	}
}

//-----------------------------------------------------------------------------
// <Manager::RequestNetworkUpdate>
// Request a network update
//-----------------------------------------------------------------------------
void Manager::RequestNetworkUpdate
(
	uint32 const _homeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		driver->RequestNetworkUpdate();
	}
}


//-----------------------------------------------------------------------------
// <Manager::AssignReturnRoute>
// 
//-----------------------------------------------------------------------------
//void Manager::AssignReturnRoute
//(
//	uint32 const _homeId,
//	uint8 const _nodeId,
//	uint8 const _targetNodeId
//)
//{
//	if( Driver* driver = GetDriver( _homeId ) )
//	{
//		driver->AssignReturnRoute( _nodeId, _targetNodeId );
//	}
//} 

//-----------------------------------------------------------------------------
// <Manager::BeginControllerCommand>
// Start the controller performing one of its network management functions
//-----------------------------------------------------------------------------
bool Manager::BeginControllerCommand
(
	uint32 const _homeId, 
	Driver::ControllerCommand _command,
	Driver::pfnControllerCallback_t _callback,	// = NULL
	void* _context,								// = NULL
	bool _highPower,							// = false
	uint8 _nodeId								// = 0xff
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return( driver->BeginControllerCommand( _command, _callback, _context, _highPower, _nodeId ) );
	}

	return false;
}

//-----------------------------------------------------------------------------
// <Manager::CancelControllerCommand>
// Stop the current controller function
//-----------------------------------------------------------------------------
bool Manager::CancelControllerCommand
(
	uint32 const _homeId
)
{
	if( Driver* driver = GetDriver( _homeId ) )
	{
		return( driver->CancelControllerCommand() );
	}

	return false;
}

