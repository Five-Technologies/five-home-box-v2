// //-----------------------------------------------------------------------------
// //
// //	Main.cpp
// //
// //	Minimal application to test OpenZWave.
// //
// //	Creates an OpenZWave::Driver and the waits.  In Debug builds
// //	you should see verbose logging to the console, which will
// //	indicate that communications with the Z-Wave network are working.
// //
// //	Copyright (c) 2010 Mal Lansell <mal@openzwave.com>
// //
// //
// //	SOFTWARE NOTICE AND LICENSE
// //
// //	This file is part of OpenZWave.
// //
// //	OpenZWave is free software: you can redistribute it and/or modify
// //	it under the terms of the GNU Lesser General Public License as published
// //	by the Free Software Foundation, either version 3 of the License,
// //	or (at your option) any later version.
// //
// //	OpenZWave is distributed in the hope that it will be useful,
// //	but WITHOUT ANY WARRANTY; without even the implied warranty of
// //	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// //	GNU Lesser General Public License for more details.
// //
// //	You should have received a copy of the GNU Lesser General Public License
// //	along with OpenZWave.  If not, see <http://www.gnu.org/licenses/>.
// //
// //-----------------------------------------------------------------------------

// #include <unistd.h>
// #include <stdlib.h>
// #include <pthread.h>
// #include "Options.h"
// #include "Manager.h"
// #include "Driver.h"
// #include "Node.h"
// #include "Group.h"
// #include "Notification.h"
// #include "value_classes/ValueStore.h"
// #include "value_classes/Value.h"
// #include "value_classes/ValueBool.h"
// #include "platform/Log.h"
// #include "Defs.h"

// using namespace OpenZWave;


// static uint32 g_homeId = 0;
// static bool   g_initFailed = false;

// typedef struct
// {
// 	uint32			m_homeId;
// 	uint8			m_nodeId;
// 	bool			m_polled;
// 	list<ValueID>	m_values;
// }NodeInfo;

// static list<NodeInfo*> g_nodes;
// static pthread_mutex_t g_criticalSection;
// static pthread_cond_t  initCond  = PTHREAD_COND_INITIALIZER;
// static pthread_mutex_t initMutex = PTHREAD_MUTEX_INITIALIZER;

// //-----------------------------------------------------------------------------
// // <GetNodeInfo>
// // Return the NodeInfo object associated with this notification
// //-----------------------------------------------------------------------------
// NodeInfo* GetNodeInfo
// (
// 	Notification const* notification
// )
// {
// 	uint32 const homeId = notification->GetHomeId();
// 	uint8 const nodeId = notification->GetNodeId();
// 	uint8 x{ 3 };
// 	cout << "------------ NODE IDDDDDD: " << unsigned(x) << endl;
// 	for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
// 	{
// 		NodeInfo* nodeInfo = *it;
// 		if( ( nodeInfo->m_homeId == homeId ) && ( nodeInfo->m_nodeId == nodeId ) )
// 		{
// 			return nodeInfo;
// 		}
// 	}

// 	return NULL;
// }

// //-----------------------------------------------------------------------------
// // <OnNotification>
// // Callback that is triggered when a value, group or node changes
// //-----------------------------------------------------------------------------
// void OnNotification
// (
// 	Notification const* notification,
// 	void* _context
// )
// {
// 	// Must do this inside a critical section to avoid conflicts with the main thread
// 	pthread_mutex_lock( &g_criticalSection );
// 	NodeInfo* nodeInfo = GetNodeInfo(notification);
// 	// cout << notification->GetType() << "was sent by NodeId:" << *nodeInfo << endl;

// //	std::cout << "Notification: " << notification << std::endl;

// 	switch( notification->GetType() )
// 	{
// 		case Notification::Type_ValueAdded:
// 		{
// 			if( NodeInfo* nodeInfo = GetNodeInfo( notification ) )
// 			{
// 				// Add the new value to our list
// 				nodeInfo->m_values.push_back( notification->GetValueID() );
// 			}
// 			break;
// 		}

