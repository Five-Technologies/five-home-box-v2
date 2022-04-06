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
#include <bitset>

using namespace OpenZWave;
using namespace Five;
using namespace std;

static pthread_mutex_t g_criticalSection;
static pthread_cond_t  initCond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t initMutex;

bool g_menuLocked{ true };
auto start{ getCurrentDatetime() };
tm* tm_start{ convertDateTime(start) };

static uint8 *bitmap[29];

static list<string> g_setTypes = {"Color", "Switch", "Level", "Duration", "Volume"};
void onNotification(Notification const* notification, void* context);
void menu();

int main(int argc, char const *argv[])
{
	string response;
	cout << "Start process..." << endl;
	cout << ">>──── LOG LEVEL ────<<\n\n"
		 << "     0. NONE\n"
		 << "     1. WARNING\n"
		 << "     2. INFO\n"
		 << "     3. DEBUG\n\n"
		 << "Select the log level (0, 1, 2, 3): ";
	cin >> response;

	switch (stoi(response)) {
	case 0:
		LEVEL = logLevel::NONE;
		break;
	case 1:
		LEVEL = logLevel::WARNING;
		break;
	case 2:
		LEVEL = logLevel::INFO;
		break;
	case 3:
		LEVEL = logLevel::DEBUG;
		break;
	default:
		LEVEL = logLevel::NONE;
		break;
	}

	cout << "Level selected: " << LEVEL << endl;

	for (int i = 0; i < 29; i++) {
		uint8 bite{ 0 };
		uint8 *bitmapPixel = &bite;
		bitmap[i] = bitmapPixel;
	}

	pthread_mutexattr_t mutexattr;

	pthread_mutexattr_init ( &mutexattr );
	pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &g_criticalSection, &mutexattr );
	pthread_mutexattr_destroy( &mutexattr );
	pthread_mutex_lock( &initMutex );

	Options::Create(CONFIG_PATH, CACHE_PATH, "");
	Options::Get()->Lock();
	Manager::Create();
	Manager::Get()->AddWatcher(onNotification, NULL);
	Manager::Get()->AddDriver(PORT);

	if (g_menuLocked) {
		thread t1(menu);
		t1.detach();
		g_menuLocked = false;
	}

	pthread_cond_wait(&initCond, &initMutex);
	pthread_mutex_unlock( &g_criticalSection );

	// 	Driver::DriverData data;
	// 	Manager::Get()->GetDriverStatistics( Five::homeID, &data );
	// 	printf("SOF: %d ACK Waiting: %d Read Aborts: %d Bad Checksums: %d\n", data.m_SOFCnt, data.m_ACKWaiting, data.m_readAborts, data.m_badChecksum);
	// 	printf("Reads: %d Writes: %d CAN: %d NAK: %d ACK: %d Out of Frame: %d\n", data.m_readCnt, data.m_writeCnt, data.m_CANCnt, data.m_NAKCnt, data.m_ACKCnt, data.m_OOFCnt);
	// 	printf("Dropped: %d Retries: %d\n", data.m_dropped, data.m_retries);

	return 0;
}

