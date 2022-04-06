#include <iostream>
#include <algorithm>
#include <cstddef>
#include "Manager.h"
#include "Options.h"
#include "Notification.h"
#include "platform/Log.h"
#include "Node.h"
#include <thread>
#include "Five.h"
#include <fstream>
#include <ctime>
#include <cstdint>
#include <filesystem>
#include <cmath>

using namespace OpenZWave;
using namespace Five;
using namespace std;
using namespace Internal::CC;

static uint32 g_homeId{ 0 };
static list<NodeInfo*> g_nodes = {};
static pthread_mutex_t g_criticalSection;
static pthread_cond_t  initCond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t initMutex;
static bool g_menuLocked{ true };
static auto start = chrono::high_resolution_clock::now();
static time_t timet_start = chrono::high_resolution_clock::to_time_t(start);
static time_t *ptr_start = &timet_start;
static tm* tm_start = localtime(ptr_start);

static list<string> g_setTypes = {"Color", "Switch", "Level", "Duration", "Volume"};

NodeInfo* newNode(Notification* const notification);
void onNotification(Notification const* notification, void* context);
void menu();
bool isNewNode(uint8 nodeID);
NodeInfo* getNodeInfo(uint8 nodeID, list<NodeInfo*> nodes);
void refreshNode(ValueID valueID, NodeInfo* oldNodeInfo);
bool addNode(Notification const notification);
bool removeNode(Notification const* notification);

int main(int argc, char const *argv[])
{
	cout << "Start process..." << endl;

	pthread_mutexattr_t mutexattr;
	string response;

	pthread_mutexattr_init ( &mutexattr );
	pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &g_criticalSection, &mutexattr );
	pthread_mutexattr_destroy( &mutexattr );
	pthread_mutex_lock( &initMutex );

	Options::Create("config/", "cpp/examples/cache/", "");
	Options::Get()->Lock();

	Manager::Create();
	Manager::Get()->AddWatcher(onNotification, NULL);

	string port = "/dev/ttyACM0";
	Manager::Get()->AddDriver(port);

	pthread_cond_wait(&initCond, &initMutex);
	pthread_mutex_unlock( &g_criticalSection );

	// 	Driver::DriverData data;
	// 	Manager::Get()->GetDriverStatistics( g_homeId, &data );
	// 	printf("SOF: %d ACK Waiting: %d Read Aborts: %d Bad Checksums: %d\n", data.m_SOFCnt, data.m_ACKWaiting, data.m_readAborts, data.m_badChecksum);
	// 	printf("Reads: %d Writes: %d CAN: %d NAK: %d ACK: %d Out of Frame: %d\n", data.m_readCnt, data.m_writeCnt, data.m_CANCnt, data.m_NAKCnt, data.m_ACKCnt, data.m_OOFCnt);
	// 	printf("Dropped: %d Retries: %d\n", data.m_dropped, data.m_retries);

	return 0;
}

// Create a new NodeInfo.
NodeInfo* newNode(Notification const* notification) {
	uint32 homeId = notification->GetHomeId();
	uint8 nodeId = notification->GetNodeId();
	ValueID valueID = notification->GetValueID();
	string name = Manager::Get()->GetNodeProductName(homeId, nodeId);
	string type = valueID.GetTypeAsString();

	NodeInfo *n = new NodeInfo();
	n->m_homeId		= homeId;
	n->m_nodeId		= nodeId;
	n->m_name     	= name;
	n->m_nodeType 	= type;

	return n;
}

// Check if <nodeID> exists in <nodes>.
bool isNewNode(uint8 nodeID) {
	list<NodeInfo*>::iterator it;
	for (it = g_nodes.begin(); it != g_nodes.end(); it++) {
		// cout << "it: " << to_string((*it)->m_nodeId) << " nodeID: " << to_string(nodeID) << ", ";
		if ((*it)->m_nodeId == nodeID) {
			return false;
		}
	}
	return true;
}

// Get the NodeInfo thanks to the <nodeID>.
NodeInfo* getNodeInfo(uint8 nodeID, list<NodeInfo*> nodes) {
	list<NodeInfo*>::iterator it;

	for (it = nodes.begin(); it != nodes.end(); it++) {
		if ((*it)->m_nodeId == nodeID) {
			return *it;
		}
	}
	return NULL;
}