// 		case Notification::Type_ValueRemoved:
// 		{
// 			if( NodeInfo* nodeInfo = GetNodeInfo( notification ) )
// 			{
// 				// Remove the value from out list
// 				for( list<ValueID>::iterator it = nodeInfo->m_values.begin(); it != nodeInfo->m_values.end(); ++it )
// 				{
// 					if( (*it) == notification->GetValueID() )
// 					{
// 						nodeInfo->m_values.erase( it );
// 						break;
// 					}
// 				}
// 			}
// 			break;
// 		}

// 		case Notification::Type_ValueChanged:
// 		{
// 			// One of the node values has changed
// 			if( NodeInfo* nodeInfo = GetNodeInfo( notification ) )
// 			{
// 				(void)nodeInfo;		// placeholder for real action
// 			}
// 			break;
// 		}

// 		case Notification::Type_Group:
// 		{
// 			// One of the node's association groups has changed
// 			if( NodeInfo* nodeInfo = GetNodeInfo( notification ) )
// 			{
// 				(void)nodeInfo;		// placeholder for real action
// 			}
// 			break;
// 		}

// 		case Notification::Type_NodeAdded:
// 		{
// 			// Add the new node to our list
// 			NodeInfo* nodeInfo = new NodeInfo();
// 			nodeInfo->m_homeId = notification->GetHomeId();
// 			nodeInfo->m_nodeId = notification->GetNodeId();
// 			nodeInfo->m_polled = false;
// 			g_nodes.push_back( nodeInfo );
// 			break;
// 		}

// 		case Notification::Type_NodeRemoved:
// 		{
// 			// Remove the node from our list
// 			uint32 const homeId = notification->GetHomeId();
// 			uint8 const nodeId = notification->GetNodeId();
// 			for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
// 			{
// 				NodeInfo* nodeInfo = *it;
// 				if( ( nodeInfo->m_homeId == homeId ) && ( nodeInfo->m_nodeId == nodeId ) )
// 				{
// 					g_nodes.erase( it );
// 					delete nodeInfo;
// 					break;
// 				}
// 			}
// 			break;
// 		}

// 		case Notification::Type_NodeEvent:
// 		{
// 			// We have received an event from the node, caused by a
// 			// basic_set or hail message.
// 			if( NodeInfo* nodeInfo = GetNodeInfo( notification ) )
// 			{
// 				(void)nodeInfo;		// placeholder for real action
// 			}
// 			break;
// 		}

// 		case Notification::Type_PollingDisabled:
// 		{
// 			if( NodeInfo* nodeInfo = GetNodeInfo( notification ) )
// 			{
// 				nodeInfo->m_polled = false;
// 			}
// 			break;
// 		}

// 		case Notification::Type_PollingEnabled:
// 		{
// 			if( NodeInfo* nodeInfo = GetNodeInfo( notification ) )
// 			{
// 				nodeInfo->m_polled = true;
// 			}
// 			break;
// 		}

// 		case Notification::Type_DriverReady:
// 		{
// 			g_homeId = notification->GetHomeId();
// 			printf("Driver ready with HomeID: 0x%.8x\n", g_homeId);
// 			bool ext_tx = Manager::Get()->HasExtendedTxStatus(g_homeId);

// 			if (ext_tx)
// 			{
// 				printf("Controller has extended TxStatus.\n");
// 			}
// 			else
// 			{
// 				printf("Controller does not have extended TxStatus.\n");
// 			}

// 			break;
// 		}

// 		case Notification::Type_DriverFailed:
// 		{
// 			g_initFailed = true;
// 			pthread_cond_broadcast(&initCond);
// 			break;
// 		}

// 		case Notification::Type_AwakeNodesQueried:
// 			break;
// 		case Notification::Type_AllNodesQueried:
// 		case Notification::Type_AllNodesQueriedSomeDead:
// 		{
// 			pthread_cond_broadcast(&initCond);
// 			break;
// 		}