void onNotification(Notification const* notification, void* context) {
	ofstream myfile;
	ValueID valueID{ notification->GetValueID() };
	string valueLabel;
	uint8 cc_id{ valueID.GetCommandClassId() };
	string cc_name{ Manager::Get()->GetCommandClassName(cc_id) };
	string path = Five::NODE_LOG_PATH + "node_" + to_string(notification->GetNodeId()) + ".log";
	string container;
	string *ptr_container = &container;
	string notifType{ "" };
	string log{ "" };

	Node::NodeData nodeData;
	Node::NodeData* ptr_nodeData = &nodeData;
	Driver::DriverData driver_data;

	if (containsType(notification->GetType(), Five::AliveNotification)) {
		if (containsNodeID(notification->GetNodeId(), (*Five::nodes))) {
			NodeInfo* n = getNode(notification->GetNodeId(), Five::nodes);
			if (n->m_isDead) {
				n->m_isDead = false;
				cout << "\n\n⭐ [NEW_NODE_APPEARS]             node " << to_string(valueID.GetNodeId()) << endl;
				cout << "   - total: " << aliveNodeSum(nodes) + deadNodeSum(nodes) << endl;
				cout << "   - alive: " << aliveNodeSum(nodes) << endl;
				cout << "   - dead : " << deadNodeSum(nodes) << "\n\n" << endl;
			}
		}
	}

	pthread_mutex_lock(&g_criticalSection); // lock critical section
	// uint8 neighborNodeID;
	// uint8 *ptr_neighborNodeID = &neighborNodeID;

	// uint8 *ptr2_neighborNodeID = &ptr_array_neighbotNodeID;

	if (Five::homeID == 0) {
		Five::homeID = notification->GetHomeId();
	}

	switch (notification->GetType()) {
		case Notification::Type_ValueAdded:
			log += "[VALUE_ADDED]	                  node " + to_string(notification->GetNodeId()) + ", value " + to_string(valueID.GetId()) + '\n';
			
			notifType = "VALUE ADDED";
			addValue(valueID, getNode(notification->GetNodeId(), Five::nodes));
			break;
		case Notification::Type_ValueRemoved:
			log += "[VALUE_REMOVED]                   node "
			    + to_string(notification->GetNodeId()) + " value "
				+ to_string(valueID.GetId()) + '\n';
			
			notifType = "VALUE REMOVED";
			removeValue(valueID);
			break;
		case Notification::Type_ValueChanged:
			Manager::Get()->GetValueAsString(valueID, ptr_container);
			Manager::Get()->GetNodeStatistics(valueID.GetHomeId(), valueID.GetNodeId(), ptr_nodeData);
			
			log += "[VALUE_CHANGED]                   node "
			    + to_string(valueID.GetNodeId()) + ", " 
				+ Manager::Get()->GetValueLabel(valueID) + ": " 
				+ *ptr_container + '\n';
			
			notifType = "VALUE CHANGED";
			Manager::Get()->SyncronizeNodeNeighbors(valueID.GetHomeId(), valueID.GetNodeId());
			// cout << "pointer: " << (*ptr_nodeData).m_routeScheme << endl;
			// cout << "route: " << Manager::Get()->GetNodeRouteScheme(ptr_nodeData) << endl;
			Manager::Get()->RequestNodeNeighborUpdate(valueID.GetHomeId(), valueID.GetNodeId());

			valueID = notification->GetValueID();
			// cout << "[" << time(0) << " : VALUE_CHANGED]" << "label: " << valueLabel << ", id: " << v.GetId() << "nodeId: " << v.GetNodeId() << endl;
			break;
		case Notification::Type_ValueRefreshed:
			log += "[VALUE_REFRESHED]                 node " + to_string(valueID.GetNodeId()) + ", "
			     + Manager::Get()->GetValueLabel(valueID) + ": " + *ptr_container + ", "
				 + ", neigbors: " + to_string(Manager::Get()->GetNodeNeighbors(valueID.GetHomeId(), 1, bitmap)) + '\n';
			
			notifType = "VALUE REFRESHED";
			Manager::Get()->GetValueAsString(valueID, ptr_container);

			// for (i = 0; i < 29; i++) {
			// 	// for (j = 0; j < 8; j++) {
			// 	// 	cout << (bitset<8>((*bitmap)[i]))[i] << "  ";
			// 	// }
			// 	// cout << '\n';
			// 	// cout << bitset<8>((*bitmap)[i]) << '\n';
			// 	cout << to_string((*bitmap)[i]) << endl;
			// }
			
			break;
		case Notification::Type_Group:
			notifType = "GROUP";
			break;
		case Notification::Type_NodeNew:
			notifType = "NODE NEW";
			break;
		case Notification::Type_NodeAdded:
			log += "[NODE_ADDED]                      node " + to_string(notification->GetNodeId()) + '\n';
			
			notifType = "NODE ADDED";
			pushNode(notification, Five::nodes);
			break;
		case Notification::Type_NodeRemoved:
			log += "[NODE_REMOVED]                    node " + to_string(notification->GetNodeId()) + '\n';
			notifType = "NODE REMOVED";
			
			removeFile(path);
			removeNode(notification, Five::nodes);
			break;
		case Notification::Type_NodeProtocolInfo:
			notifType = "NODE PROTOCOL INFO";
			break;
		case Notification::Type_NodeNaming:
			log += "[NODE_NAMING]                     node " + to_string(valueID.GetNodeId()) + '\n';
			notifType = "NODE NAMING";
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
			log += "[DRIVER_READY]                    driver READY\n" + getDriverData(notification->GetHomeId()) + '\n';
			notifType = "DRIVER READY";
			Manager::Get()->GetDriverStatistics(notification->GetHomeId(), &driver_data);
			break;
		case Notification::Type_DriverFailed:
			notifType = "DRIVER FAILED";
			break;
		case Notification::Type_DriverReset:
			notifType = "DRIVER RESET";
			break;
		case Notification::Type_EssentialNodeQueriesComplete:
			log += "[ESSENTIAL_NODE_QUERIES_COMPLETE] node " + to_string(notification->GetNodeId()) + ", queries COMPLETE" + '\n';
			notifType = "ESSENTIAL NODE QUERIES COMPLETE";
			// cout << "valueID: " << v.GetAsString() << endl;
			break;
		case Notification::Type_NodeQueriesComplete:
			log += "[NODE_QUERIES_COMPLETE]           node " + to_string(valueID.GetNodeId()) + '\n';
			notifType = "NODE QUERIES COMPLETE";
			break;
		case Notification::Type_AwakeNodesQueried:
			notifType = "AWAKE NODES QUERIED";
			break;
		case Notification::Type_AllNodesQueriedSomeDead:
			log += "\n🚨 [ALL_NODES_QUERIED_SOME_DEAD]  node " + to_string(valueID.GetNodeId()) + '\n'
			     + "   - total: " + to_string(aliveNodeSum(nodes) + deadNodeSum(nodes)) + '\n'
			     + "   - alive: " + to_string(aliveNodeSum(nodes)) + '\n'
			     + "   - dead : " + to_string(deadNodeSum(nodes)) + '\n'
			     + "   - start: " + getTime(convertDateTime(start))
			     + "   - elapse: " + to_string(difference(getCurrentDatetime(), start)) + "s\n" + '\n';			
			notifType = "ALL NODES QUERIED SOME DEAD";
			break;
		case Notification::Type_AllNodesQueried:
			log += "\n✅ [ALL_NODES_QUERIED]            node " + to_string(valueID.GetNodeId()) + '\n'
			     + "   - total: " + to_string(aliveNodeSum(nodes) + deadNodeSum(nodes)) + '\n'
			     + "   - alive: " + to_string(aliveNodeSum(nodes)) + '\n'
			     + "   - dead : " + to_string(deadNodeSum(nodes)) + '\n'
			     + "   - start: " + getTime(convertDateTime(start))
			     + "   - elapse: " + to_string(difference(getCurrentDatetime(), start)) + "s\n" + '\n';
			notifType = "ALL NODES QUERIED";
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
			log += "[MANUFACTURER_SPECIFIC_DB_READY]  manufacturer database READY" + '\n';
			notifType = "MANUFACTURER SPECIFIC DB READY";
			break;
		default:
			break;
	}

	if (notifType == "") {
		notifType = to_string(notification->GetType());
	}

	switch (Five::LEVEL)
	{
	case Five::logLevel::DEBUG:
		cout << log;
		break;
	default:
		break;
	}

	// cout << ">> " << notifType << endl;
	// cout << log;
	
	if (notification->GetType() != Notification::Type_NodeRemoved) {
		myfile.open(path, ios::app);

		myfile << "[" << getDate(convertDateTime(getCurrentDatetime())) << ", "<< getTime(convertDateTime(getCurrentDatetime())) << "] " 
			<< notifType << ", " << cc_name << " --> " 
			<< to_string(valueID.GetIndex()) << "(" << valueLabel << ")\n";

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
		
		while (x --> 0) {
			this_thread::sleep_for(chrono::seconds(1));
		}

		// if (std::find(g_setTypes.begin(), g_setTypes.end(), "Duration") != g_setTypes.end()){
		// 	cout << "Works" << endl;
		// }

		cout << "\n>>──────| MENU |──────<<\n" << endl;
		cout << "     1. Add node" << endl;
		cout << "     2. Remove node" << endl;
		cout << "     3. Get value" << endl;
		cout << "     4. Set value (old)" << endl;
		cout << "     5. Reset Key" << endl;
		cout << "     6. Wake Up" << endl;
		cout << "     7. Heal" << endl;
		cout << "     8. Set value (new)" << endl;
		cout << "     9. Get network" << endl;

		cout << "\nSelect a number: ";
		cin >> response;

		try {
			choice = stoi(response);
		} catch(const std::exception& e) {
			std::cerr << e.what() << '\n';
		}


		switch (choice) {
		case 1:
			Manager::Get()->AddNode(Five::homeID, false);
			break;
		case 2:
			Manager::Get()->RemoveNode(Five::homeID);
			break;
		case 3:
			for(nodeIt =Five::nodes->begin(); nodeIt !=Five::nodes->end(); nodeIt++) {
				counterNode++;
				cout << counterNode << ". " << (*nodeIt)->m_name << endl;
			}

			cout << "\nChoose the node from which you want the values: " << endl;

			cin >> response;
			choice = stoi(response);
			counterNode = 0;
			
			for(nodeIt =Five::nodes->begin(); nodeIt !=Five::nodes->end(); nodeIt++){
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
			for(nodeIt =Five::nodes->begin(); nodeIt !=Five::nodes->end(); nodeIt++)
			{
				counterNode++;
				cout << counterNode << ". " << (*nodeIt)->m_name << endl;
			}

			cout << "\nChoose what node you want a value from: " << endl;

			cin >> response;
			choice = stoi(response);
			counterNode = 0;
			cout << "Choose the value to set: " << endl;
			for(nodeIt =Five::nodes->begin(); nodeIt !=Five::nodes->end(); nodeIt++)
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
					Manager::Get()->ResetController(Five::homeID);
					isOk = true;
				}else if(choice == 2){
					Manager::Get()->SoftReset(Five::homeID);
					isOk = true;
				}else {
					cout << "Please enter 1 or 2\n";
				}
			}
			menuRun = 0;
			break;
		case 6:
			for (nodeIt =Five::nodes->begin(); nodeIt !=Five::nodes->end(); nodeIt++){
				cout << unsigned((*nodeIt)->m_nodeId) << ". " << (*nodeIt)->m_name << endl;
			}
			cout << "Which node do you want to heal ?";
			cin >> response;
			choice = stoi(response);

			for (nodeIt =Five::nodes->begin(); nodeIt !=Five::nodes->end(); nodeIt++){
				if ((*nodeIt)->m_nodeId == choice){
					Manager::Get()->HealNetworkNode((*nodeIt)->m_homeId, (*nodeIt)->m_nodeId, true);
				}
			}
			break;
		case 7:
			break;
		case 8:
			for (nodeIt =Five::nodes->begin(); nodeIt !=Five::nodes->end(); nodeIt++)
			{
				cout << unsigned((*nodeIt)->m_nodeId) << ". " << (*nodeIt)->m_name << endl;
			}

			cout << "\nChoose what node you want a value from: " << endl;

			cin >> response;
			choice = stoi(response);
			//counterNode = 0;
			for (nodeIt =Five::nodes->begin(); nodeIt !=Five::nodes->end(); nodeIt++)
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
						Five::setList((*valueIt));
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
								setSwitch((*valueIt), true);
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
			for (int i = 0; i < int(fileName.length()); i++) {
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

		// Manager::Get()->AddNode(Five::homeID, false);
		// Manager::Get()->RemoveNode(Five::homeID);
		// cout << "Node removed" << endl;
		// Manager::Get()->TestNetwork(Five::homeID, 5);
		// cout << "Name: " << Manager::Get()->GetNodeProductName(Five::homeID, 2).c_str() << endl;
	}
}