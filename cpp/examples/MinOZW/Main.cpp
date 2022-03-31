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

static uint32 g_homeId{ 0 };
static list<NodeInfo*> g_nodes;
static pthread_mutex_t g_criticalSection;
static pthread_cond_t  initCond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t initMutex;
static bool g_menuLocked{ true };

NodeInfo* getNodeInfo(Notification* notification);
void onNotification(Notification const* notification, void* context);
void menu();

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
	Options::Get()->Lock();

	Manager::Create();
	Manager::Get()->AddWatcher(onNotification, NULL );

	string port = "/dev/ttyACM0";
	Manager::Get()->AddDriver( port );

	while (true) {
		thread t1(menu);
		t1.join();
	}

	pthread_cond_wait(&initCond, &initMutex);
	pthread_mutex_unlock( &g_criticalSection );

	// 	Driver::DriverData data;
	// 	Manager::Get()->GetDriverStatistics( g_homeId, &data );
	// 	printf("SOF: %d ACK Waiting: %d Read Aborts: %d Bad Checksums: %d\n", data.m_SOFCnt, data.m_ACKWaiting, data.m_readAborts, data.m_badChecksum);
	// 	printf("Reads: %d Writes: %d CAN: %d NAK: %d ACK: %d Out of Frame: %d\n", data.m_readCnt, data.m_writeCnt, data.m_CANCnt, data.m_NAKCnt, data.m_ACKCnt, data.m_OOFCnt);
	// 	printf("Dropped: %d Retries: %d\n", data.m_dropped, data.m_retries);

	return 0;
}

NodeInfo* getNodeInfo(Notification const* notification) {
	uint32 const homeId = notification->GetHomeId();
	uint8 const nodeId = notification->GetNodeId();
	for( list<NodeInfo*>::iterator it = g_nodes.begin(); it != g_nodes.end(); ++it ) {
		NodeInfo* nodeInfo = *it;
		if((nodeInfo->m_homeId == homeId) && (nodeInfo->m_nodeId == nodeId)) {
			return nodeInfo;
		}
	}

	return NULL;
}

void onNotification(Notification const* notification, void* context) {
    ValueID v;
	list<NodeInfo*>::iterator it;
	list<ValueID>::iterator it2;
	string valueLabel;
	NodeInfo* nodeInfo = new NodeInfo();

	pthread_mutex_lock(&g_criticalSection); // lock critical section

	if (g_homeId == 0)
		g_homeId = notification->GetHomeId();
	
	switch (notification->GetType()) {
		case Notification::Type_ValueAdded:
			//We get from the notification the values's ID and name.
			v = notification->GetValueID();
			valueLabel = Manager::Get()->GetValueLabel(v);

			//We add the value to the value list of the node
			for(it = g_nodes.begin(); it != g_nodes.end(); ++it ) {
				uint8 nodeId = (*it) -> m_nodeId;
				if ( nodeId == v.GetNodeId() ) {
					((*it) -> m_values).push_back(v);
				}
					
			}
			// cout << "[" << time(0) << " : VALUE_ADDED] label: " << valueLabel << ", id: " << v.GetId() << "nodeId: " << v.GetNodeId() << endl;
			break;
		case Notification::Type_ValueRemoved:
			//We get from the notification the values's ID and name.
<<<<<<< HEAD
			v = notification->GetValueID();
			valueLabel = Manager::Get()->GetValueLabel(v);
=======
			//v = notification->GetValueID();
			//valueLabel = Manager::Get()->GetValueLabel(v);
>>>>>>> 72c79f56cae254b83ea35cd4c58ec6d80e684693


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
			// cout << NotificationService::valueRefreshed(notification, g_nodes) << endl;

			for (it = g_nodes.begin(); it != g_nodes.end(); it++) {
				if ((*it)->m_nodeId == notification->GetNodeId()) {
					for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++) {
						cout << it2->GetId();
					}
					cout << endl;
				}
			}
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
}

void menu() {
	string response;
	int choice{ 0 };
	int x{ 5 };
	int counterNode{0};
	int counterValue{0};
	list<NodeInfo*>::iterator nodeIt;
	list<ValueID>::iterator valueIt;
	
	string container;
	string* ptr_container = &container;

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
        for(nodeIt = g_nodes.begin(); nodeIt != g_nodes.end(); nodeIt++) {
			counterNode++;
			cout << counterNode << ". " << (*nodeIt)->m_name << endl;
		}

		cout << "\nChoose what node you want a value from: " << endl;

		cin >> response;
		choiceNode = stoi(response);
		counterNode = 0;
		
		for(nodeIt = g_nodes.begin(); nodeIt != g_nodes.end(); nodeIt++){
			counterNode++;
			if (counterNode == choice) {
				for(valueIt = (*nodeIt) -> m_values.begin(); valueIt != (*nodeIt) -> m_values.end(); valueIt++) {
					cout << counterValue << ". " << Manager::Get()->GetValueLabel(*valueIt) << endl;
					counterValue++;
				}
				
				cout << "\nChoose a valueID: ";
				cin >> response;
				choice = stoi(response);

				for (valueIt = (*nodeIt)->m_values.begin(); valueIt != (*nodeIt)->m_values.end(); valueIt++) {
					if (choice == std::distance((*nodeIt)->m_values.begin(), valueIt)) {
						cout << Manager::Get()->GetValueLabel(*valueIt) << valueIt->GetAsString() << endl;
						Manager::Get()->GetValueAsString((*valueIt), ptr_container);
						cout << "Current value: " << *ptr_container << endl;
						cout << "Set to what ? ";
						cin >> response;
						Manager::Get()->SetValue((*valueIt), response);
						break;
					}
				}
			}
		}


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