// 		case Notification::Type_DriverReset:
// 		case Notification::Typenotification:
// 		case Notification::Type_NodeNaming:
// 		case Notification::Type_NodeProtocolInfo:
// 		case Notification::Type_NodeQueriesComplete:
// 		case Notification::Type_NodeNew:
// 		case Notification::Type_SceneEvent:
// 		case Notification::Type_CreateButton:
// 		case Notification::Type_DeleteButton:
// 		case Notification::Type_ButtonOn:
// 		case Notification::Type_ButtonOff:
// 		case Notification::Type_EssentialNodeQueriesComplete:
// 		case Notification::Type_DriverRemoved:
// 		case Notification::Type_ControllerCommand:
// 		case Notification::Type_NodeReset:
// 		case Notification::Type_UserAlerts:
// 		case Notification::Type_ManufacturerSpecificDBReady:
// 		case Notification::Type_ValueRefreshed:
// 		{
// 		}
// 	}

// 	pthread_mutex_unlock( &g_criticalSection );
// }

// //-----------------------------------------------------------------------------
// // <main>
// // Create the driver and then wait
// //-----------------------------------------------------------------------------
// int main( int argc, char* argv[] )
// {
// 	pthread_mutexattr_t mutexattr;

// 	pthread_mutexattr_init ( &mutexattr );
// 	pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE );
// 	pthread_mutex_init( &g_criticalSection, &mutexattr );
// 	pthread_mutexattr_destroy( &mutexattr );

// 	pthread_mutex_lock( &initMutex );

// 	// petergebruers replace getVersionAsString() with getVersionLongAsString() because
// 	// the latter prints more information, based on the status of the repository
// 	// when "make" was run. A Makefile gets this info from git describe --long --tags --dirty

// 	printf("Starting MinOZW with OpenZWave Version %s\n", Manager::getVersionLongAsString().c_str());

// 	// Create the OpenZWave Manager.
// 	// The first argument is the path to the config files (where the manufacturer_specific.xml file is located
// 	// The second argument is the path for saved Z-Wave network state and the log file.  If you leave it NULL
// 	// the log file will appear in the program's working directory.
// 	Options::Create( "../../../config/", "", "" );
// 	Options::Get()->AddOptionInt( "SaveLogLevel", LogLevel_Detail );
// 	Options::Get()->AddOptionInt( "QueueLogLevel", LogLevel_Debug );
// 	Options::Get()->AddOptionInt( "DumpTrigger", LogLevel_Error );
// 	Options::Get()->AddOptionInt( "PollInterval", 500 );
// 	Options::Get()->AddOptionBool( "IntervalBetweenPolls", true );
// 	Options::Get()->AddOptionBool("ValidateValueChanges", true);
// 	Options::Get()->Lock();

// 	Manager::Create();

// 	// Add a callback handler to the manager.  The second argument is a context that
// 	// is passed to the OnNotification method.  If the OnNotification is a method of
// 	// a class, the context would usually be a pointer to that class object, to
// 	// avoid the need for the notification handler to be a static.
// 	Manager::Get()->AddWatcher( OnNotification, NULL );

// 	// Add a Z-Wave Driver
// 	// Modify this line to set the correct serial port for your PC interface.

// #ifdef DARWIN
// 	string port = "/dev/cu.usbserial";
// #elif WIN32
//         string port = "\\\\.\\COM6";
// #else
// 	string port = "/dev/ttyACM0";
// #endif
// 	if ( argc > 1 )
// 	{
// 		port = argv[1];
// 	}
// 	if( strcasecmp( port.c_str(), "usb" ) == 0 )
// 	{
// 		Manager::Get()->AddDriver( "HID Controller", Driver::ControllerInterface_Hid );
// 	}
// 	else
// 	{
// 		Manager::Get()->AddDriver( port );
// 	}

// 	// Now we just wait for either the AwakeNodesQueried or AllNodesQueried notification,
// 	// then write out the config file.
// 	// In a normal app, we would be handling notifications and building a UI for the user.
// 	pthread_cond_wait( &initCond, &initMutex );

