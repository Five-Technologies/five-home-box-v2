#include <iostream>
#include <algorithm>
#include <cstddef>
#include "Manager.h"
#include "Options.h"
#include "Driver.h"
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
#include "command_classes/CommandClass.h"

using namespace OpenZWave;
using namespace Five;
using namespace std;

static pthread_mutex_t g_criticalSection;
static pthread_cond_t  initCond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t initMutex;

bool g_menuLocked{ true };
bool g_checkLocked(1);
auto start{ getCurrentDatetime() };
tm* tm_start{ convertDateTime(start) };

static uint8 *bitmap[29];

void onNotification(Notification const* notification, void* context);
void menu();
void nodeSwitch(int stateInt, int *lock);
void CheckFailedNode(string path);
void statusObserver(list<NodeInfo*> *nodes);

int main(int argc, char const *argv[]) {
	STARTED_AT = getCurrentDatetime().time_since_epoch().count();
	
	string response{ "3" };
	cout << "Start process..." << endl;

	// cout << ">>──── LOG LEVEL ────<<\n\n"
	// 	 << "     0. NONE\n"
	// 	 << "     1. WARNING\n"
	// 	 << "     2. INFO\n"
	// 	 << "     3. DEBUG\n\n"
	// 	 << "Select the log level (0, 1, 2, 3): ";
	// cin >> response;

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

	// Initialize the mutex ATTRIBUTES with the default value.
	pthread_mutexattr_init( &mutexattr );

	/* PTHREAD_MUTEX_NORMAL: it does not detect dead lock.
	   - If you want lock a locked mutex, you will get a deadlock mutex.
	   - If you want unlock an unlocked mutex, you will get an undefined behavior.

	   PTHREAD_MUTEX_ERRORCHECK: it returns an error if you you want lock a locked mutex or unlock an unlocked mutex.

	   PTHREAD_MUTEX_RECURSIVE:
	   - If you want lock a locked mutex, it stays at locked state an returns a success.
	   - If you want unlock an unlocked mutex, it stays at unlocked state an returns a success.

	   PTHREAD_MUTEX_DEFAULT: if you lock a locked mutex or unlock an unlocked mutex, you will get an undefined behavior.
	*/
	pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE );

	/* Initialize the mutex referenced by mutex with attributes specified by attr.
	   If the attribute is NULL, the default mutex attributes are used.
	   Upon initialization is successful, the mutex becomes INITIALIZED and UNLOCKED.

	   /!\ Initialization the same mutex twice will get an undefined behavior. /!\
	*/
	pthread_mutex_init( &g_criticalSection, &mutexattr );

	// You only can use the mutex ATTR one time, it does not affect any other mutexes and you should detroy it to uninitialize it.
	pthread_mutexattr_destroy( &mutexattr );


	pthread_mutex_lock( &initMutex );

	Options::Create(CONFIG_PATH, CACHE_PATH, "");
	Options::Get()->Lock();
	Manager::Create();
	Manager::Get()->AddWatcher(onNotification, NULL);
	Manager::Get()->AddDriver(DRIVER_PATH);

	thread t3(Five::statusObserver, Five::nodes);
	t3.detach();

	thread t4(Five::server, ZWAVE_PORT);
	t4.detach();

	if (g_checkLocked){
		thread t2(CheckFailedNode, FAILED_NODE_PATH);
		t2.detach();
		this_thread::sleep_for(chrono::seconds(30));
		g_checkLocked = false;
	}

	pthread_cond_wait(&initCond, &initMutex);
	pthread_mutex_unlock( &g_criticalSection );
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

	if (notification->GetType() == Notification::Type_ValueChanged) {
		string msg = buildNotifMsg(notification);

		// thread ttemp(sendMsg, "0.0.0.0", PHP_PORT, msg);
		// ttemp.join();
	}

	Node::NodeData nodeData;
	Node::NodeData* ptr_nodeData = &nodeData;

	pthread_mutex_lock(&g_criticalSection); // Lock the critical section
	
	if (Five::homeID == 0) {
		Five::homeID = notification->GetHomeId();
	}

	list<NodeInfo*>::iterator it;

	switch (notification->GetType()) {
		case Notification::Type_ValueAdded:
			notifType = "VALUE ADDED";
			log += "[VALUE_ADDED]	                  node " + to_string(notification->GetNodeId()) + ", value " + to_string(valueID.GetId()) + '\n';

			for (it = nodes->begin(); it != nodes->end(); it++) {
				if (notification->GetNodeId() == (*it)->m_nodeId) {
					(*it)->m_name = Manager::Get()->GetNodeProductName(homeID, notification->GetNodeId());
				}
			}

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
			notifType = "VALUE CHANGED";

			Manager::Get()->GetValueAsString(valueID, ptr_container);
			Manager::Get()->GetNodeStatistics(valueID.GetHomeId(), valueID.GetNodeId(), ptr_nodeData);

			log += "[VALUE_CHANGED]                   node "
			    + to_string(valueID.GetNodeId()) + ", "
				+ Manager::Get()->GetValueLabel(valueID) + ": "
				+ *ptr_container + '\n';

			valueID = notification->GetValueID();
			break;
		case Notification::Type_ValueRefreshed:
			notifType = "VALUE REFRESHED";
			Manager::Get()->GetValueAsString(valueID, ptr_container);
			log += "[VALUE_REFRESHED]                 node " + to_string(valueID.GetNodeId()) + ", "
				+ Manager::Get()->GetValueLabel(valueID) + ": " + *ptr_container + "\n";
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

	if (containsType(notification->GetType(), Five::AliveNotification) || notification->GetNodeId() == 1) {
		// if ((containsType(notification->GetType(), Five::AliveNotification) || (nodes->size() == 1 && notification->GetType() == Notification::Type_AllNodesQueried)) && g_menuLocked) {
		// 	thread t1(menu);
		// 	t1.detach();
		// 	g_menuLocked = false;
		// }

		if (containsNodeID(notification->GetNodeId(), (*Five::nodes))) {
			NodeInfo* n = getNode(notification->GetNodeId(), Five::nodes);
			if (n->m_isDead) {
				n->m_isDead = false;
				if (LEVEL != logLevel::NONE && n->m_nodeId != 1) {
					cout << "\n\n⭐ [NEW_NODE_APPEARS]             node " << to_string(valueID.GetNodeId()) << endl;
					cout << "   - total: " << aliveNodeSum(nodes) + deadNodeSum(nodes) << endl;
					cout << "   - alive: " << aliveNodeSum(nodes) << endl;
					cout << "   - dead : " << deadNodeSum(nodes) << "\n\n" << endl;
				}
			}
			n->m_sync = chrono::high_resolution_clock::now();
		}
	}

	if (notification->GetType() != Notification::Type_NodeRemoved) {
		myfile.open(path, ios::app);

		myfile << "[" << getDate(convertDateTime(getCurrentDatetime())) << ", "<< getTime(convertDateTime(getCurrentDatetime())) << "] "
			<< notifType << ", " << cc_name << " --> "
			<< to_string(valueID.GetIndex()) << "(" << valueLabel << ")\n";

		myfile.close();
	}

	pthread_mutex_unlock(&g_criticalSection); // Unlock the critical section.
}