// Refresh members in the oldNodeInfo thanks to this valueID.
void refreshNode(ValueID valueID, NodeInfo* oldNodeInfo) {
	oldNodeInfo->m_homeId = valueID.GetHomeId();
	oldNodeInfo->m_name = Manager::Get()->GetNodeName(valueID.GetHomeId(), valueID.GetNodeId());
	oldNodeInfo->m_nodeType = Manager::Get()->GetNodeType(valueID.GetHomeId(), valueID.GetNodeId());
	oldNodeInfo->m_values.push_back(valueID);
}

bool addNode(Notification const *notification) {
	NodeInfo* n{ newNode(notification) };
	uint8 nodeID{ notification->GetNodeId() };

	if (isNewNode(nodeID)) {
		g_nodes.push_back(n);
		return true;
	}
	return false;
}

bool removeNode(Notification const *notification) {
	uint8 nodeID{ notification->GetNodeId() };
	list<NodeInfo*>::iterator it;

	for (it = g_nodes.begin(); it != g_nodes.end(); it++) {
		if ((*it)->m_nodeId == nodeID) {
			g_nodes.remove(*it);
			return true;
		}
	}
	return false;
}

bool addValue(Notification const* notification) {
	uint8 nodeID{ notification->GetNodeId() };
	list<NodeInfo*>::iterator it;

	for (it = g_nodes.begin(); it != g_nodes.end(); it++) {
		if ((*it)->m_nodeId == nodeID) {
			(*it)->m_name = Manager::Get()->GetNodeProductName(notification->GetHomeId(), nodeID);
			(*it)->m_values.push_back(notification->GetValueID());
			return true;
		}
	}
	return false;
}

bool removeValue(ValueID valueID) {
	uint8 nodeID{ valueID.GetNodeId() };
	list<NodeInfo*>::iterator it;
	list<ValueID>* valueIDs;
	list<ValueID>::iterator it2;

	for (it = g_nodes.begin(); it != g_nodes.end(); it++) {
		if ((*it)->m_nodeId == nodeID) {
			valueIDs = &((*it)->m_values);

			for (it2 = valueIDs->begin(); it2 != valueIDs->end(); it2++) {
				if (*it2 == valueID) {
					valueIDs->remove(valueID);
					return true;
				}
			}
			return false;
		}
	}
	return false;
}

bool removeFile(string relativePath) {
	char arr[relativePath.length()];
	strcpy(arr, relativePath.c_str());
	remove(arr);
	return true;
}