// 	// Since the configuration file contains command class information that is only
// 	// known after the nodes on the network are queried, wait until all of the nodes
// 	// on the network have been queried (at least the "listening" ones) before
// 	// writing the configuration file.  (Maybe write again after sleeping nodes have
// 	// been queried as well.)
// 	if( !g_initFailed )
// 	{

// 		// The section below demonstrates setting up polling for a variable.  In this simple
// 		// example, it has been hardwired to poll COMMAND_CLASS_BASIC on the each node that
// 		// supports this setting.
// 		pthread_mutex_lock( &g_criticalSection );
// 		for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
// 		{
// 			NodeInfo* nodeInfo = *it;

// 			// skip the controller (most likely node 1)
// 			//if( nodeInfo->m_nodeId == 1) continue;

// 			printf("NodeID: %d \n ", nodeInfo->m_nodeId);
// 			printf("\t NodeName: %s \n ", Manager::Get()->GetNodeName(nodeInfo->m_homeId,nodeInfo->m_nodeId).c_str());
// 			printf("\t ManufacturerName: %s \n ", Manager::Get()->GetNodeManufacturerName(nodeInfo->m_homeId,nodeInfo->m_nodeId).c_str());
// 			printf("\t NodeProductName: %s \n ", Manager::Get()->GetNodeProductName(nodeInfo->m_homeId,nodeInfo->m_nodeId).c_str());

// 			printf("Values announced by the nodes without polling: \n");
// 			for( list<ValueID>::iterator it2 = nodeInfo->m_values.begin(); it2 != nodeInfo->m_values.end(); ++it2 )
// 			{
// 				ValueID v = *it2;
// 				printf("\t ValueLabel: %s \n", Manager::Get()->GetValueLabel(v).c_str());
// 				printf("\t\t ValueType: %s (%d) \n", v.GetTypeAsString().c_str(), v.GetType());
// 				printf("\t\t ValueHelp: %s \n", Manager::Get()->GetValueHelp(v).c_str());
// 				printf("\t\t ValueUnits: %s \n", Manager::Get()->GetValueUnits(v).c_str());
// 				printf("\t\t ValueMin: %d \n", Manager::Get()->GetValueMin(v));
// 				printf("\t\t ValueMax: %d \n", Manager::Get()->GetValueMax(v));
// 				printf("\t\t ValueGenre: %s (%d)\n", v.GetGenreAsString().c_str(), v.GetGenre());

// 				if( v.GetCommandClassId() == COMMAND_CLASS_BASIC )
// 				{
// //					Manager::Get()->EnablePoll( v, 2 );		// enables polling with "intensity" of 2, though this is irrelevant with only one value polled
// 					break;
// 				}
// 			}
// 		}
// 		pthread_mutex_unlock( &g_criticalSection );

// 		// If we want to access our NodeInfo list, that has been built from all the
// 		// notification callbacks we received from the library, we have to do so
// 		// from inside a Critical Section.  This is because the callbacks occur on other
// 		// threads, and we cannot risk the list being changed while we are using it.
// 		// We must hold the critical section for as short a time as possible, to avoid
// 		// stalling the OpenZWave drivers.
// 		// At this point, the program just waits for 3 minutes (to demonstrate polling),
// 		// then exits
// 		//for( int i = 0; i < 60*3; i++ )
// 		//{
// 		//	pthread_mutex_lock( &g_criticalSection );
// 			// but NodeInfo list and similar data should be inside critical section
// 		//	pthread_mutex_unlock( &g_criticalSection );
// 		//	sleep(1);
// 		//}

// 		Driver::DriverData data;
// 		Manager::Get()->GetDriverStatistics( g_homeId, &data );
// 		printf("SOF: %d ACK Waiting: %d Read Aborts: %d Bad Checksums: %d\n", data.m_SOFCnt, data.m_ACKWaiting, data.m_readAborts, data.m_badChecksum);
// 		printf("Reads: %d Writes: %d CAN: %d NAK: %d ACK: %d Out of Frame: %d\n", data.m_readCnt, data.m_writeCnt, data.m_CANCnt, data.m_NAKCnt, data.m_ACKCnt, data.m_OOFCnt);
// 		printf("Dropped: %d Retries: %d\n", data.m_dropped, data.m_retries);
// 	}

