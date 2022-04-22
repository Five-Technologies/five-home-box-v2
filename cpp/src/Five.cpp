#include "Five.h"
#include "Notification.h"
#include "Manager.h"
#include "command_classes/CommandClass.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <math.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <thread>
#include <fstream>

#define PORT 5101

using namespace OpenZWave;
using namespace Five;
using namespace std;

NodeInfo* Five::getNode(uint8 nodeID, list<NodeInfo*> *nodes) {
    list<NodeInfo*>::iterator it;
    for (it = nodes->begin(); it != nodes->end(); it++) {
        if ((*it)->m_nodeId == nodeID) {
            return (*it);
        }
    }
    return NULL;
}

NodeInfo Five::getNodeConfig(uint32 homeId, uint8 nodeId, list<NodeInfo*> *nodes)
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
bool Five::valueAdded(Notification const* notification, list<NodeInfo*> *nodes) {
    return true;
}

bool Five::valueRefreshed(Notification const* notification, list<NodeInfo*> *nodes) {
    return true;
}

bool Five::valueRemoved(Notification const* notification, list<NodeInfo*> *nodes) {
    return true;
}

/// A value has changed on the Z-Wave network and this is a different value.
bool Five::valueChanged(Notification const* notification, list<NodeInfo*> *nodes) {
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

bool Five::setSwitch(ValueID valueId, bool state) {   
    string answer;
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



bool Five::setIntensity(ValueID valueId, IntensityScale intensity) {
    Manager::Get()->SetValue(valueId, to_string(intensity));
    return true;
}

bool Five::setColor(ValueID valueId)
{
    cout << "Enter hexadecimal color" << endl;
    string hexColor;
    cin >> hexColor;
    Manager::Get()->SetValue(valueId, hexColor);
    return true;
}

bool Five::setList(ValueID valueId){
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

bool Five::setVolume(ValueID valueId, IntensityScale intensity){
    Manager::Get()->SetValue(valueId, to_string(intensity));
    return true;
}

bool Five::setDuration(ValueID valueId) {
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
    Manager::Get()->SetValue(valueId, stoi(response));
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
NodeInfo* Five::createNode(Notification const* notification) {
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
bool Five::isNodeNew(uint8 nodeID, list<NodeInfo*> *nodes) {
	list<NodeInfo*>::iterator it;
	for (it = nodes->begin(); it != nodes->end(); it++) {
		if ((*it)->m_nodeId == nodeID) {
			return false;
		}
	}
	return true;
}

int Five::deadNodeSum(list<NodeInfo*> *nodes) {
	int counter{ 0 };
	list<NodeInfo*>::iterator it;
	for (it = nodes->begin(); it != nodes->end(); it++) {
		if ((*it)->m_isDead && (*it)->m_nodeId != 1) {
			counter++;
		}
	}
	return counter;
}

int Five::aliveNodeSum(list<NodeInfo*> *nodes) {
	return (nodes->size() - 1) - deadNodeSum(nodes);
}

bool Five::containsType(Notification::NotificationType needle, vector<Notification::NotificationType> haystack) {
    for (int i = 0; i < int(haystack.capacity()); i++) {
        if (needle == haystack[i]) {
            return true;
        }
    }
    return false;
}

bool Five::containsNodeID(uint8 needle, list<NodeInfo*> haystack) {
    list<NodeInfo*>::iterator it;
    for (it = haystack.begin(); it != haystack.end(); it++) {
        if ((*it)->m_nodeId == needle) {
            return true;
        }
    }
    return false;
}

bool Five::isNodeAlive(Notification notif, list<NodeInfo*> *nodes, vector<Notification::NotificationType> aliveNotifications) {
    // uint8 nodeID{ valueID.GetNodeId() };
    // bool containsType{ Five::ContainsType(notif.GetType(), aliveNotifications) };
    return true;
}

// Refresh members in the oldNodeInfo thanks to this valueID.
void Five::refreshNode(ValueID valueID, NodeInfo* oldNodeInfo) {
	oldNodeInfo->m_homeId = valueID.GetHomeId();
	oldNodeInfo->m_name = Manager::Get()->GetNodeName(valueID.GetHomeId(), valueID.GetNodeId());
	oldNodeInfo->m_nodeType = Manager::Get()->GetNodeType(valueID.GetHomeId(), valueID.GetNodeId());
	oldNodeInfo->m_values.push_back(valueID);
}

void Five::pushNode(Notification const *notification, list<NodeInfo*> *nodes) {
	NodeInfo* n{ createNode(notification) };
	uint8 nodeID{ notification->GetNodeId() };

	if (isNodeNew(nodeID, nodes)) {
		nodes->push_back(n);
	}
}

void Five::removeNode(Notification const *notification, list<NodeInfo*> *nodes) {
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

bool Five::addValue(ValueID valueID, NodeInfo *node) {
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

bool Five::removeValue(ValueID valueID) {
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
bool Five::removeFile(string path) {
	char arr[path.length()];
    strcpy(arr, path.c_str());
    remove(arr);
	return true;
}

string Five::getDriverData(uint32 homeID) {
	Driver::DriverData driverData;
	string output = "";

	Manager::Get()->GetDriverStatistics(homeID, &driverData);
	output += "   - ACK counter: ";
	output += to_string(driverData.m_ACKCnt);
	output += "   - ACK waiting: ";
	output += to_string(driverData.m_ACKWaiting);
	return output;
}

chrono::high_resolution_clock::time_point Five::getCurrentDatetime() {
    return std::chrono::high_resolution_clock::now();
}

tm* Five::convertDateTime(chrono::high_resolution_clock::time_point datetime) {
    time_t convertTime{ std::chrono::high_resolution_clock::to_time_t(datetime) };
    return localtime(&convertTime);
}

string Five::getTime(tm* datetime) {
    return to_string(datetime->tm_hour) + ":" + to_string(datetime->tm_min) + ":" + to_string(datetime->tm_sec);
}

string Five::getDate(tm* datetime) {
    return to_string(datetime->tm_year) + "-" + to_string(datetime->tm_mon) + "-" + to_string(datetime->tm_mday);
}

double Five::difference(chrono::high_resolution_clock::time_point datetime01, chrono::high_resolution_clock::time_point datetime02) {
    chrono::duration<double> elapsed_seconds = datetime01 - datetime02;
	double rounded_elapsed = ceil(elapsed_seconds.count() * 1000) / 1000;
    return rounded_elapsed;
}

// Convert a string into an array of char.
// The output parameter must have the same length as the string.
// <output> will be filled.
void Five::stoc(string chain, char* output) {
    for (int i = 0; i < int(chain.length()); i++) {
        output[i] = chain[i];
    }
}

bool Five::nodeChoice(int* choice, list<NodeInfo*>::iterator it){
    string response;
    cout << "\n";
        for(it =Five::nodes->begin(); it !=Five::nodes->end(); it++) {
            cout << "[" << to_string((*it)->m_nodeId) << "] ";
            (*it)->m_isDead ? cout << "❌ " : cout << "✅ ";
            cout << getTime(convertDateTime((*it)->m_sync)) << ", "
                 << (*it)->m_name << "\n";
        }

        cout << "\nChoose ('q' to exit): ";
        cin >> response;
        if(response == "q"){
            *choice = -1 ;
            return true;
        }
        *choice = stoi(response);
        return true;
}

bool Five::printValues(int* choice, list<NodeInfo*>::iterator* it, list<ValueID>::iterator it2, bool getOnly){
    string container;
    string* ptr_container = &container;
    list<string>::const_iterator sIt;
    string response;
    int counterValue(0);
    for(*it = nodes->begin(); *it != nodes->end(); (*it)++){
        if (*choice == ((**it)->m_nodeId)) {
            if(getOnly) {
                cout << "\n>>────|VALUES OF THE NODE " << to_string((**it)->m_nodeId) << "|────<<\n\n"
                     << "[<nodeId>] <label>, <value>, <readOnly>\n\n";
                for(it2 = (**it)->m_values.begin(); it2 != (**it)->m_values.end(); it2++) {
                    Manager::Get()->GetValueAsString((*it2), ptr_container);
                    cout << "[" << counterValue++ << "] "
                         << Manager::Get()->GetValueLabel(*it2) << ", "
                         << *ptr_container << ", " 
                         << Manager::Get()->IsValueReadOnly(*it2)
                         << "                          " << to_string(it2->GetId()) << endl;
                } 
                return true;
            } else {
                for (it2 = (**it)->m_values.begin(); it2 != (**it)->m_values.end(); it2++) {
                    if ((ValueID::ValueType_List == (*it2).GetType() || ValueID::ValueType_Button == (*it2).GetType()) && !Manager::Get()->IsValueReadOnly(*it2)) {
                        cout << "[" << ++counterValue << "] " << Manager::Get()->GetValueLabel((*it2)) << endl;
                    } else for(sIt = TYPES.begin(); sIt != TYPES.end(); ++sIt) {
                        if ((Manager::Get()->GetValueLabel((*it2)).find((*sIt)) != string::npos) && !Manager::Get()->IsValueReadOnly(*it2)) {
                            cout << "[" << ++counterValue << "] " << Manager::Get()->GetValueLabel((*it2)) << endl;
                        }
                    }
                }
                cout << "\nSelect a value ('q' to exit): ";
                cin >> response;
                if (response == "q") {
                    *choice = -1;
                    return true;
                }
                *choice = stoi(response);
                return true;
            }
            break;
        }
    
    }
    return true;
}

bool Five::newSetValue(int* choice, list<NodeInfo*>::iterator* it, list<ValueID>::iterator it2, bool isOk){
    list<string>::const_iterator sIt;
    string container;
    string* ptr_container = &container;
    string response;
    int listchoice(0);
    int counterValue(0);

    cout << "nexset" << endl;

    for (it2 = (**it)->m_values.begin(); it2 != (**it)->m_values.end(); it2++)
    {
        cout << "test" << endl;
        if (ValueID::ValueType_Button == (*it2).GetType() && !Manager::Get()->IsValueReadOnly(*it2))
        {
            counterValue++;
            if (*choice == counterValue) {
                setButton((*it2));
                return true;
            }
            
        }
        if (ValueID::ValueType_List == (*it2).GetType() && !Manager::Get()->IsValueReadOnly(*it2))
        {
            counterValue++;
            if (*choice == counterValue)
            {
                Five::setList((*it2));
                return true;
            }
            
        }else for (sIt = TYPES.begin(); sIt != TYPES.end(); ++sIt)
        {
            if(Manager::Get()->GetValueLabel((*it2)).find((*sIt)) != string::npos && !Manager::Get()->IsValueReadOnly(*it2))
            {
                counterValue++;
                if (*choice == counterValue)
                {
                    string valLabel = Manager::Get()->GetValueLabel(*it2);
                    cout << "You chose " << valLabel << endl;
                    Manager::Get()->GetValueAsString((*it2), ptr_container);
                    cout << "Current value: " << *ptr_container << endl;
                    // cout << "Set to what ? ";
                    //cin >> response;

                    //Checking value type to choose the right method
                    if(valLabel.find("Switch") != string::npos){
                        setSwitch((*it2), true);
                        return true;
                    }else if(valLabel.find("Color") != string::npos && (*it2).GetType() == ValueID::ValueType_String)
                    {
                        setColor(*it2);
                        return true;
                    } else if(valLabel.find("Level") != string::npos && (*it2).GetType() == ValueID::ValueType_Byte)
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
                            return true;
                        }
                        
                        listchoice = stoi(response);
                        switch(listchoice){
                            case 1:
                                setIntensity((*it2), IntensityScale::VERY_HIGH);
                                break;
                            case 2:
                                setIntensity((*it2), IntensityScale::HIGH);
                                break;
                            case 3:
                                setIntensity((*it2), IntensityScale::MEDIUM);
                                break;
                            case 4:
                                setIntensity((*it2), IntensityScale::LOW);
                                break;
                            case 5:
                                setIntensity((*it2), IntensityScale::VERY_LOW);
                                break;
                        }
                        return true;
                        
                    }else if(valLabel.find("Volume") != string::npos)
                    {
                        cout << "Choose a value between:" << endl << "1. Very High\n" << "2. High\n" << "3. Medium\n" << "4. Low\n" << "5. Very Low\n"; 
                        cin >> response;
                        listchoice = stoi(response);
                        switch(listchoice){
                            case 1:
                                setIntensity((*it2), IntensityScale::VERY_HIGH);
                                break;
                            case 2:
                                setIntensity((*it2), IntensityScale::HIGH);
                                break;
                            case 3:
                                setIntensity((*it2), IntensityScale::MEDIUM);
                                break;
                            case 4:
                                setIntensity((*it2), IntensityScale::LOW);
                                break;
                            case 5:
                                setIntensity((*it2), IntensityScale::VERY_LOW);
                                break;
                            default:
                                break;
                        }	
                        return true;
                    }else if(valLabel.find("Duration") != string::npos)
                    {
                        setDuration((*it2));
                        return true;
                    }else if((*it2).GetType() == ValueID::ValueType_Int){
                        setInt(*it2);
                        return true;
                    }else if((*it2).GetType() == ValueID::ValueType_Bool){
                        setBool(*it2);
                        return true;
                    }
                    //Manager::Get()->SetValue((*valueIt), response);
                    break;
                }
            }
        }
    }
    return true;
}

bool UT_isDigit(string arg) {
    int i;
    
    for (i=0; i<(int)arg.size(); i++) {
        if (!isdigit(arg[i])) {
            return false;
        }
    }
    return true;
}

bool UT_isValueIdExists(string id, ValueID* ptr_valueID) {
    bool valueIdFound(false);

    for (auto it = nodes->begin(); it != nodes->end(); it++) {
        list<ValueID>::iterator it2;

        for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++) {
            if (to_string((*it2).GetId()) == id) {
                valueIdFound = true;
                break;
            }
        }

        if (valueIdFound) {
            *ptr_valueID = *it2;
            break;
        }
    }

    return valueIdFound;
}

bool UT_isNodeIdExists(string id) {
    bool nodeIdFound(false);

    for (auto it = nodes->begin(); it != nodes->end(); it++) {
        if (to_string((*it)->m_nodeId) == id) {
            nodeIdFound = true;
            break;
        }
    }
    return nodeIdFound;
}

string Five::receiveMsg(sockaddr_in address, int server_fd) {
    vector<string> args;
    char *ptr;

    string output("");
    string myFunc("");
    int counter(0);
    int new_socket(0);
    int valread(0);
    char buffer[1024] = {0};
    int addrlen = sizeof(address);
    
    for (int i = 0; i < 1024; i++) { buffer[i] = 0; }

    new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    valread = read(new_socket, buffer, 1024);

    output += "[" + getTime(convertDateTime(chrono::high_resolution_clock::now()));
    output += "] ";
    output += inet_ntoa(address.sin_addr);
    output += ":" + to_string(ntohs(address.sin_port));
    output += ", message(";
    output += to_string(valread);
    output += "): \"";
    output += buffer;
    output += "\" --> status: ";
    
    ptr = strtok(buffer, ",");


    while (ptr != NULL) {
        if (counter++ == 0) {
             for (unsigned long i = 0; i < sizeof(ptr); i++) {
                 if(ptr[i] != '\0'){
                     myFunc += ptr[i];
                 }
             }
        } else {
            args.push_back(ptr);
        }

        ptr = strtok(NULL, ",");
    }

    if (myFunc == "setValue") {
        if ((int)args.size() != 2) {
            output += "400, \"ArgumentError\"";
            return output;
        }

        if (!UT_isDigit(args[0])) {
            output += "404, \"ValueTypeError\"";
            return output;
        }
        
        ValueID valueID;

        if (!UT_isValueIdExists(args[0], &valueID)) {
            output += "404, \"ValueNotFound\"";
            return output;
        }

        Manager::Get()->SetValue(valueID, args[1]);                
        output += "200";
        return output;
    } else if (myFunc == "addNode") {
        //Putting the driver into a listening state
        Manager::Get()->AddNode(Five::homeID, false);
        output += "200";
        return output;
    } else if (myFunc == "rmvNode"){ //removeNode
        Manager::Get()->RemoveNode(Five::homeID);
        output += "200";
        return output;
    } else if (myFunc == "getValue"){
        int counter(0);
        string container;
        string* ptr_container = &container;

        if ((int)args.size() != 1) {
            output += "404, \"ArgError\"";
            return output;
        }

        if (!UT_isDigit(args[0])){
            output += "404, \"ValueTypeError\"";
            return output;
        }

        if(!UT_isNodeIdExists(args[0])){
            output += "404, \"NodeNotFound\"";
            return output;
        }

        for(auto it = nodes->begin(); it != nodes->end(); it++){
            if (stoi(args[0]) == (*it)->m_nodeId) {
                cout << "\n>>────|VALUES OF THE NODE " << to_string((*it)->m_nodeId) << "|────<<\n\n"
                    << "[<nodeId>] <label>, <value>, <readOnly>\n\n";
                for(auto it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++) {
                    Manager::Get()->GetValueAsString((*it2), ptr_container);
                    cout << "[" << counter++ << "] "
                            << Manager::Get()->GetValueLabel(*it2) << ", "
                            << *ptr_container << ", " 
                            << Manager::Get()->IsValueReadOnly(*it2)
                            << "                          " << to_string(it2->GetId()) << endl;
                } 
            }
        }
        output += "200";
        return output;
    } else if (myFunc == "resetKey"){

        if ((int)args.size() != 1) {
            output += "404, \"ArgError\"";
            return output;
        }

        if (UT_isDigit(args[0])){
            output += "404, \"ValueTypeError\"";
            return output;
        }

        if(args[0] == "hard"){
            Manager::Get()->ResetController(Five::homeID);
        } else if(args[0] == "soft"){
            Manager::Get()->SoftReset(Five::homeID);
        }
        
        output += "200";
        return output;
    } else if (myFunc == "hlNode"){ //healNode
        if ((int)args.size() != 1) {
            output += "404, \"ArgError\"";
            return output;
        }

        if (!UT_isDigit(args[0])){
            output += "404, \"ValueTypeError\"";
            return output;
        }

        if(!UT_isNodeIdExists(args[0])){
            output += "404, \"NodeNotFound\"";
            return output;
        }

        for(auto it = nodes->begin(); it != nodes->end(); it++){
            if (stoi(args[0]) == (*it)->m_nodeId) {
                Manager::Get()->HealNetworkNode((*it)->m_homeId, (*it)->m_nodeId, true);
            }
        }
        output += "200";
        return output;
    } else if (myFunc == "isFailed"){
        bool isFailed = false;
        if ((int)args.size() != 1) {
            output += "404, \"ArgError\"";
            return output;
        }

        if (!UT_isDigit(args[0])){
            output += "404, \"ValueTypeError\"";
            return output;
        }

        if(!UT_isNodeIdExists(args[0])){
            output += "404, \"NodeNotFound\"";
            return output;
        }

        for(auto it = nodes->begin(); it != nodes->end(); it++){
            if (stoi(args[0]) == (*it)->m_nodeId) {
                isFailed = Manager::Get()->IsNodeFailed(Five::homeID, (*it)->m_nodeId);
            }
        }
        if(isFailed){
        output += "200, Node " + args[0] + " is failed";
        } else {
        output += "200, Node " + args[0] + " is not failed";
        }
        return output;
    } else if (myFunc == "Ping"){
        Manager::Get()->RemoveNode(Five::homeID);
        output += "200";
        return output;
    } else if (myFunc == "Brdcast"){
        Manager::Get()->RemoveNode(Five::homeID);
        output += "200";
        return output;
    } else if (myFunc == "Nghbors"){
        Manager::Get()->RemoveNode(Five::homeID);
        output += "200";
        return output;
    } else if (myFunc == "Polls"){
        Manager::Get()->RemoveNode(Five::homeID);
        output += "200";
        return output;
    } else if (myFunc == "Map"){
        Manager::Get()->RemoveNode(Five::homeID);
        output += "200";
        return output;
    }
    output += "300, \"Unsupported message\"";
    return output;
}

void Five::server(int port) {
	int server_fd;
	struct sockaddr_in address;
	int opt = 1;
	
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	bind(server_fd, (struct sockaddr*)&address, sizeof(address));
	listen(server_fd, 3);
    cout << "Listening on port " << port << endl;
    
    while (true) {
        cout << receiveMsg(address, server_fd) << endl;
    }
}

int Five::sendMsg(const char* address, const int port, string message) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return EXIT_FAILURE;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return EXIT_FAILURE;
    }

    // cout << "-> MSG: " << to_string(message.length()) << endl;
    // cout << message << endl;

    char fMessage[message.length() + 1];
    strcpy(fMessage, message.c_str());

    for (int i = 0; i < (int)message.length(); i++) {
        fMessage[i] = message[i];
    }

    send(sock, fMessage, strlen(fMessage), 0);
    
    return EXIT_SUCCESS;
}

string convertToString(char* a, int size)
{
    int i;
    string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

string Five::buildNotifMsg(Notification const *notification) {
    string msg = "";
    msg += "\"notificationType\": \"";
    msg += NOTIFICATIONS[(int)notification->GetType()];
    msg += "\", \"object\": { \"nodeId\": ";
    msg += to_string(notification->GetNodeId());
    msg += ", \"manufacturerName\": \"";
    msg += Manager::Get()->GetNodeManufacturerName(homeID, notification->GetNodeId());
    msg += "\", \"nodeDead\": ";
    msg += to_string(getNode(notification->GetNodeId(), nodes)->m_isDead);
    msg += ", \"lastUpdate\": \"";
    
    NodeInfo* node = getNode(notification->GetNodeId(), nodes);
    list<ValueID> values = node->m_values;
    string date = getDate(convertDateTime(node->m_sync));
    string time = getTime(convertDateTime(node->m_sync));

    msg += date;
    msg += " ";
    msg += time;
    msg += "\", \"valueIds\": [ ";

    for (auto it=values.begin(); it!=values.end(); it++) {
        if (it != values.begin()) msg += ", ";

        msg += "{ \"id\": \"";
        msg += to_string(it->GetId());
        msg += "\", \"readonly\": ";
        msg += to_string(Manager::Get()->IsValueReadOnly(*it));
        msg += ", \"label\": \"";
        msg += Manager::Get()->GetValueLabel(*it);
        msg += "\", \"value\": ";

        ValueID::ValueType valType = it->GetType();
        bool isNumeric(false);
        string value;

        Manager::Get()->GetValueAsString(*it, &value);

        for (int i=0; i<(int)(sizeof(NUMERIC_TYPES)/sizeof(int)); i++) {
            if (NUMERIC_TYPES[i] == valType) {
                isNumeric = true;
                break;
            }
        }

        if (valType == ValueID::ValueType::ValueType_Bool) {
            value[0] = tolower(value[0]);
        }

        if (isNumeric) {
            msg += value;
        } else {
            if (value == "False" || value == "True") {
                value[0] = tolower(value[0]);
                msg += value;
            } else {
                msg += "\"";
                msg += value;
                msg += "\"";
            }
        }

        msg += " }";
    }

    msg += " ] } }";

    msg = ", " + msg;
    string header = "{ \"msgLength\": ";
    int msgLength = msg.length() + header.length();

    msg = to_string(msgLength + to_string(msgLength).length()) + msg;
    msg = header + msg;
    return msg;
}

void Five::statusObserver(list<NodeInfo*> *nodes) {
    while(true) {
		fstream file;
		string line;
		list<NodeInfo*>::iterator it;
		list<ValueID>::iterator it2;
		int32 wakeUpInterval;

		// Get the current counter from epoch.
		int64 now = chrono::high_resolution_clock::now().time_since_epoch().count();


		Driver::ControllerState newState( Driver::ControllerState::ControllerState_Error );

		if (homeID != 0)
			newState = Manager::Get()->GetDriverState(homeID);

		if (driverState != newState) {
			cout << "State: " << STATES[newState] << "\n";
			driverState = newState;
		}

		for (it = nodes->begin(); it != nodes->end(); it++) {
			list<ValueID> valueIDs = (*it)->m_values;

			for (it2 = valueIDs.begin(); it2 != valueIDs.end(); it2++) {
				string label = Manager::Get()->GetValueLabel(*it2);

				if (label.find("Wake-up Interval") != string::npos) { // The object has a sleep interval.
					Manager::Get()->GetValueAsInt(*it2, &wakeUpInterval); // Stores the object wakeUpInteral value.

					// Check if (now - lastUpdate) is greater than the wakeUpInterval
					// or if the object is not synchronized yet.
					if (((now - (*it)->m_sync.time_since_epoch().count()) > (wakeUpInterval * pow(10, 9))) || convertDateTime((*it)->m_sync)->tm_hour == 1) {
						(*it)->m_isDead = true;

						// file.open(FAILED_NODE_PATH, ios::in);
						// while(getline(file, line)){
						// 	if(line.find("Label: " + (*it)->m_name) != string::npos){
						// 		isIn = 1;
						// 	}
						// }
						// file.close();
						// if(!isIn){
						// 	file.open(FAILED_NODE_PATH, ios::app);
						// 	file << "Label: " << (*it)->m_name << " Id: " << unsigned((*it)->m_nodeId) << " Type: " << (*it)->m_nodeType << endl;
						// 	file.close();
						// }
					}else {
						(*it)->m_isDead = false;

						// file.open(FAILED_NODE_PATH, ios::in);
						// fstream temp;
						// temp.open("temp.txt", ios::app);
						// while(getline(file, line)){
						// 	cout << line << endl;
						// 	string s = "Label: " + (*it)->m_name;
						// 	cout << s << endl;
						// 	if(line.find(s) == string::npos){
						// 		temp << line;
						// 	}
						// }
						// temp.close();
						// file.close();

						// const char *p = FAILED_NODE_PATH.c_str();
						// remove(p);
						// rename("temp.txt", p);
					}
					break;
				}
			}
		}

		this_thread::sleep_for(chrono::milliseconds(Five::OBSERVER_PERIOD));
	}
}