void onNotification(Notification const* notification, void* context) {
	list<NodeInfo*>::iterator it;
	list<ValueID>::iterator it2;
	ofstream myfile;
	ValueID v;
	string valueLabel;
	uint8 cc_id;
	string cc_name;
	string container;
	string *ptr_container = &container;

	v = notification->GetValueID();
	cc_id = v.GetCommandClassId();
	cc_name = Manager::Get()->GetCommandClassName(cc_id);
	
	bool isNewNode = true;

	string path = "cpp/examples/cache/nodes/node_" + to_string(notification->GetNodeId()) + ".log";
	int n = path.length();
	char charArray[n + 1];
	strcpy(charArray, path.c_str());

	chrono::system_clock::time_point tp_now = chrono::system_clock::now();
	time_t tt_now = chrono::system_clock::to_time_t(tp_now);
	const time_t* ptr_tt_now = &tt_now;
	tm* tm_now = localtime(ptr_tt_now);
	
	string formatHour = to_string(tm_now->tm_hour) + ":" + to_string(tm_now->tm_min) + ":" + to_string(tm_now->tm_sec);
	string formatDate = to_string(tm_now->tm_mday) + "/" + to_string(tm_now->tm_mon);
	chrono::duration<double> elapsed_seconds = tp_now - start;
	double rounded_elapsed = ceil(elapsed_seconds.count() * 1000) / 1000;
	string notifType{ "" };


	pthread_mutex_lock(&g_criticalSection); // lock critical section

	if (g_homeId == 0) {
		g_homeId = notification->GetHomeId();
	}

	switch (notification->GetType()) {
		case Notification::Type_ValueAdded:
			notifType = "VALUE ADDED";
			if (addValue(notification)) {
				cout << "[VALUE_ADDED]	                  node " << to_string(notification->GetNodeId()) << ", value " << v.GetId() << endl;
			}
			break;
		case Notification::Type_ValueRemoved:
			// notifType = "VALUE REMOVED";
			if (removeValue(v)) {
				cout << "[VALUE_REMOVED]                   node " << to_string(notification->GetNodeId()) << " value " << v.GetId() << endl;
			}
			break;
		case Notification::Type_ValueChanged:
			notifType = "VALUE CHANGED";
			
			v = notification->GetValueID();
			// cout << "[" << time(0) << " : VALUE_CHANGED]" << "label: " << valueLabel << ", id: " << v.GetId() << "nodeId: " << v.GetNodeId() << endl;
			break;
		case Notification::Type_ValueRefreshed:
			notifType = "VALUE REFRESHED";

			// for (it = g_nodes.begin(); it != g_nodes.end(); it++) {
			// 	if ((*it)->m_nodeId == notification->GetNodeId()) {
			// 		for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++) {
			// 			cout << it2->GetId();
			// 		}
			// 		cout << endl;
			// 	}
			// }
			break;
		case Notification::Type_Group:
			notifType = "GROUP";
			break;
		case Notification::Type_NodeNew:
			notifType = "NODE NEW";
			break;
		case Notification::Type_NodeAdded:
			// notifType = "NODE ADDED";
			if (addNode(notification)) {
				cout << "[NODE_ADDED]                      node " << to_string(notification->GetNodeId()) << endl;
			}
			break;
		case Notification::Type_NodeRemoved:
			// notifType = "NODE REMOVED";
			if (removeNode(notification)) {
				cout << "[NODE_REMOVED]                    node " << to_string(notification->GetNodeId()) << endl;
				removeFile("cpp/examples/cache/nodes/node_" + to_string(notification->GetNodeId()) + ".log");
			}
			break;
		case Notification::Type_NodeProtocolInfo:
			notifType = "NODE PROTOCOL INFO";
			break;
		case Notification::Type_NodeNaming:
			notifType = "NODE NAMING";
			cout << "[NODE_NAMING]                     node " << to_string(v.GetNodeId()) << endl;
			break;
		case Notification::Type_NodeEvent:
			notifType = "NODE EVENT";
			break;
		case Notification::Type_PollingDisabled:
			notifType = "POLLING DISABLED";
			break;
		case Notification::Type_PollingEnabled:
			notifType = "POLLING ENABLED";
			break;
		case Notification::Type_SceneEvent:
			notifType = "SCENE EVENT";
			break;
		case Notification::Type_CreateButton:
			notifType = "CREATE BUTTON";
			break;
		case Notification::Type_DeleteButton:
			notifType = "DELETE BUTTON";
			break;
		case Notification::Type_ButtonOn:
			notifType = "BUTTON ON";
			break;
		case Notification::Type_ButtonOff:
			notifType = "BUTTON OFF";
			break;
		case Notification::Type_DriverReady:
			notifType = "DRIVER READY";
			cout << "[DRIVER_READY]                    driver READY" << endl;
			break;
		case Notification::Type_DriverFailed:
			notifType = "DRIVER FAILED";
			break;
		case Notification::Type_DriverReset:
			notifType = "DRIVER RESET";
			break;
		case Notification::Type_EssentialNodeQueriesComplete:
			notifType = "ESSENTIAL NODE QUERIES COMPLETE";
			cout << "[ESSENTIAL_NODE_QUERIES_COMPLETE] node " << to_string(notification->GetNodeId()) << ", queries COMPLETE" << endl;
			// cout << "valueID: " << v.GetAsString() << endl;
			break;
		case Notification::Type_NodeQueriesComplete:
			notifType = "NODE QUERIES COMPLETE";
			cout << "[NODE_QUERIES_COMPLETE]           node " << to_string(v.GetNodeId()) << endl;
			break;
		case Notification::Type_AwakeNodesQueried:
			notifType = "AWAKE NODES QUERIED";
			break;
		case Notification::Type_AllNodesQueriedSomeDead:
			notifType = "ALL NODES QUERIED SOME DEAD";
			break;
		case Notification::Type_AllNodesQueried:
			notifType = "ALL NODES QUERIED";
			cout << "\n✅ [ALL_NODES_QUERIED]               node " << to_string(v.GetNodeId()) << endl;
			cout << "   - total: " << g_nodes.size() << endl;
			cout << "   - alive: " << endl;
			cout << "   - dead : " << endl;
			cout << "   - start: " << asctime(tm_start);
			cout << "   - elapse: " << rounded_elapsed << "s\n" << endl;
			
			if (g_menuLocked) {
				thread t1(menu);
				t1.detach();
				g_menuLocked = false;
			}
			break;
		case Notification::Type_Notification:
			notifType = "NOTIFICATION";
			break;
		case Notification::Type_DriverRemoved:
			notifType = "DRIVER REMOVED";
			break;
		case Notification::Type_ControllerCommand:
			notifType = "CONTROLLER COMMAND";

			// cout << "Create command class..." << endl;
			// cout << notification->Type_ControllerCommand << notification->GetCommand();
			break;
		case Notification::Type_NodeReset:
			notifType = "NODE RESET";
			break;
		case Notification::Type_UserAlerts:
			notifType = "USER ALERTS";
			break;
		case Notification::Type_ManufacturerSpecificDBReady:
			// The valueID is empty, you can't use it here.
			notifType = "MANUFACTURER SPECIFIC DB READY";
			cout << "[MANUFACTURER_SPECIFIC_DB_READY]  manufacturer database READY" << endl;
			
			break;
		default:
			break;
	}

	if (notifType == "") {
		notifType = to_string(notification->GetType());
	}

	// cout << ">> " << notifType << endl;
	
	if (notification->GetType() != Notification::Type_NodeRemoved) {
		myfile.open(path, ios::app);

		myfile << "[" << formatDate << ", "<< formatHour << "] " 
			<< notifType << ", " << cc_name << " --> " 
			<< to_string(v.GetIndex()) << "(" << valueLabel << ")\n";

		myfile.close();
	}

	pthread_mutex_unlock(&g_criticalSection); // unlock critical section
}