// 	// program exit (clean up)
// 	if( strcasecmp( port.c_str(), "usb" ) == 0 )
// 	{
// 		Manager::Get()->RemoveDriver( "HID Controller" );
// 	}
// 	else
// 	{
// 		Manager::Get()->RemoveDriver( port );
// 	}
// 	Manager::Get()->RemoveWatcher( OnNotification, NULL );
// 	Manager::Destroy();
// 	Options::Destroy();
// 	pthread_mutex_destroy( &g_criticalSection );
// 	for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
// 	{
// 		NodeInfo* nodeInfo = *it;
// 		nodeInfo->m_values.clear();
// 		delete nodeInfo;
// 	}
// 	g_nodes.clear();
// 	return 0;
// }















#include <iostream>
#include "Manager.h"
#include "Options.h"
#include "Notification.h"
#include "platform/Log.h"
#include "Node.h"
#include <thread>
#include "Five.h"

using namespace OpenZWave;
using namespace Five;
using namespace std;

static uint32 g_homeId = 0;
static bool   g_initFailed = false;

static list<NodeInfo*> g_nodes;
static pthread_mutex_t g_criticalSection;
static pthread_cond_t  initCond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t initMutex;
static bool g_menuLocked{ true };

NodeInfo* GetNodeInfo
(
	Notification const* notification
)
{
	uint32 const homeId = notification->GetHomeId();
	uint8 const nodeId = notification->GetNodeId();
	for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it )
	{
		NodeInfo* nodeInfo = *it;
		if( ( nodeInfo->m_homeId == homeId ) && ( nodeInfo->m_nodeId == nodeId ) )
		{
			return nodeInfo;
		}
	}

	return NULL;
}

