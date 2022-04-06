#include "Five.h"
#include "Notification.h"
#include "Manager.h"
#include "command_classes/CommandClass.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <math.h>

using namespace OpenZWave;
using namespace Five;

NodeInfo* Five::GetNode(uint8 nodeID, list<NodeInfo*> *nodes) {
    list<NodeInfo*>::iterator it;
    for (it = nodes->begin(); it != nodes->end(); it++) {
        if ((*it)->m_nodeId == nodeID) {
            return (*it);
        }
    }
    return NULL;
}

NodeInfo Five::GetNodeConfig(uint32 homeId, uint8 nodeId, list<NodeInfo*> *nodes)
{
    list<NodeInfo*>::iterator it;
    NodeInfo node;
    
    node.m_homeId = homeId;
    node.m_name = Manager::Get()->GetNodeProductName(node.m_homeId, nodeId);
    node.m_nodeId = nodeId;
    node.m_nodeType = Manager::Get()->GetNodeType(node.m_homeId, nodeId);
    for(it = nodes->begin(); it != nodes->end(); ++it)
    {
        if((*it)->m_nodeId == nodeId)
        {
            node.m_values = (*it)->m_values;
        }
    }
    return node;
    
}
bool Five::ValueAdded(Notification const* notification, list<NodeInfo*> *nodes) {
    return true;
}

bool Five::ValueRefreshed(Notification const* notification, list<NodeInfo*> *nodes) {
    return true;
}

bool Five::ValueRemoved(Notification const* notification, list<NodeInfo*> *nodes) {
    return true;
}

/// A value has changed on the Z-Wave network and this is a different value.
bool Five::ValueChanged(Notification const* notification, list<NodeInfo*> *nodes) {
    bool state;
    bool* ptr = &state;
    
    ValueID v{ notification->GetValueID() };
    string output{ "[VALUE CHANGED]\n" };
    list<NodeInfo*>::iterator it;

    for (it = nodes->begin(); it != nodes->end(); it++) {
        if ((*it)->m_nodeId == v.GetNodeId()) {
            (*it)->m_values.push_back(v);
        }
    }

    output += "id           : " + int(v.GetId()) + '\n';
    output += "index        : " + int(v.GetIndex()) + '\n';
    output += "type         : " + v.GetTypeAsString() + '\n';
    output += "node_id      : " + int(v.GetNodeId()) + '\n';
    output += "node_name    : " + Manager::Get()->GetNodeProductName(v.GetHomeId(), v.GetNodeId()) + '\n';
    output += "node_type    : " + int(Manager::Get()->GetNodeDeviceType(v.GetHomeId(), v.GetNodeId())) + '\n';
    output += "node_stage   : " + Manager::Get()->GetNodeQueryStage(v.GetHomeId(), v.GetNodeId()) + '\n';
    output += "node_version : " + int(Manager::Get()->GetNodeVersion(v.GetHomeId(), v.GetNodeId())) + '\n';
    output += "node_specific: " + Manager::Get()->GetNodeSpecificString(v.GetHomeId(), v.GetNodeId()) + '\n';
    output += "node_label   : " + Node(v.GetHomeId(), v.GetNodeId()).GetInstanceLabel(v.GetCommandClassId(), v.GetInstance()) + '\n';
    output += "cc_id        : " + int(v.GetCommandClassId()) + '\n';
    output += "name         : " + Manager::Get()->GetCommandClassName(v.GetCommandClassId()) + '\n';

    if (v.GetType() == ValueID::ValueType_Bool) {
        Manager::Get()->GetValueAsBool(v, ptr);
        output += "value_as_bool: " + *ptr + '\n';
    }

    // return output;
    return true;
}

bool Five::setSwitch(ValueID valueId)
{   
    string answer;
    bool state(0);
    cout << "true(1) or false(0) ?" << endl;
	cin >> answer;

    if(answer=="true" || answer=="True" || answer == "1"){
        state = true;
    }else if(answer == "false" || answer == "False" || answer == "0"){
        state = false;
    }

    Manager::Get()->SetValue(valueId, state);
    return true;
}



bool Five::SetIntensity(ValueID valueId, IntensityScale intensity) {
    Manager::Get()->SetValue(valueId, to_string(intensity));
    return true;
}

