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
#include <thread>

#define PORT 5101

using namespace OpenZWave;
using namespace Five;

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
                if (ptr[i] != '\0') {
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

        for (int i = 0; i < (int)args.size(); i++) {
            if (i == 0) {
                if (!UT_isDigit(args[i])) {
                    output += "400, \"ValueTypeError\"";
                    return output;
                }
                
                ValueID valueID;

                if (!UT_isValueIdExists(args[i], &valueID)) {
                    output += "404, \"ValueNotFound\"";
                    return output;
                }

                Manager::Get()->SetValue(valueID, args[1]);                
                output += "200";
                return output;
            }
        }
    } else if (myFunc == "addNode") {
        Manager::Get()->AddNode(homeID, false);
    } else if (myFunc == "rmNode") {
        Manager::Get()->RemoveNode(homeID);
    }

    output += "200";
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