void OnNotification(Notification const* notification, void* context)
{
	pthread_mutex_lock(&g_criticalSection); // lock critical section

	if (g_homeId == 0)
		g_homeId = notification->GetHomeId();

	cout << ">>> NOTIFICATION: " << notification << endl;
	cout << ">>> CONTEXT: " << context << endl;

	// cout << "Before AddNode()\n";
	// if (g_homeId != 0)
	// {
	// 	// uint32 const homeId = 3700699877;
	// 	cout << "In condition\n";
	// 	Manager::Get()->AddNode(g_homeId);
	// 	cout << "After condition\n";
	// }
	// cout << "After AddNode()\n";

	// cout << "---- NOTIFICATION: ----\n";
	// cout << "Type: " << notification->GetType() << '\n';
	// cout << "HomeId: " << notification->GetHomeId() << '\n';
	// cout << "NodeId: " << notification->GetNodeId() << '\n';
	// cout << "Number of nodes: " << g_nodes.size() << '\n';

	NodeInfo* nodeInfo = new NodeInfo();
    ValueID v;
	list<NodeInfo*>::iterator itNode;
	list<ValueID>::iterator itValueID;
	Node::NodeData* nodeData = new Node::NodeData;
	bool myBool;
	bool* myBoolPtr = &myBool;	
	bool* myValue;
	string valueLabel;
	
	cout << notification << endl;	

	switch (notification->GetType())
	{
		case Notification::Type_ValueAdded:
			//We get from the notification the values's ID and name.
			v = notification->GetValueID();
			valueLabel = Manager::Get()->GetValueLabel(v);

			

			//We add the value to the value list of the node
			for(itNode = g_nodes.begin(); itNode != g_nodes.end(); ++itNode )
			{
				uint8 nodeId = (*itNode) -> m_nodeId;
				if ( nodeId == v.GetNodeId() )
					((*itNode) -> m_values).push_back(v);
					
			}

			cout << "[" << time(0) << " : VALUE_ADDED] label: " << valueLabel << ", id: " << v.GetId() << "nodeId: " << v.GetNodeId() << endl;
			break;
		case Notification::Type_ValueRemoved:
			//We get from the notification the values's ID and name.
			//v = notification->GetValueID();
			//valueLabel = Manager::Get()->GetValueLabel(v);


			//We delete the value from the value list of the node

			// for(itNode = g_nodes.begin(); itNode != g_nodes.end(); ++itNode )
			// {
			// 	uint8 nodeId = (*itNode) -> m_nodeId;
			// 	if ( nodeId == v.GetNodeId() )
			// 		((*itNode) -> m_values).remove(v);
			// }

			cout << "[" << time(0) << " : VALUE_REMOVED]" /*label: " << valueLabel << ", id: " << v.GetId() << "nodeId: " << v.GetNodeId()*/ << endl;
			break;
		case Notification::Type_ValueChanged:
			v = notification->GetValueID();
			valueLabel = Manager::Get()->GetValueLabel(v);

			cout << "[" << time(0) << " : VALUE_CHANGED]" << "label: " << valueLabel << ", id: " << v.GetId() << "nodeId: " << v.GetNodeId() << endl;
			break;
		case Notification::Type_ValueRefreshed:
			cout << NotificationService::valueRefreshed(notification, g_nodes) << endl;
			break;
		case Notification::Type_Group:
			break;
		case Notification::Type_NodeNew:
			break;
		case Notification::Type_NodeAdded:
			nodeInfo->m_homeId = notification->GetHomeId();
			nodeInfo->m_nodeId = notification->GetNodeId();
			nodeInfo->m_name = Manager::Get()->GetNodeProductName(nodeInfo->m_homeId, nodeInfo->m_nodeId);
			nodeInfo->m_nodeType = notification->GetType();
			g_nodes.push_back( nodeInfo );
			break;
		case Notification::Type_NodeRemoved:
			for(itNode = g_nodes.begin(); itNode != g_nodes.end(); ++itNode)
			{
				if((*itNode)->m_nodeId == notification->GetNodeId())
				{
					g_nodes.remove((*itNode));
				}
			}
			cout << "[" << time(0) << " : NODE_REMOVED]" << "id: " << notification->GetNodeId() << endl;
			break;
		case Notification::Type_NodeProtocolInfo:
			break;
		case Notification::Type_NodeNaming:
			break;
		case Notification::Type_NodeEvent:
			break;
		case Notification::Type_PollingDisabled:
			break;
		case Notification::Type_PollingEnabled:
			break;
		case Notification::Type_SceneEvent:
			break;
		case Notification::Type_CreateButton:
			break;
		case Notification::Type_DeleteButton:
			break;
		case Notification::Type_ButtonOn:
			break;
		case Notification::Type_ButtonOff:
			break;
		case Notification::Type_DriverReady:
			break;
		case Notification::Type_DriverFailed:
			break;
		case Notification::Type_DriverReset:
			break;
		case Notification::Type_EssentialNodeQueriesComplete:
			break;
		case Notification::Type_AllNodesQueriedSomeDead:
			break;
		case Notification::Type_AllNodesQueried:
			if (!g_menuLocked)
				g_menuLocked = false;
			break;
		case Notification::Type_Notification:
			break;
		case Notification::Type_DriverRemoved:
			break;
		case Notification::Type_ControllerCommand:
			cout << "Create command class..." << endl;
			cout << notification->Type_ControllerCommand << notification->GetCommand();
			break;
		case Notification::Type_NodeReset:
			break;
		case Notification::Type_UserAlerts:
			break;
		case Notification::Type_ManufacturerSpecificDBReady:
			break;
		default:
			break;
	}
	
    

	pthread_mutex_unlock(&g_criticalSection); // unlock critical section
	return;
}