bool Five::SetColor(ValueID valueId)
{
    cout << "Enter hexadecimal color" << endl;
    string hexColor;
    cin >> hexColor;
    Manager::Get()->SetValue(valueId, hexColor);
    return true;
}

bool Five::SetList(ValueID valueId){
    vector<string> vectS;
    vector<string> *vectSPtr = &vectS;
    int counter{0};
    string s;
    string* ptr = &s;
    cout << valueId.GetTypeAsString() << endl;
    Manager::Get()->GetValueListItems(valueId, vectSPtr) ;
    vector<string>::iterator it;
    int choice;
    string response;
    Manager::Get()->GetValueListSelection(valueId, ptr);
    cout << "Current Value: " << s << endl;
    for(it = vectS.begin(); it != vectS.end(); ++it){
        counter++;
        cout << counter << ". " << (*it) << endl;
    }
    cout << "Choose your poison" << endl;
    cin >> response;
    choice = stoi(response);
    cout << choice << endl;
    counter = 0;
    for(it = vectS.begin(); it != vectS.end(); ++it){
        counter++;
        if(choice == counter){
            cout << (*it) << endl;
            Manager::Get()->SetValueListSelection(valueId, (*it));
        }
    }
    //Manager::Get()->SetValue(valueId, UnitName);
    return true;
}

bool Five::SetVolume(ValueID valueId, IntensityScale intensity){
    Manager::Get()->SetValue(valueId, to_string(intensity));
    return true;
}

bool Five::SetDuration(ValueID valueId) {
    string response;
    cout << "Please enter a duration in seconds:" << endl;
    cin >> response;
    Manager::Get()->SetValue(valueId, response);
    return true;
}

bool Five::setInt(ValueID valueId){
    string response;
    cout << "Please enter a value in Int:" << endl;
    cin >> response;
    Manager::Get()->SetValue(valueId, response);
    return true;
}

bool Five::setBool(ValueID valueId)
{   
    string answer;
    bool state(0);
    cout << "true(1) or false(0) ?" << endl;
	cin >> answer;
	if(answer == "true" || answer == "True" || answer == "1"){
        state = true;
    }else if(answer == "false" || answer == "False" || answer == "0"){
        state = false;
    }

    Manager::Get()->SetValue(valueId, state);
    return true;
}

bool Five::setButton(ValueID valueId){
    string input;
    cout << "Press a key to push button" << endl;
    cin >> input;
    Manager::Get()->PressButton(valueId);
    cout << "Press a key to release button" << endl;
    cin >> input;
    Manager::Get()->ReleaseButton(valueId);
    return true;
}