void menu() {
	bool menuRun(1);
	while(menuRun){
		string response;
		bool isOk = false;
		list<string>::iterator sIt;
		int choice{ 0 };
		int listchoice{ 0 };
		int x{ 5 };
		int counterNode{0};
		int counterValue{0};
		list<NodeInfo*>::iterator nodeIt;
		list<ValueID>::iterator valueIt;
		string container;
		string* ptr_container = &container;
		string fileName{ "" };
		
		//int status;

		while (x --> 0) {
			std::cout << x << endl;
			this_thread::sleep_for(chrono::seconds(1));
		}

		if (std::find(g_setTypes.begin(), g_setTypes.end(), "Duration") != g_setTypes.end()){
			cout << "Works" << endl;
		}

		cout << "----- MENU -----" << endl << endl;
		cout << "1. Add node" << endl;
		cout << "2. Remove node" << endl;
		cout << "3. Get value" << endl;
		cout << "4. Set value" << endl;
		cout << "5. Reset Key" << endl;
		cout << "6. Wake Up" << endl;
		cout << "7. Heal" << endl;
		cout << "8. NewSetValue" << endl;

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

			cout << "\nChoose the node from which you want the values: " << endl;

			cin >> response;
			choice = stoi(response);
			counterNode = 0;
			
			for(nodeIt = g_nodes.begin(); nodeIt != g_nodes.end(); nodeIt++){
				counterNode++;
				if (counterNode == choice) {
					for(valueIt = (*nodeIt) -> m_values.begin(); valueIt != (*nodeIt) -> m_values.end(); valueIt++) {
						Manager::Get()->GetValueAsString((*valueIt), ptr_container);
						cout << counterValue << ". " << Manager::Get()->GetValueLabel(*valueIt) << " : " << *ptr_container << endl;
						counterValue++;
					}
					
					// cout << "\nChoose a valueID: ";
					// cin >> response;
					// choice = stoi(response);

					// for (valueIt = (*nodeIt)->m_values.begin(); valueIt != (*nodeIt)->m_values.end(); valueIt++) {
					// 	if (choice == std::distance((*nodeIt)->m_values.begin(), valueIt)) {
					// 		cout << Manager::Get()->GetValueLabel(*valueIt) << valueIt->GetAsString() << endl;
					// 		Manager::Get()->GetValueAsString((*valueIt), ptr_container);
					// 		cout << "Current value: " << *ptr_container << endl;
					// 		break;
					// 	}
					// }
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

			cout << "\nChoose what node you want a value from: " << endl;

			cin >> response;
			choice = stoi(response);
			counterNode = 0;
			cout << "Choose the value to set: " << endl;
			for(nodeIt = g_nodes.begin(); nodeIt != g_nodes.end(); nodeIt++)
			{
				counterNode++;
				if (counterNode == choice)
				{
					for(valueIt = (*nodeIt) -> m_values.begin(); valueIt != (*nodeIt) -> m_values.end(); valueIt++)
					{
						if(!Manager::Get()->IsValueReadOnly(*valueIt)){
							counterValue++;
							cout << counterValue << ". " << Manager::Get()->GetValueLabel(*valueIt) << endl;
						}
						
					}

					cout << "\nChoose a valueID: ";
					cin >> response;
					counterValue = 0;
					choice = stoi(response);

					for (valueIt = (*nodeIt)->m_values.begin(); valueIt != (*nodeIt)->m_values.end(); valueIt++) {
						// Manager::Get()->GetValueAsString(*valueIt, ptr_container);
						// cout << Manager::Get()->GetValueLabel(*valueIt) << ": " << *ptr_container << endl;
						if(!Manager::Get()->IsValueReadOnly(*valueIt)){
							counterValue++;
						}
						
						if (choice == counterValue) {
							cout << Manager::Get()->GetValueLabel(*valueIt) << valueIt->GetAsString() << endl;
							Manager::Get()->GetValueAsString((*valueIt), ptr_container);
							cout << "Current value: " << *ptr_container << endl;
							cout << "Set to what ? ";
							cin >> response;
							// int test = 0;
							// int* testptr = &test;
							//setUnit((*valueIt));
							Manager::Get()->SetValue((*valueIt), response);
							//Manager::Get()->GetValueAsInt((*valueIt), testptr);
							//cout << *testptr;
							break;
						}
					}
					break;
				}
			}
			// cin >> response;
			// choice = stoi(response);
			// counterNode = 0;
			// counterValue = 0;
			
			// for(valueIt = (*nodeIt) -> m_values.begin(); valueIt != (*nodeIt) -> m_values.end(); valueIt++)
			// {
			// 	counterValue++;
			// 	if (counterValue == choice)
			// 		{
			// 			Manager::Get()->GetValueAsString(*valueIt, ptr_container);
			// 			cout << "The current value is: " << ptr_container << endl;
			// 			cout << "Enter the new value: " << endl;
			// 			cin >> response;
			// 			Manager::Get()->SetValue(*valueIt, response);
			// 		}
			// }

			break;
		case 5:
			cout << "Choose between: " << endl;
			cout << "1. Hard Reset (Z-Wave Network will be deleted during reset)\n" << "2. Soft Reset (Z-Wave Network will be kept during reset)\n";
			while(!isOk){
				cin >> response;
				choice = stoi(response);
				if(choice == 1){
					Manager::Get()->ResetController(g_homeId);
					isOk = true;
				}else if(choice == 2){
					Manager::Get()->SoftReset(g_homeId);
					isOk = true;
				}else {
					cout << "Please enter 1 or 2\n";
				}
			}
			menuRun = 0;
			break;
		case 6:
			for (nodeIt = g_nodes.begin(); nodeIt != g_nodes.end(); nodeIt++){
				cout << unsigned((*nodeIt)->m_nodeId) << ". " << (*nodeIt)->m_name << endl;
			}
			cout << "Which node do you want to heal ?";
			cin >> response;
			choice = stoi(response);

			for (nodeIt = g_nodes.begin(); nodeIt != g_nodes.end(); nodeIt++){
				if ((*nodeIt)->m_nodeId == choice){
					Manager::Get()->HealNetworkNode((*nodeIt)->m_homeId, (*nodeIt)->m_nodeId, true);
				}
			}
			break;
		case 7:
			break;
		case 8:
			for (nodeIt = g_nodes.begin(); nodeIt != g_nodes.end(); nodeIt++)
			{
				cout << unsigned((*nodeIt)->m_nodeId) << ". " << (*nodeIt)->m_name << endl;
			}

			cout << "\nChoose what node you want a value from: " << endl;

			cin >> response;
			choice = stoi(response);
			//counterNode = 0;
			for (nodeIt = g_nodes.begin(); nodeIt != g_nodes.end(); nodeIt++)
			{
				if ((*nodeIt)->m_nodeId == choice)
				{
					for (valueIt = (*nodeIt)->m_values.begin(); valueIt != (*nodeIt)->m_values.end(); valueIt++)
					{
						if ((ValueID::ValueType_List == (*valueIt).GetType() || ValueID::ValueType_Button == (*valueIt).GetType()) && !Manager::Get()->IsValueReadOnly(*valueIt))
						{
							counterValue++;
							cout << counterValue << ". " << Manager::Get()->GetValueLabel((*valueIt)) << endl;
						}else for(sIt = g_setTypes.begin(); sIt != g_setTypes.end(); ++sIt){
							if (Manager::Get()->GetValueLabel((*valueIt)).find((*sIt)) != string::npos && !Manager::Get()->IsValueReadOnly(*valueIt))
							{
								counterValue++;
								cout << counterValue << ". " << Manager::Get()->GetValueLabel((*valueIt)) << endl;
							}
						}
						
					}

					break;
				}
			}
			cin >> response;
			choice = stoi(response);
			counterValue = 0;
			for (valueIt = (*nodeIt)->m_values.begin(); valueIt != (*nodeIt)->m_values.end(); valueIt++)
			{
				if (ValueID::ValueType_Button == (*valueIt).GetType() && !Manager::Get()->IsValueReadOnly(*valueIt))
				{
					counterValue++;
					if (choice == counterValue)
					{
						setButton((*valueIt));
					}
					
				}
				if (ValueID::ValueType_List == (*valueIt).GetType() && !Manager::Get()->IsValueReadOnly(*valueIt))
				{
					counterValue++;
					if (choice == counterValue)
					{
						setList((*valueIt));
					}
					
				}else for (sIt = g_setTypes.begin(); sIt != g_setTypes.end(); ++sIt){
					if(Manager::Get()->GetValueLabel((*valueIt)).find((*sIt)) != string::npos && !Manager::Get()->IsValueReadOnly(*valueIt)){
						counterValue++;
						if (choice == counterValue)
						{
							string valLabel = Manager::Get()->GetValueLabel(*valueIt);
							cout << "You chose " << valLabel << endl;
							Manager::Get()->GetValueAsString((*valueIt), ptr_container);
							cout << "Current value: " << *ptr_container << endl;
							// cout << "Set to what ? ";
							//cin >> response;

							//Checking value type to choose the right method
							if(valLabel.find("Switch") != string::npos){
								setSwitch((*valueIt));
							}else if(valLabel.find("Color") != string::npos && (*valueIt).GetType() == ValueID::ValueType_String)
							{
								setColor(*valueIt);
							} else if(valLabel.find("Level") != string::npos && (*valueIt).GetType() == ValueID::ValueType_Byte)
							{
								cout << "Choose a value between:" << endl << "1. Very High\n" << "2. High\n" << "3. Medium\n" << "4. Low\n" << "5. Very Low\n"; 
								while(!isOk){
									cin >> response;
									try{
										listchoice = stoi(response);
										isOk = true;
									} catch(exception &err){
										cout << "Please enter an integer" << endl;
									}
								}
								
								listchoice = stoi(response);
								switch(listchoice){
									case 1:
										setIntensity((*valueIt), IntensityScale::VERY_HIGH);
										break;
									case 2:
										setIntensity((*valueIt), IntensityScale::HIGH);
										break;
									case 3:
										setIntensity((*valueIt), IntensityScale::MEDIUM);
										break;
									case 4:
										setIntensity((*valueIt), IntensityScale::LOW);
										break;
									case 5:
										setIntensity((*valueIt), IntensityScale::VERY_LOW);
										break;
								}
								
							}else if(valLabel.find("Volume") != string::npos)
							{
								cout << "Choose a value between:" << endl << "1. Very High\n" << "2. High\n" << "3. Medium\n" << "4. Low\n" << "5. Very Low\n"; 
								cin >> response;
								listchoice = stoi(response);
								switch(listchoice){
									case 1:
										setIntensity((*valueIt), IntensityScale::VERY_HIGH);
										break;
									case 2:
										setIntensity((*valueIt), IntensityScale::HIGH);
										break;
									case 3:
										setIntensity((*valueIt), IntensityScale::MEDIUM);
										break;
									case 4:
										setIntensity((*valueIt), IntensityScale::LOW);
										break;
									case 5:
										setIntensity((*valueIt), IntensityScale::VERY_LOW);
										break;
									default:
										break;
								}	
							}else if(valLabel.find("Duration") != string::npos)
							{
								cout << "test" << endl;
								setDuration((*valueIt));
							}else if((*valueIt).GetType() == ValueID::ValueType_Int){
								setInt(*valueIt);
							}else if((*valueIt).GetType() == ValueID::ValueType_Bool){
								setBool(*valueIt);
							}
							//Manager::Get()->SetValue((*valueIt), response);
							break;
						}
					}
				}
				
				/*if ((std::find(g_setTypes.begin(), g_setTypes.end(), Manager::Get()->GetValueLabel((*valueIt))) != g_setTypes.end()))
				{
					counterValue++;
					if (choice == counterValue)
					{
						string valLabel = Manager::Get()->GetValueLabel(*valueIt);
						cout << "You chose " << valLabel << endl;
						Manager::Get()->GetValueAsString((*valueIt), ptr_container);
						// cout << "Current value: " << *ptr_container << endl;
						// cout << "Set to what ? ";
						//cin >> response;

						//Checking value type to choose the right method
						if(valLabel == "Switch"){
							cout << "True(1) or False(0) ?" << endl;
							cin >> response;
							choice = stoi(response);
							setSwitch((*valueIt), choice);
						}else if(valLabel == "Color")
						{
							setColor(*valueIt);
						} else if(valLabel == "Level")
						{
							cout << "Choose a value between:" << endl << "1. Very High\n" << "2. High\n" << "3. Medium\n" << "4. Low\n" << "5. Very Low\n"; 
							cin >> response;
							choice = stoi(response);
							switch(choice){
								case 1:
									setIntensity((*valueIt), IntensityScale::VERY_HIGH);
									break;
								case 2:
									setIntensity((*valueIt), IntensityScale::HIGH);
									break;
								case 3:
									setIntensity((*valueIt), IntensityScale::MEDIUM);
									break;
								case 4:
									setIntensity((*valueIt), IntensityScale::LOW);
									break;
								case 5:
									setIntensity((*valueIt), IntensityScale::VERY_LOW);
									break;
							}
							
						}
						//Manager::Get()->SetValue((*valueIt), response);
						break;
					}
				}*/
			}
			break;
		default:
			cout << "You must enter 1, 2, 3 or 4." << endl;
			break;
		}

		if (fileName.size() > 0) {
			char arr[fileName.length()];
			strcpy(arr, fileName.c_str());
			for (int i = 0; i < fileName.length(); i++) {
				cout << arr[i];
			}
			cout << endl;
			// int i;
			// int counter{ fileName.size() + 1 };
			// char 
			// const char *fileChar = fileName.c_str();
			// cout << (*fileChar)[0] << (*fileChar)[1] << endl;
			// char[counter] fileChar = 
			// for (i = 0; i < fileName.size(); i++) {
			// 	fileChar app fileName.at(i);
			// }
		}

		// Manager::Get()->AddNode(g_homeId, false);
		// Manager::Get()->RemoveNode(g_homeId);
		// cout << "Node removed" << endl;
		// Manager::Get()->TestNetwork(g_homeId, 5);
		// cout << "Name: " << Manager::Get()->GetNodeProductName(g_homeId, 2).c_str() << endl;
	}
}