void menu()
{
	string response;
	int choice{ 0 };
	int choiceNode{ 0 };
	string currentV;
	string* currentVPtr = &currentV;
	int choiceValue{0};
	int x{ 5 };
	int counterNode{0};
	int counterValue{0};
	list<NodeInfo*>::iterator nodeIt;
	list<ValueID>::iterator valueIt;
  	while (x --> 0) {
    	std::cout << x << endl;
    	this_thread::sleep_for(chrono::seconds(1));
  	}

  	cout << "----- MENU -----" << endl << endl;
  	cout << "1. Add node" << endl;
  	cout << "2. Remove node" << endl;
  	cout << "3. Get value" << endl;
  	cout << "4. Set value" << endl;
	cout << "5. Reset Key" << endl;
	cout << "6. Wake Up" << endl;
	cout << "7. Heal" << endl;

	cout << "Please choose: ";
    cin >> response;

    try {
        choice = stoi(response);
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }


    switch (choice) {
    case 1:
        Manager::Get()->AddNode(g_homeId, false);
        break;
    case 2:
        Manager::Get()->RemoveNode(g_homeId);
        break;
    case 3:
		cout << "Choose what node you want a value from: " << endl;
        for(nodeIt = g_nodes.begin(); nodeIt != g_nodes.end(); nodeIt++)
		{
			counterNode++;
			cout << counterNode << ". " << (*nodeIt)->m_name << endl;
			
		}
		cin >> response;
		choiceNode = stoi(response);
		counterNode = 0;
		for(nodeIt = g_nodes.begin(); nodeIt != g_nodes.end(); nodeIt++)
		{
			counterNode++;
			if (counterNode == choiceNode)
			{
				for(valueIt = (*nodeIt) -> m_values.begin(); valueIt != (*nodeIt) -> m_values.end(); valueIt++)
				{
					counterValue++;
					cout << counterValue << ". " << Manager::Get()->GetValueLabel(*valueIt) << endl;
				}
			}
			
			
		}
			
				
		// Manager::Get()->;
        break;
    case 4:
		cout << "Choose what node you want to set a value from: " << endl;
        for(nodeIt = g_nodes.begin(); nodeIt != g_nodes.end(); nodeIt++)
		{
			counterNode++;
			cout << counterNode << ". " << (*nodeIt)->m_name << endl;
			
		}
		cin >> response;
		choiceNode = stoi(response);
		counterNode = 0;
		cout << "Choose the value to set: " << endl;
		for(nodeIt = g_nodes.begin(); nodeIt != g_nodes.end(); nodeIt++)
		{
			counterNode++;
			if (counterNode == choiceNode)
			{
				for(valueIt = (*nodeIt) -> m_values.begin(); valueIt != (*nodeIt) -> m_values.end(); valueIt++)
				{
					counterValue++;
					cout << counterValue << ". " << Manager::Get()->GetValueLabel(*valueIt) << endl;
				}
				break;
			}
		}
		cin >> response;
		choiceValue = stoi(response);
		counterNode = 0;
		counterValue = 0;
		
		for(valueIt = (*nodeIt) -> m_values.begin(); valueIt != (*nodeIt) -> m_values.end(); valueIt++)
		{
			counterValue++;
			if (counterValue == choiceValue)
				{
					Manager::Get()->GetValueAsString(*valueIt, currentVPtr);
					cout << "The current value is: " << currentV << endl;
					cout << "Enter the new value: " << endl;
					cin >> response;
					Manager::Get()->SetValue(*valueIt, response);
				}
		}
			

        break;
	case 5:
		break;
	case 6:
		break;
	case 7:
		break;
    default:
        cout << "You must enter 1, 2, 3 or 4." << endl;
        break;
    }

	// Manager::Get()->AddNode(g_homeId, false);
	// Manager::Get()->RemoveNode(g_homeId);
	// cout << "Node removed" << endl;
	// Manager::Get()->TestNetwork(g_homeId, 5);
	// cout << "Name: " << Manager::Get()->GetNodeProductName(g_homeId, 2).c_str() << endl;
}