void watchState(uint32 homeID, int loopTimeOut) {
	Driver::ControllerState referenceState{ Manager::Get()->GetDriverState(homeID) };
	Driver::ControllerState currentState{ Manager::Get()->GetDriverState(homeID) };

	while (loopTimeOut --> 0) {
		bool outOfRange{ referenceState <= 0 };

		if (!outOfRange)
			outOfRange = referenceState > STATES->size();

		if (!outOfRange)
			outOfRange = currentState <= 0;

		if (!outOfRange)
			outOfRange = currentState > STATES->size();

		cout << "State: " << Five::STATES[currentState] << "\n";

		switch (currentState) {
			case Driver::ControllerState::ControllerState_Completed:
				cout << "Final state: " << STATES[currentState] << "\n";
				return;
			// case Driver::ControllerState::ControllerState_Error:
			// 	cout << "Final state: " << STATES[currentState] << "\n";
			// 	return;
			default:
				if (referenceState != currentState) {
					referenceState = currentState;
				}

				break;
		}

		currentState = Manager::Get()->GetDriverState(homeID);

		this_thread::sleep_for(chrono::milliseconds(Five::STATE_PERIOD));
	}
	cout << "TimeOut\n";
}

void menu() {
	bool menuRun(1);

	while(menuRun){
		string response;
		bool isOk = false;
		list<string>::iterator sIt;
		int choice{ 0 };
		int lock(0);
		int stateInt(0);
		int counterNode{0};
		int counterValue{0};
		list<NodeInfo*>::iterator it;
		list<ValueID>::iterator it2;
		string container;
		string* ptr_container = &container;
		string fileName{ "" };

		// while (x --> 0) {
		// 	this_thread::sleep_for(chrono::seconds(1));
		// }

		cout << "\n>>──────|MENU|──────<<\n\n"
			 << "[-1] Dev\n"
			 << "[1] Add node\n"
		  	 << "[2] Remove node\n"
		 	 << "[3] Get value\n"
		 	 << "[4] Set value (old)\n"
		 	 << "[5] Reset Key\n"
		 	 << "[7] Heal\n"
		 	 << "[8] Set value (new)\n"
			 << "[9] Network\n"
			 << "[10] Is Node Failed\n"
			 << "\nChoose: ";

		cin >> response;

		try {
			choice = stoi(response);
		} catch(const std::exception& e) {
			std::cerr << e.what() << '\n';
		}

		int milliCounter{ 0 };

		switch (choice) {
			case 11:
			nodeChoice(&choice, it);

			Manager::Get()->RequestAllConfigParams(homeID, choice);
			break;
			case 10:
				cout << "Choose what node to check: " << endl;

				//Printing node names and receiving user's choice
				for(it =Five::nodes->begin(); it !=Five::nodes->end(); it++)
				{
					counterNode++;
					cout << counterNode << ". " << (*it)->m_name << endl;
				}

				cin >> response;
				choice = stoi(response);
				cout << Manager::Get()->IsNodeFailed(homeID, (unsigned int)choice) << endl;
				break;

			case -1:
				cout << "\n>>────|DEV|────<<\n\n";
				for (it = nodes->begin(); it != nodes->end(); it++) {
					cout << "[" << to_string((*it)->m_nodeId) << "] "
						 << (*it)->m_name << "\n";
				}

				cout << "\nSelect a node ('q' to exit): ";
				cin >> response;
				for (it = nodes->begin(); it != nodes->end(); it++) {
					if (response == to_string((*it)->m_nodeId)) {
						while (true) {
							cout << "Is awake: ";
							if (Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId)) {
								cout << "1\n";
								break;
							}
							cout << "elapsed: " << to_string(milliCounter) << "ms, 0\n";
							this_thread::sleep_for(chrono::milliseconds(100));
							milliCounter += 100;
						}
						break;
					}
				}
				break;
			case 9:
				cout << "\n>>───|NETWORK|───<<\n\n"
					 << "[1] Ping\n"
					 << "[2] Broadcast\n"
					 << "[3] Neighbors\n"
					 << "[4] Polls\n"
					 << "[5] Map\n"
					 << "\nChoose ('q' to exit): ";

				cin >> response;

				if (response == "1") {
					cout << "\n>>───|PING|───<<\n\n";

					for (it = nodes->begin(); it != nodes->end(); it++) {
						cout << "       [ " << to_string((*it)->m_nodeId) << " ]\n";
					}
					cout << "\nChoose ('q' to exit): ";
					cin >> response;

					for (it = nodes->begin(); it != nodes->end(); it++) {
						cout << response << " " << to_string((*it)->m_nodeId) << '\n';
						if (response == to_string((*it)->m_nodeId)) {
							for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++) {
								if (Manager::Get()->GetValueLabel(*it2) == "Library Version") {
									cout << "Ping sent...\n";
									int counter{ 60 };
									while (counter --> 0) {
										Manager::Get()->RefreshValue(*it2);

										this_thread::sleep_for(chrono::milliseconds(500));
										cout << "ask..." << Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId) << "\n";
									}
									cout << "Done\n";
								}
							}
						}
					}
				} else if (response == "5") {
					// cout << "Hello\n";
					// cout << nodes->size() <<  "Hello\n";
					// cout << lastNode << "Hello\n";
					// it = nodes->end();
					// uint8 lastNode = (*it)->m_nodeId;

					int nCounter(nodes->size());
					int matrix[nCounter][nCounter];

					for (int i = 0; i < nCounter; i++) {
						for (int j = 0; j < nCounter; j++) {
							matrix[i][j] = 0;
						}
					}

					// cout << "Setup matrix:\n";
					for (int i = 0; i < nCounter; i++) {
						for (int j = 0; j < nCounter; j++) {
							// cout << matrix[i][j] << " ";
						}
						// cout << "\n";
					}

					// int nodeIds[nCounter];

					for (int i = 0; i < nCounter; i++) {
						it = nodes->begin();
						advance(it, i);
						// nodeIds[i] = (*it)->m_nodeId;
					}

					// cout << "Length: " << to_string(nCounter) << "\n";

					for (int i = 0; i < nCounter; i++) { // Nodes loop.
						it = nodes->begin();
						advance(it, i);
						Manager::Get()->GetNodeNeighbors(homeID, (*it)->m_nodeId, bitmap);
						// cout << "Node " << to_string((*it)->m_nodeId) << "\n";

						if (!(*it)->m_isDead && (*it)->m_nodeId != 1) { // Check if the node is not the controller and can returns its neighbors.
							cout << "\n";
							for (int j = 0; j < 29; j++) { // Neigbors loop.
								cout << to_string((*bitmap)[j]) << " ";
							}
							cout << "\n";

							// cout << "\n";

							// cout << "Neigbors: ";
							for (int j = 0; j < 29; j++) { // Neigbors loop.
								for (int k = 0; k < (int)nodes->size(); k++) {
									auto jt = nodes->begin();
									advance(jt, k);
									if ((*jt)->m_nodeId == (*bitmap)[j]) {
										// cout << to_string((*jt)->m_nodeId) << " ";
										matrix[i][k] = 1;
										break;
									}
								}
								// for (auto jt = nodes->begin(); jt != nodes->end(); jt++) {
								// 	if ((*jt)->m_nodeId == (*bitmap)[j]) {
								// 		cout << to_string((*jt)->m_nodeId) << " ";
								// 		matrix[i][]
								// 		break;
								// 	}
								// }
							}
							cout << "\n";

							// cout << to_string((*it)->m_nodeId) << "\n";

							// for (int j = 0; j < 29; j++) { // Neigbors loop.
							// 	bool eof(true);

							// 	for (int k = 0; k < nCounter; k++) {
							// 		cout << "nodeId " << to_string(nodeIds[k]) << ": " << to_string((*bitmap)[j]) << "\n";
							// 		if ((*bitmap)[j] == nodeIds[k]) { // Check if the current value is a neigbor.
							// 			eof = false;
							// 			break;
							// 		}
							// 	}

							// 	if (eof) {
							// 		cout << "eof\n";
							// 		break;
							// 	} else {
							// 		// cout << (*it)->m_nodeId << ": " << (*bitmap)[j] << "\n";
							// 		matrix[i][j] = (*bitmap)[j];
							// 	}
							// }
						}
					}

					// for (int i = 0; i < nCounter; i++) {
					// 	for (int j = 0; j < nCounter; j++) {
					// 		cout << matrix[i][j] << " ";
					// 	}
					// 	cout << "\n";
					// }

					// for (it = nodes->begin(); it != nodes->end(); it++) {
					// 	if (!(*it)->m_isDead && (*it)->m_nodeId != 1) {
					// 		cout << Manager::Get()->GetNodeNeighbors(homeID, (*it)->m_nodeId, bitmap) << "\n";

					// 		for (int i = 0; i < 29; i++) {
					// 			// if ((*bitmap)[i] > lastNode || (*bitmap)[i] == 0) {
					// 			// 	cout << "Endf\n";
					// 			// }
					// 			// cout << to_string((*bitmap)[i]) << " ";

					// 			for (auto iter = nodes->begin(); iter != nodes->end(); iter++) {
					// 				if ((*iter)->m_nodeId == (*bitmap)[i]) {
					// 					matrix[(*iter)->m_nodeId][(*iter)->m_nodeId]++;
					// 					break;
					// 				}
					// 			}
					// 		}
					// 	}
					// }

					for (int i = 0; i < nCounter; i++) {
						for (int j = 0; j < nCounter; j++) {
							cout << matrix[i][j] << " ";
						}
						cout << "\n";
					}
				} else if (response == "2") {
					cout << "\n>>─────|BROADCAST|─────<<\n\n";

					for(it = nodes->begin(); it != nodes->end(); ++it){
						for(it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++){
							if (Manager::Get()->GetValueLabel(*it2) == "Library Version") {
								cout << "Ping sent...\n";
								int counter{ 60 };
								while (counter --> 0) {
									Manager::Get()->RefreshValue(*it2);

									this_thread::sleep_for(chrono::milliseconds(500));
									if(Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId)){
										cout << (*it)->m_name << ": OK" << endl;
										break;
									}else{
										cout << (*it)->m_name << ": not OK" << endl;
									}
								}
							}
						}
					}
				} else if (response == "3") {
					cout << "\n>>─────|NEIGHBORS|─────<<\n\n";

					for (it = nodes->begin(); it != nodes->end(); it++) {
						if ((*it)->m_nodeId != 1 && !(*it)->m_isDead) { // The driver doesn't have neighbors property
							cout << "[" << to_string((*it)->m_nodeId) << "] " << (*it)->m_name << "\n";
						}
					}
					cout << "\nSelect a node ('q' to exit): ";
					cin >> response;
					cout << "\nNeighbor chain: ";
					for (it = nodes->begin(); it != nodes->end(); it++) {
						if (to_string((*it)->m_nodeId) == response) {
							Manager::Get()->GetNodeNeighbors(homeID, (*it)->m_nodeId, bitmap);
							for (int i = 0; i < 29; i++) {
								// for (j = 0; j < 8; j++) {
								// 	cout << (bitset<8>((*bitmap)[i]))[i] << "  ";
								// }
								cout << to_string((*bitmap)[i]);
							}
							cout << "\n\n"
								<< "[1] Synchronize neighbors\n"
								<< "[2] Request neighbor update\n "
								<< "[3] Heal network\n"
								<< "\nChoose ('q' to exit): ";
							cin >> response;

							if (response == "1") {
								Manager::Get()->SyncronizeNodeNeighbors(homeID, (*it)->m_nodeId);
								cout << "Done.\n";
							} else if (response == "2") {
								thread ttemp(watchState, homeID, LOOP_TIMEOUT);
								ttemp.detach();

								Manager::Get()->RequestNodeNeighborUpdate(homeID, (*it)->m_nodeId)
									? cout << "Update request sent\n"
									: cout << "Update request failed\n";
							} else if (response == "3") {
								Manager::Get()->HealNetwork(homeID, true);
							}
							break;
						}
					}


				} else if (response == "4") {
					cout << "\n>>─────|POLLS|─────<<\n\n"
						 << "/!\\ If you set the poll intensity, you must restart the ZWave key to add modification.\n\n"
						 << "[1] Set intensity\n"
						 << "[2] Set interval for all devices (PLEASE BE CAREFUL)\n\n"
						 << "Choose ('q' to exit): ";

					cin >> response;
					cout << "\n";

					if (response == "1") {
						for (it = nodes->begin(); it != nodes->end(); it++) {
							cout << "[" << to_string((*it)->m_nodeId) << "] " << (*it)->m_name << "\n";
						}
						cout << "\nSelect a node ('q' to exit): ";
						cin >> response;
						cout << '\n';

						for (it = nodes->begin(); it != nodes->end(); it++) {
							if (response == to_string((*it)->m_nodeId)) {
								for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++) {
									cout << "[" << to_string(counterValue++) << "] " << Manager::Get()->GetValueLabel(*it2)
									     << ", " << to_string(Manager::Get()->GetPollIntensity(*it2)) << "\n";
								}

								cout << "\nSelect a valueID (press 'q' to exit): ";
								cin >> response;
								counterValue = 0;

								for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++) {
									if (response == to_string(counterValue++)) {
										cout << "\nValueID \"" << Manager::Get()->GetValueLabel(*it2)
											<< "\", current: " << to_string(Manager::Get()->GetPollIntensity(*it2))
											<< " poll(s)/interval\n";

										cout << "\nSet intensity (0=none, 1=every time through the list, 2-every other time, 'q' to exit): ";
										cin >> response;

										if (response != "q") {
											Manager::Get()->SetPollIntensity(*it2, stoi(response));
											cout << "Done\n";
										}
										break;
									}
								}
							}
						}
					} else if (response == "2") {
						cout << "/!\\ [PLEASE READ] Set the time period between polls of a node's state. Due to patent concerns, some devices do not report state changes automatically to the controller. These devices need to have their state polled at regular intervals. The length of the interval is the same for all devices. To even out the Z-Wave network traffic generated by polling, OpenZWave divides the polling interval by the number of devices that have polling enabled, and polls each in turn. It is recommended that if possible, the interval should not be set shorter than the number of polled devices in seconds (so that the network does not have to cope with more than one poll per second).\n\nCurrent poll interval: " << Manager::Get()->GetPollInterval() << "ms\n\nSet interval in milliseconds ('q' to exit): ";
						cin >> response;

						if (response != "q") {
							Manager::Get()->SetPollInterval(stoi(response), false);
							cout << "Done.\n";
						}
					}
				}
				break;
		case 1:
		{
			//Putting the driver into a listening state
			Manager::Get()->AddNode(Five::homeID, false);

			//Printing progression messages
			thread t3(nodeSwitch, stateInt, &lock);
			t3.detach();
			stateInt = Manager::Get()->GetDriverState(Five::homeID);

			break;
		}
		case 2:
		{
			//Putting the driver into a listening state
			Manager::Get()->RemoveNode(Five::homeID);

			//Printing progression messages
			// thread t3(statusObserver, nodes);
			// t3.join();

			// thread t3(nodeSwitch, stateInt, &lock);
			// t3.detach();
			// stateInt = Manager::Get()->GetDriverState(Five::homeID);

			break;
		}
		case 3:
			//Printing node names and receiving user's choice
			nodeChoice(&choice, it);

			if(choice == -1){ //Impossible value except if user chooses to quit
				break;
			}

			//Printing all the node's values with name, id and ReadOnly state
			printValues(&choice, &it, it2, true);
			break;
		case 4:
			//Printing node names and receiving user's choice

			nodeChoice(&choice, it);

			if(choice == -1){ //Impossible value except if user chooses to quit
				break;
			}
			//counterNode = 0;

			//Printing value names and receiving user's choice
			for(it =Five::nodes->begin(); it !=Five::nodes->end(); it++)
			{
				counterNode++;
				if ((*it)->m_nodeId == choice)
				{
					for(it2 = (*it) -> m_values.begin(); it2 != (*it) -> m_values.end(); it2++)
					{
						if(!Manager::Get()->IsValueReadOnly(*it2)){
							counterValue++;
							cout << counterValue << ". " << Manager::Get()->GetValueLabel(*it2) << endl;
						}

					}

					cout << "\nChoose a valueID: ";
					cin >> response;
					counterValue = 0;
					choice = stoi(response);

					for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++) {
						// Manager::Get()->GetValueAsString(*valueIt, ptr_container);
						// cout << Manager::Get()->GetValueLabel(*valueIt) << ": " << *ptr_container << endl;
						if(!Manager::Get()->IsValueReadOnly(*it2)){
							counterValue++;
						}

						if (choice == counterValue) {

							//Printing the current value
							cout << Manager::Get()->GetValueLabel(*it2) << it2->GetAsString() << endl;
							Manager::Get()->GetValueAsString((*it2), ptr_container);
							cout << "Current value: " << *ptr_container << endl;

							//Asking the user to set the wanted value
							cout << "Set to what ? ";
							cin >> response;
							int tempCounter{ 100 };
							// int test = 0;
							// int* testptr = &test;
							//setUnit((*valueIt));

							//Sending the value until current value is identical, or until timeout (for sleeping nodes)
							while (response != *ptr_container && tempCounter-->0) {
								// if (response.find('.')) {
								// 	float temp = stof(response);
								// 	Manager::Get()->SetValue(*it2, temp);
								// } else {
								// }

								Manager::Get()->SetValue((*it2), response);
								Manager::Get()->GetValueAsString((*it2), ptr_container);
								this_thread::sleep_for(chrono::milliseconds(100));
								cout << "send " << response << ", " << *ptr_container << "\n";
							}

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
			break;
		case 7:
			//Printing node names and receiving user's choice
			nodeChoice(&choice, it);

			//Ask the node to recheck his neighboors
			for (it =Five::nodes->begin(); it !=Five::nodes->end(); it++){
				if ((*it)->m_nodeId == choice){
					Manager::Get()->HealNetworkNode((*it)->m_homeId, (*it)->m_nodeId, true);
				}
			}
			break;
		case 8:
			//Printing node names and receiving user's choice
			nodeChoice(&choice, it);
			//counterNode = 0;
			if(choice == -1){ //Impossible value except if user chooses to quit
				break;
			}

			//Printing value names and receiving user's choice
			printValues(&choice, &it, it2, false);

			if(choice == -1){ //Impossible value except if user chooses to quit
				break;
			}

			//Calling appropriate method depending on user's choice
			newSetValue(&choice, &it, it2, isOk);
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

//Function printing progression messages during Add Node and Remove Node
void nodeSwitch(int stateInt, int *lock){
	int counter(500);
	while (counter --> 0) {
		switch (stateInt){
			case 1:
				if(*lock != stateInt){
				cout << "STARTING" << endl;
				}
				*lock = 1;
				break;
			case 4:
				if(*lock != stateInt){
				cout << "WAITING" << endl;
				}
				*lock = 4;
				break;
			case 7:
				if(*lock != stateInt){
				cout << "COMPLETED" << endl;
				}
				*lock = 7;
				break;
			default:
				break;
		}
		this_thread::sleep_for(chrono::milliseconds(20));
	}
	cout << "Done" << endl;
}

//Function looping in the background to check if nodes are failed
void CheckFailedNode(string path){
	while(true){
		list<NodeInfo*>::iterator it;
		fstream file;
		bool isIn(0);
		this_thread::sleep_for(chrono::seconds(FAILED_NODE_INTERVAL));

		for(it = n.begin(); it != n.end(); it++){
			// cout << "in node for" << endl;
			uint8 nodeId = (*it)->m_nodeId;
			string line;
			string nodeName = (*it)->m_name;
			string nodeType = (*it)->m_nodeType;
			if(Manager::Get()->IsNodeFailed(homeID, nodeId)){
				file.open(path, ios::in);
				while(getline(file, line)){
					if(line.find("Label: " + nodeName) != string::npos){
						isIn = 1;
					}
				}
				file.close();
				// cout << "NODE HAS FAILED: " << nodeName << " id: " << unsigned(nodeId) << endl;
				if(!isIn){
					file.open(path, ios::app);
					file << "Label: " << nodeName << " Id: " << unsigned(nodeId) << " Type: " << nodeType << endl;
					file.close();
				}
			}else if(!(Manager::Get()->IsNodeFailed(homeID, nodeId))){
				// cout << "node not failed" << "id: " << unsigned(nodeId) << endl;
				file.open(path, ios::in);
				fstream temp;
				temp.open("temp.txt", ios::app);
				while(getline(file, line)){
					cout << line << endl;
					string s = "Label: " + nodeName;
					// cout << s << endl;
					if(line.find(s) == string::npos){
						temp << line;
					}
				}
				temp.close();
				file.close();

				const char *p = path.c_str();
				remove(p);
				rename("temp.txt", p);
			}
		}

	}
}