// Create a new NodeInfo.
NodeInfo* Five::CreateNode(Notification const* notification) {
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
bool Five::IsNodeNew(uint8 nodeID, list<NodeInfo*> *nodes) {
	list<NodeInfo*>::iterator it;
	for (it = nodes->begin(); it != nodes->end(); it++) {
		if ((*it)->m_nodeId == nodeID) {
			return false;
		}
	}
	return true;
}

int Five::DeadNodeSum(list<NodeInfo*> *nodes) {
	int counter{ 0 };
	list<NodeInfo*>::iterator it;
	for (it = nodes->begin(); it != nodes->end(); it++) {
		if ((*it)->m_isDead && (*it)->m_nodeId != 1) {
			counter++;
		}
	}
	return counter;
}

int Five::AliveNodeSum(list<NodeInfo*> *nodes) {
	return (nodes->size() - 1) - DeadNodeSum(nodes);
}

bool Five::ContainsType(Notification::NotificationType needle, vector<Notification::NotificationType> haystack) {
    for (int i = 0; i < int(haystack.capacity()); i++) {
        if (needle == haystack[i]) {
            return true;
        }
    }
    return false;
}

bool Five::ContainsNodeID(uint8 needle, list<NodeInfo*> haystack) {
    list<NodeInfo*>::iterator it;
    for (it = haystack.begin(); it != haystack.end(); it++) {
        if ((*it)->m_nodeId == needle) {
            return true;
        }
    }
    return false;
}

bool Five::IsNodeAlive(Notification notif, list<NodeInfo*> *nodes, vector<Notification::NotificationType> aliveNotifications) {
    // uint8 nodeID{ valueID.GetNodeId() };
    // bool containsType{ Five::ContainsType(notif.GetType(), aliveNotifications) };
    return true;
}

// Refresh members in the oldNodeInfo thanks to this valueID.
void Five::RefreshNode(ValueID valueID, NodeInfo* oldNodeInfo) {
	oldNodeInfo->m_homeId = valueID.GetHomeId();
	oldNodeInfo->m_name = Manager::Get()->GetNodeName(valueID.GetHomeId(), valueID.GetNodeId());
	oldNodeInfo->m_nodeType = Manager::Get()->GetNodeType(valueID.GetHomeId(), valueID.GetNodeId());
	oldNodeInfo->m_values.push_back(valueID);
}

void Five::PushNode(Notification const *notification, list<NodeInfo*> *nodes) {
	NodeInfo* n{ CreateNode(notification) };
	uint8 nodeID{ notification->GetNodeId() };

	if (IsNodeNew(nodeID, nodes)) {
		nodes->push_back(n);
	}
}

void Five::RemoveNode(Notification const *notification, list<NodeInfo*> *nodes) {
    uint8 nodeID{ notification->GetNodeId() };
	list<NodeInfo*>::iterator it;
    NodeInfo* n;

	for (it = nodes->begin(); it != nodes->end(); it++) {
		if ((*it)->m_nodeId == nodeID) {
            n = (*it);
			nodes->remove(n);
            return;
		}
	}
}

bool Five::AddValue(ValueID valueID, NodeInfo *node) {
	// uint8 nodeID{ valueID.GetNodeId() };
	bool isNew{ true };
    
    list<ValueID>::iterator it;
    for (it = node->m_values.begin(); it != node->m_values.end(); it++) {
        if ((*it).GetId() == valueID.GetId()) {
            isNew = false;
        }
    }

    if (isNew) {
        // cout << node->m_nodeId << endl;
        node->m_values.push_back(valueID);
    }

    return isNew;


	// for (it = nodes->begin(); it != nodes->end(); it++) {
	// 	if ((*it)->m_nodeId == nodeID) {
	// 		(*it)->m_name = Manager::Get()->GetNodeProductName(valueID.GetHomeId(), nodeID);
	// 		(*it)->m_values.push_back(valueID);
	// 		return true;
	// 	}
	// }
	// return false;
}

bool Five::RemoveValue(ValueID valueID) {
	uint8 nodeID{ valueID.GetNodeId() };
	list<NodeInfo*>::iterator it;
	list<ValueID>* valueIDs;
	list<ValueID>::iterator it2;

	for (it = nodes->begin(); it != nodes->end(); it++) {
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

// If the path is right, remove the target file.
bool Five::RemoveFile(string path) {
	char arr[path.length()];
    strcpy(arr, path.c_str());
    remove(arr);
	return true;
}

string Five::GetDriverData(uint32 homeID) {
	Driver::DriverData driverData;
	string output = "";

	Manager::Get()->GetDriverStatistics(homeID, &driverData);
	output += "   - ACK counter: ";
	output += to_string(driverData.m_ACKCnt);
	output += "   - ACK waiting: ";
	output += to_string(driverData.m_ACKWaiting);
	return output;
}

chrono::high_resolution_clock::time_point Five::GetCurrentDatetime() {
    return std::chrono::high_resolution_clock::now();
}

tm* Five::ConvertDateTime(chrono::high_resolution_clock::time_point datetime) {
    time_t convertTime{ std::chrono::high_resolution_clock::to_time_t(datetime) };
    return localtime(&convertTime);
}

string Five::GetTime(tm* datetime) {
    return to_string(datetime->tm_hour) + ":" + to_string(datetime->tm_min) + ":" + to_string(datetime->tm_sec);
}

string Five::GetDate(tm* datetime) {
    return to_string(datetime->tm_mday) + "/" + to_string(datetime->tm_mon);
}

double Five::Difference(chrono::high_resolution_clock::time_point datetime01, chrono::high_resolution_clock::time_point datetime02) {
    chrono::duration<double> elapsed_seconds = datetime01 - datetime02;
	double rounded_elapsed = ceil(elapsed_seconds.count() * 1000) / 1000;
    return rounded_elapsed;
}

// Convert a string into an array of char.
// The output parameter must have the same length as the string.
// <output> will be filled.
void Five::Stoc(string chain, char* output) {
    for (int i = 0; i < int(chain.length()); i++) {
        output[i] = chain[i];
    }
}