int main(int argc, char const *argv[])
{
	pthread_mutexattr_t mutexattr;
	string response;

	pthread_mutexattr_init ( &mutexattr );
	pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &g_criticalSection, &mutexattr );
	pthread_mutexattr_destroy( &mutexattr );
	pthread_mutex_lock( &initMutex );

	printf("Starting MinOZW with OpenZWave Version %s\n", Manager::getVersionLongAsString().c_str());

	Options::Create("config/", "log", "");
	// Options::Get()->AddOptionInt( "SaveLogLevel", LogLevel_Debug );
	// Options::Get()->AddOptionInt( "QueueLogLevel", LogLevel_Debug );
	// Options::Get()->AddOptionInt( "DumpTrigger", LogLevel_None );
	// Options::Get()->AddOptionInt( "PollInterval", 500 );
	// Options::Get()->AddOptionBool( "IntervalBetweenPolls", true );
	// Options::Get()->AddOptionBool("ValidateValueChanges", true);
	// Options::Get()->AddOptionString("LogFileName", "info.log", false );
	// Options::Get()->AddOptionBool("AppendLogFile", true );

	Options::Get()->Lock();

	Manager::Create();
	Manager::Get()->AddWatcher( OnNotification, NULL );

	string port = "/dev/ttyACM0";
	Manager::Get()->AddDriver( port );
	//Manager::Get()->SetValue();

	// Log::Create("Log.txt", true, false, LogLevel_Debug, LogLevel_Debug, LogLevel_Debug);

	while(true)
	{
		thread t1(menu);
		t1.join();
	}
	pthread_cond_wait(&initCond, &initMutex);

	if (!g_initFailed)
	{
		pthread_mutex_lock( &g_criticalSection );
		for (list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it)
		{
			NodeInfo* nodeInfo = *it;

			if (nodeInfo->m_homeId == 1)
			{
				std::cout << "This is the controller";
			}

			// printf("NodeID: %d \n ", nodeInfo->m_nodeId);
			// printf("\t NodeName: %s \n ", Manager::Get()->GetNodeName(nodeInfo->m_homeId,nodeInfo->m_nodeId).c_str());
			// printf("\t ManufacturerName: %s \n ", Manager::Get()->GetNodeManufacturerName(nodeInfo->m_homeId,nodeInfo->m_nodeId).c_str());
			// printf("\t NodeProductName: %s \n ", Manager::Get()->GetNodeProductName(nodeInfo->m_homeId,nodeInfo->m_nodeId).c_str());

			// printf("Values announced by the nodes without polling: \n");
			// for (list<ValueID>::iterator it2 = nodeInfo->m_values.begin(); it2 != nodeInfo->m_values.end(); ++it2)
			// {
			// 	ValueID v = *it2;
			// 	printf("\t ValueLabel: %s \n", Manager::Get()->GetValueLabel(v).c_str());
			// 	printf("\t\t ValueType: %s (%d) \n", v.GetTypeAsString().c_str(), v.GetType());
			// 	printf("\t\t ValueHelp: %s \n", Manager::Get()->GetValueHelp(v).c_str());
			// 	printf("\t\t ValueUnits: %s \n", Manager::Get()->GetValueUnits(v).c_str());
			// 	printf("\t\t ValueMin: %d \n", Manager::Get()->GetValueMin(v));
			// 	printf("\t\t ValueMax: %d \n", Manager::Get()->GetValueMax(v));
			// 	printf("\t\t ValueGenre: %s (%d)\n", v.GetGenreAsString().c_str(), v.GetGenre());

			// 	if (v.GetCommandClassId() == COMMAND_CLASS_BASIC)
			// 	{
			// 		Manager::Get()->EnablePoll(v, 2);
			// 		break;
			// 	}
			// }
		}
	pthread_mutex_unlock( &g_criticalSection );

	// 	Driver::DriverData data;
	// 	Manager::Get()->GetDriverStatistics( g_homeId, &data );
	// 	printf("SOF: %d ACK Waiting: %d Read Aborts: %d Bad Checksums: %d\n", data.m_SOFCnt, data.m_ACKWaiting, data.m_readAborts, data.m_badChecksum);
	// 	printf("Reads: %d Writes: %d CAN: %d NAK: %d ACK: %d Out of Frame: %d\n", data.m_readCnt, data.m_writeCnt, data.m_CANCnt, data.m_NAKCnt, data.m_ACKCnt, data.m_OOFCnt);
	// 	printf("Dropped: %d Retries: %d\n", data.m_dropped, data.m_retries);
	}

	return 0;
}
