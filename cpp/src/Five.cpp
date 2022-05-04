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
#include <vector>
#include <regex>
#include "../../json/single_include/nlohmann/json.hpp"

#define PORT 5101

using namespace OpenZWave;
using namespace Five;
using namespace std;
using namespace nlohmann;

//Returns the node object whose ID you've given
NodeInfo* Five::getNode(uint8 nodeID, list<NodeInfo*> *nodes) {
    list<NodeInfo*>::iterator it;
    for (it = nodes->begin(); it != nodes->end(); it++) {
        if ((*it)->m_nodeId == nodeID) {
            return (*it);
        }
    }
    return NULL;
}

//Creates a node object containing all the informations about the node whose ID you've given
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

    for (int i = 0; i < 29; i++) {
		uint8 bite{ 0 };
		uint8 *bitmapPixel = &bite;
		node.m_neighbors[i] = bitmapPixel;
	}

    if (nodeId != 1)
    {
        Manager::Get()->GetNodeNeighbors(homeId, nodeId, node.m_neighbors);
    }
    
    return node;

}

// A value has changed on the Z-Wave network and this is a different value.
bool Five::valueChanged(Notification const* notification, list<NodeInfo*> *nodes) {
    ValueID valueId = notification->GetValueID();

    for (auto it = nodes->begin(); it != nodes->end(); it++) {
        if ((*it)->m_nodeId == valueId.GetNodeId()) {
            (*it)->m_values.push_back(valueId);
        }
    }

    return true;
}

// Allows to turn on or off any object who has a switch parameter.
bool Five::setSwitch(ValueID valueId, bool state) {   
    string answer;
    cout << "true(1) or false(0) ?" << endl;
	cin >> answer;

    if (answer=="true" || answer=="True" || answer == "1") {
        state = true;
    } else if (answer == "false" || answer == "False" || answer == "0") {
        state = false;
    }

    Manager::Get()->SetValue(valueId, state);
    return true;
}

// Allows to set the value of an intensity type value.
bool Five::setIntensity(ValueID valueId, int intensity) {
    Manager::Get()->SetValue(valueId, to_string(intensity));
    return true;
}

// Allows to change the color of a light by giving an hexadecimal value.
bool Five::setHexColor(ValueID valueId, string hexColor) {

	if (regex_match(hexColor, regex("^#[0-9a-zA-Z]{10}$"))) {
		Manager::Get()->SetValue(valueId, hexColor);
		return true;
	}

	return false;
}

// Allows to change the value of a list type value by displaying all possible values.
bool Five::setList(ValueID valueId) {
    vector<string> vectS;
    string response;
    string s;

    int counter = 0;
    
	Manager::Get()->GetValueListItems(valueId, &vectS) ;
    Manager::Get()->GetValueListSelection(valueId, &s);
    
	cout << "Current Value: " << s << endl;
    
	for(auto it = vectS.begin(); it != vectS.end(); ++it){
        cout << "[" << ++counter << "] " << *it << endl;
    }
	
	counter = 0;
	cout << "Select: " << endl;
	cin >> response;

    for(auto it = vectS.begin(); it != vectS.end(); ++it){
        counter++;
        if (response == to_string(counter)) {
            cout << *it << endl;
            Manager::Get()->SetValueListSelection(valueId, *it);
        }
    }
    return true;
}

// Allows to set the intensity of a volume parameter.
bool Five::setVolume(ValueID valueId, int intensity) {

	if (intensity >= 0 && intensity < 100) {
		Manager::Get()->SetValue(valueId, to_string(intensity));
		return true;
	}

	return false;
}

// Allows to set the value of a duration parameter.
bool Five::setDuration(ValueID valueId, int duration) {

	if (duration >= 0) {
		Manager::Get()->SetValue(valueId, duration);
    	return true;
	}

	return false;    
}

// Allows to set the value of a int parameter.
bool Five::setInt(ValueID valueId) {
    string response;
    
	cout << "Please enter a value in Int: " << endl;
    cin >> response;

	if (UT_isDigit(response)) {
		Manager::Get()->SetValue(valueId, response);
    	return true;
	}

	cout << "Not a digit." << endl;
	return false;
}

// Allows to set the value of a boolean parameter.
bool Five::setBool(ValueID valueId) {
    string answer;

    cout << "true(1) or false(0) ?" << endl;
	cin >> answer;

	string trues[] = {"true", "True", "1"};
	string falses[] = {"false", "False", "0"};

	for (long unsigned int i = 0; i < sizeof(trues)/sizeof(trues[0]); i++) {		
		if (answer == trues[i]) {
			Manager::Get()->SetValue(valueId, true);
			return true;
		} else if (answer == falses[i]) {
			Manager::Get()->SetValue(valueId, false);
			return true;
		}
	}

	cout << "Unsupported value." << endl;
	return false;
}

// Allows to push or release a button parameter.
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

	NodeInfo *n = new NodeInfo();
	n->m_homeId		= homeId;
	n->m_nodeId		= nodeId;
	n->m_name     	= Manager::Get()->GetNodeProductName(homeId, nodeId);
	n->m_nodeType 	= notification->GetValueID().GetTypeAsString();

    for (int i = 0; i < 29; i++) {
		uint8 bite = 0;
		uint8 *bitmapPixel = &bite;
		n->m_neighbors[i] = bitmapPixel;
	}

	return n;
}

// Check if <nodeID> exists in <nodes>.
bool Five::isNodeNew(uint8 nodeID, list<NodeInfo*> *nodes) {
	for (auto it = nodes->begin(); it != nodes->end(); it++) {
		if ((*it)->m_nodeId == nodeID) {
			return false;
		}
	}

	return true;
}

//Returns the number of dead nodes in <nodes>
int Five::deadNodeSum(list<NodeInfo*> *nodes) {
	int counter = 0;

	for (auto it = nodes->begin(); it != nodes->end(); it++) {
		if ((*it)->m_isDead && (*it)->m_nodeId != 1) {
			counter++;
		}
	}

	return counter;
}

//Returns the number of alive nodes in <nodes>
int Five::aliveNodeSum(list<NodeInfo*> *nodes) {
	return (nodes->size() - 1) - deadNodeSum(nodes);
}

//Check if type <needle> is in list <haystack>
bool Five::containsType(Notification::NotificationType needle, vector<Notification::NotificationType> haystack) {
    for (int i = 0; i < int(haystack.capacity()); i++) {
        if (needle == haystack[i]) {
            return true;
        }
    }
    return false;
}

//Check if status <needle> is in list <haystack>
bool Five::containsStatus(StatusCode needle, vector<StatusCode> haystack) {
    for (int i = 0; i < int(haystack.capacity()); i++) {
        if (needle == haystack[i]) {
            return true;
        }
    }
    return false;
}

//Check if node <needle> is in list <haystack>
bool Five::containsNodeID(uint8 needle, list<NodeInfo*> haystack) {
    list<NodeInfo*>::iterator it;
    for (it = haystack.begin(); it != haystack.end(); it++) {
        if ((*it)->m_nodeId == needle) {
            return true;
        }
    }
    return false;
}

// Refresh members in the oldNodeInfo thanks to this valueID.
void Five::refreshNode(ValueID valueID, NodeInfo* oldNodeInfo) {
	oldNodeInfo->m_homeId = valueID.GetHomeId();
	oldNodeInfo->m_name = Manager::Get()->GetNodeName(valueID.GetHomeId(), valueID.GetNodeId());
	oldNodeInfo->m_nodeType = Manager::Get()->GetNodeType(valueID.GetHomeId(), valueID.GetNodeId());
	oldNodeInfo->m_values.push_back(valueID);
}

//Takes the node linked to <notification>, and adds it to <node>
void Five::pushNode(Notification const *notification, list<NodeInfo*> *nodes) {
	NodeInfo* n{ createNode(notification) };
	uint8 nodeID{ notification->GetNodeId() };

	if (isNodeNew(nodeID, nodes)) {
		nodes->push_back(n);
	}
}

//Takes the node linked to <notification>, and removes it from <node>
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

//Adds a value to a node's values list
bool Five::addValue(ValueID valueID, NodeInfo *node) {
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

//Removes a value from a node's values list
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

//Returns the current Date and Time
chrono::high_resolution_clock::time_point Five::getCurrentDatetime() {
    return std::chrono::high_resolution_clock::now();
}

//Converts the Date and Time to the wanted format
tm* Five::convertDateTime(chrono::high_resolution_clock::time_point datetime) {
    time_t convertTime{ std::chrono::high_resolution_clock::to_time_t(datetime) };
    return localtime(&convertTime);
}

//Returns only the time
string Five::getTime(tm* datetime) {
	string zeroMin = "";
	string zeroHour = "";
	string zeroSec = "";

	if (datetime->tm_min < 10) zeroMin = "0";
	if (datetime->tm_hour < 10) zeroHour = "0";
	if (datetime->tm_sec < 10) zeroSec = "0";

    return zeroHour + to_string(datetime->tm_hour) + ":" + zeroMin + to_string(datetime->tm_min) + ":" + zeroSec + to_string(datetime->tm_sec);
}

//Returns only the date
string Five::getDate(tm* datetime) {
	string zeroDay = "";
	string zeroMonth = "";

	if (datetime->tm_mday < 10) zeroDay = "0";
	if (datetime->tm_mon < 10) zeroMonth = "0";

    return to_string(datetime->tm_year + 1900) + "-" + zeroMonth + to_string(datetime->tm_mon + 1) + "-" + zeroDay + to_string(datetime->tm_mday);
}

//Returns the elapsed time between two date/time
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

//Iterrates through the node list and asks the user to pick one (console command)
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
//Prints the values of a node, and eventually asks for a choice (depending on bool value)
bool Five::printValues(int* choice, list<NodeInfo*>::iterator* it, list<ValueID>::iterator it2, bool getOnly) {
    string container;
    list<string>::const_iterator sIt;
    string response;
    int counterValue(0);
    for(*it = nodes->begin(); *it != nodes->end(); (*it)++){
        if (*choice == ((**it)->m_nodeId)) {
            if(getOnly) {
                cout << "\n>>────|VALUES OF THE NODE " << to_string((**it)->m_nodeId) << "|────<<\n\n"
                     << "[<nodeId>] <label>, <value>, <readOnly>\n\n";
                for(it2 = (**it)->m_values.begin(); it2 != (**it)->m_values.end(); it2++) {
                    Manager::Get()->GetValueAsString((*it2), &container);
                    cout << "[" << counterValue++ << "] "
                         << Manager::Get()->GetValueLabel(*it2) << ", "
                         <<  container << ", "
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

//Sets a value using specific setValue function depending on the value
bool Five::newSetValue(int* choice, list<NodeInfo*>::iterator* it, list<ValueID>::iterator it2, bool isOk) {
    list<string>::const_iterator sIt;
    string container;
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
                    Manager::Get()->GetValueAsString((*it2), &container);
                    cout << "Current value: " <<  container << endl;
                    // cout << "Set to what ? ";
                    //cin >> response;

                    //Checking value type to choose the right method
                    if(valLabel.find("Switch") != string::npos){
                        setSwitch((*it2), true);
                        return true;
                    }else if(valLabel.find("Color") != string::npos && (*it2).GetType() == ValueID::ValueType_String)
                    {
						cout << "Set color: ";
						cin >> response;
                        setHexColor(*it2, response);
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

                    } else if (valLabel.find("Volume") != string::npos) {
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
                    } else if (valLabel.find("Duration") != string::npos) {
                        cout << "Set duration: ";
						cin >> response;
						if (UT_isDigit(response)) {
							setDuration(*it2, stoi(response));
						}
                        return true;
                    } else if ((*it2).GetType() == ValueID::ValueType_Int) {
                        setInt(*it2);
                        return true;
                    } else if ((*it2).GetType() == ValueID::ValueType_Bool) {
                        setBool(*it2);
                        return true;
                    }

                    break;
                }
            }
        }
    }
    return true;
}

//Checks if an arg contains digits or not
bool Five::UT_isDigit(string arg) {
    int i;

    for (i=0; i<(int)arg.size(); i++) {
        if (!isdigit(arg[i])) {
            return false;
        }
    }
    return true;
}

//Checks if the given id corresponds to an existing ValueID, and if yes places it in pointer
bool Five::UT_isValueIdExists(string id, ValueID* ptr_valueID) {
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

//Checks if the given id corresponds to an existing Node
bool Five::UT_isNodeIdExists(string id) {
    bool nodeIdFound(false);

    for (auto it = nodes->begin(); it != nodes->end(); it++) {
        if (to_string((*it)->m_nodeId) == id) {
            nodeIdFound = true;
            break;
        }
    }
    return nodeIdFound;
}

// Build the message to php client in a json format.
string Five::buildPhpMsg(string commandName, vector<string> args, int *running) {
    fstream socketFile;
    string errorPath = SOCKET_LOG_PATH + "errors.log";
    string infoPath = SOCKET_LOG_PATH + "info.log";
    vector<StatusCode> validCode = {VALID_ok, VALID_created, VALID_accepted, VALID_noContent};
    vector<StatusCode> invalidCode = {INVALID_badRequest, INVALID_forbidden, INVALID_imATeapot, INVALID_methodNotAllowed, INVALID_notAcceptable, INVALID_notFound, INVALID_requestTimeout, INVALID_unauthorized, SERVER_bandWidthLimitExceeded, SERVER_notImplemented, SERVER_unavailable};
    Message msg = Message::InvalidCommand;
    StatusCode status = StatusCode::INVALID_badRequest;
    ValueID valueID;
    
    auto awakeTime = (getCurrentDatetime().time_since_epoch().count() - startedAt.time_since_epoch().count()) / 1000000000;
    string body = "";
    body += "\"upTime\": " + to_string(awakeTime) + ", ";
    body += "\"commandName\": \"" + commandName + "\", ";
    body += "\"args\": [";

    for (auto it = args.begin(); it != args.end(); it++) {
        if (it != args.begin()) {
            body += ", ";
        }

        body += '\"' + *it + '\"';
    }

    body += "], \"body\": { ";

    if (commandName == COMMANDS[0].name) { // setValue
        if ((int)args.size() < 2) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::ArgumentError;
		} else if (!UT_isDigit(args[0])) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::InvalidArgument;
        } else if (!UT_isValueIdExists(args[0], &valueID)) {
            status = StatusCode::INVALID_notFound;
            msg = Message::ValueNotFoundError;
        } else if((int)args.size() > 2){
			cout << "test" << endl;
			status = StatusCode::VALID_accepted;
            msg = Message::None;
			for(int i = 2; i < (int)args.size(); i++ ){
				args[1] += " " + args[i];
			}
			Manager::Get()->SetValue(valueID, args[1]);
		} else{
            status = StatusCode::VALID_accepted;
            msg = Message::None;
            Manager::Get()->SetValue(valueID, args[1]);
        }
    } else if (commandName == COMMANDS[1].name) { // include
        status = StatusCode::VALID_created;
        msg = Message::None;
        Manager::Get()->AddNode(Five::homeID, false);
    } else if (commandName == COMMANDS[2].name) { // exclude
        status = StatusCode::VALID_accepted;
        msg = Message::None;
        Manager::Get()->RemoveNode(Five::homeID);
    } else if (commandName == COMMANDS[3].name) { // getNode
        if ((int)args.size() == 0) {
            status = StatusCode::VALID_ok;
            msg = Message::None;
            body += "\"nodes\": [ ";
            for (auto it = nodes->begin(); it != nodes->end(); it++) {
                if (it != nodes->begin()) {
                    body += ", ";
                }
                body += nodeToJson(getNode((*it)->m_nodeId, nodes));
            }
            body += " ], ";

        } else if ((int)args.size() != 1) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::ArgumentError;
        } else  if (!UT_isDigit(args[0])){
            status = StatusCode::INVALID_badRequest;
            msg = Message::ValueTypeError;
        } else if(!UT_isNodeIdExists(args[0])){
            status = StatusCode::INVALID_badRequest;
            msg = Message::NodeNotFoundError;
        } else {
            status = StatusCode::VALID_ok;
            msg = Message::None;

            for(auto it = nodes->begin(); it != nodes->end(); (it)++){
                if (stoi(args[0]) == ((*it)->m_nodeId)) {
                    body += "\"node\": " + nodeToJson(getNode((*it)->m_nodeId, nodes)) + ", "; // Display node information.
                }
            }
        }
    } else if (commandName == COMMANDS[4].name) { // reset
        if ((int)args.size() != 1) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::ArgumentError;
        } else if (UT_isDigit(args[0])) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::ValueTypeError;
        } else if (args[0] == "hard") {
            status = StatusCode::VALID_accepted;
            msg = Message::None;
            Manager::Get()->ResetController(Five::homeID);
        } else if(args[0] == "soft") {
            status = StatusCode::VALID_accepted;
            msg = Message::None;
            Manager::Get()->SoftReset(Five::homeID);
        } else if (args[0] == "bash") {
			status = StatusCode::VALID_accepted;
            msg = Message::None;
			*running = 1;
		} else {
            status = StatusCode::INVALID_badRequest;
            msg = Message::InvalidArgument;
        }
    } else if (commandName == COMMANDS[5].name) { // heal
        if ((int)args.size() == 0) {
            status = StatusCode::VALID_noContent;
            msg = Message::None;
            Manager::Get()->HealNetwork(homeID, true);
        } else if ((int)args.size() != 1) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::ArgumentError;
        } else if (!UT_isDigit(args[0])) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::ValueTypeError;
        } else if (!UT_isNodeIdExists(args[0])) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::NodeNotFoundError;
        } else {
            status = StatusCode::VALID_noContent;
            msg = Message::None;
            for(auto it = nodes->begin(); it != nodes->end(); it++){
                if (stoi(args[0]) == (*it)->m_nodeId) {
                    Manager::Get()->HealNetworkNode((*it)->m_homeId, (*it)->m_nodeId, true);
                }
            }
        }
    } else if (commandName == COMMANDS[6].name) { // isFailed
        bool isFailed = false;

        if ((int)args.size() != 1) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::ArgumentError;
        } else if (!UT_isDigit(args[0])){
            status = StatusCode::INVALID_badRequest;
            msg = Message::ValueTypeError;
        } else if(!UT_isNodeIdExists(args[0])){
            status = StatusCode::INVALID_badRequest;
            msg = Message::NodeNotFoundError;
        } else {
            status = StatusCode::VALID_noContent;
            msg = Message::None;

            for (auto it = nodes->begin(); it != nodes->end(); it++) {
                if (stoi(args[0]) == (*it)->m_nodeId) {
                    isFailed = Manager::Get()->IsNodeFailed(Five::homeID, (*it)->m_nodeId);
                    break;
                }
            }

            body += "\"isFailed\": " + to_string(isFailed) + ", ";
        }
    } else if (commandName == COMMANDS[7].name) { // ping
        string message;
        int countTrue = 0;
        int countFalse = 0;

        if ((int)args.size() != 2) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::ArgumentError;
        }else for(int i = 0; i < 2; i++) {
            if (!UT_isDigit(args[i])){
                status = StatusCode::INVALID_badRequest;
                msg = Message::ValueTypeError;
            }
        } 
        if (!UT_isNodeIdExists(args[0])) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::NodeNotFoundError;
        } else {
            status = StatusCode::VALID_noContent;
            msg = Message::None;
            for (auto it = nodes->begin(); it != nodes->end(); it++) {
                if (args[0] == to_string((*it)->m_nodeId)) {
                    for (auto it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++) {
                        if (Manager::Get()->GetValueLabel(*it2) == "Library Version") {
                            int counter{stoi(args[1])};
                            while (counter --> 0) {
                                Manager::Get()->RefreshValue(*it2);
                                // msg = "ask node " + to_string((*it)->m_nodeId) + " ... " + to_string(Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId)) + "\n";
                                // sendMsg(LOCAL_ADDRESS, PHP_PORT,  msg);
                                if(Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId)){
                                    countTrue += 1;
                                } else{
                                    countFalse += 1;
                                }
                                this_thread::sleep_for(chrono::milliseconds(500));
                            }

                        }
                    }
                }
            }
        }
        body = "\"Ping sent\": " + args[1] + ", \"received\": " + to_string(countTrue) + ", \"failed\": " + to_string(countFalse) + ", ";
    } else if (commandName == COMMANDS[8].name) { // help
        status = StatusCode::VALID_ok;
        msg = Message::None;
        int cmdCounter = sizeof(COMMANDS)/sizeof(COMMANDS[0]);

        body += "\"commands\": [ ";
        for (int i = 0; i < cmdCounter; i++) {
            if (i != 0) {
                body += ", ";
            }
            body += "{ \"name\": \"" + COMMANDS[i].name + "\", ";
            body += "\"args\": [ ";
            for (auto it = COMMANDS[i].arguments.begin(); it != COMMANDS[i].arguments.end(); it++) {
                if (it != COMMANDS[i].arguments.begin()) {
                    body += ", ";
                }
                body += '\"' + *it + '\"';
            }
            body += " ], \"description\": \"" + COMMANDS[i].description + "\" }";
        }
        body += " ], ";
    } else if (commandName == COMMANDS[9].name) { //broadcast
        string message;
        int countTrue = 0;
        int countFalse = 0;
        bool pinged(false);

        if((int)args.size() != 0){
            status = StatusCode::INVALID_badRequest;
            msg = Message::ArgumentError;
        } else {
            for(auto it = nodes->begin(); it != nodes->end(); ++it){
                for(auto it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++){
                    if (Manager::Get()->GetValueLabel(*it2) == "Library Version") {
                        int counter{ 60 };
                        while (counter --> 0) {
                            Manager::Get()->RefreshValue(*it2);
                            this_thread::sleep_for(chrono::milliseconds(500));
                            message = "ask node " + to_string((*it)->m_nodeId) + " ... " + to_string(Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId)) + "\n";
                            sendMsg(LOCAL_ADDRESS, PHP_PORT, message);
                            if(Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId)){
                                counter = 0;
                                pinged = true;
                            }
                        }
                    }
                    if(pinged){
                        countTrue += 1;
                    } else{
                        countFalse += 1;
                    }
                }
            }
            body += "\"successful nodes\": " + to_string(countTrue) + " \"failed nodes\": " + to_string(countFalse) + ", ";

        }
    } else if (commandName == COMMANDS[10].name) { // reboot
        status = StatusCode::VALID_ok;
        msg = Message::None;
		*running = 2;
    } else if (commandName == COMMANDS[11].name) { // setMode
		if (args.capacity() != 1) {
			status = StatusCode::INVALID_badRequest;
			msg = Message::ArgumentError;
		} else {
			bool found = false;
			for (unsigned long i = 0; i < sizeof(modes)/sizeof(modes[0]); i++) {
				cout << modes[i]->name << ", " << args[0] << endl;
				if (modes[i]->name == args[0]) {
					currentMode = modes[i];
					found = true;
					status = StatusCode::VALID_noContent;
					msg = Message::None;
					ifstream ifs("/srv/app/zwave/cpp/examples/cache/config.json");
					json j;
					ifs >> j;
					j["mode"]["name"] = currentMode->name;
					j["mode"]["log"] = levels[currentMode->log];
					j["mode"]["pollInterval"] = currentMode->pollInterval;
					ofstream ofs("/srv/app/zwave/cpp/examples/cache/config.json");
					ofs << setw(2) << j << endl;
					break;
				}
			}

			if (!found) {
				status = StatusCode::INVALID_badRequest;
				msg = Message::InvalidArgument;
			}
		}
	} else if (commandName == COMMANDS[12].name) {
		*running = 3;
		status = StatusCode::VALID_noContent;
		msg = Message::None;
	}

	Level lvl = Level::DEBUG;

	if (200 <= status && status < 300) {
		lvl = Level::INFO;
	} else if (400 <= status && status < 500) {
		lvl = Level::ERROR;
	} else if (500 <= status) {
		lvl = Level::NOTICE;
	}

	string argsString = "";
	for (int i = 0; i < (int)args.capacity(); i++) {
		if (i > 0)
			argsString += ",";
		argsString += args[i];
	}

	writeLog(lvl, __FILE__, __LINE__, "socketRequest", "cmdName=\"" + commandName + "\"&args=\"" + argsString + "\"&status=" + to_string(status) + "&msg=\"" + messages[msg] + '\"');
    
    body += "\"status\": " + to_string(status);
    body += ", \"message\": \"" + messages[msg] + "\" }, ";
	body += "\"mode\": \"" + currentMode->name + "\", ";
	body += "\"logLevel\": \"" + levels[currentMode->log] + "\" }";

    int body_length = body.length();
    body = "{ \"messageLength\": " + to_string(body_length + to_string(body_length).length()) + ", " + body;

    switch(logs) {
        case Level::DEBUG:
			if(containsStatus(status, validCode)){
				socketFile.open(infoPath, ios::app);
				socketFile << "[" << getDate(convertDateTime(getCurrentDatetime())) << ", " << getTime(convertDateTime(getCurrentDatetime())) << "] " << "\"status\": " 
				<< to_string(status) << ", \"message\": \"" << messages[msg] << "\", " << commandName;
				for(int i = 0; i < (int)args.size(); i++){
					socketFile << "," << args[i];
				}
				socketFile << endl;
				socketFile.close();
			} else if(containsStatus(status, invalidCode)) {
				socketFile.open(errorPath, ios::app);
				socketFile << "[" << getDate(convertDateTime(getCurrentDatetime())) << ", " << getTime(convertDateTime(getCurrentDatetime())) << "] " << "\"status\": " 
				<< to_string(status) << ", \"message\": \"" << messages[msg] << "\", " << commandName;
				for(int i = 0; i < (int)args.size(); i++){
					socketFile << "," << args[i];
				}
				socketFile << endl;
				socketFile.close();
			}
            break;
        case Level::INFO:
			if(containsStatus(status, validCode)){
				socketFile.open(infoPath, ios::app);
				socketFile << "[" << getDate(convertDateTime(getCurrentDatetime())) << ", " << getTime(convertDateTime(getCurrentDatetime())) << "] " << "\"status\": " 
				<< to_string(status) << ", \"message\": \"" << messages[msg] << "\", " << commandName;
				for(int i = 0; i < (int)args.size(); i++){
					socketFile << "," << args[i];
				}
				socketFile << endl;
				socketFile.close();
			}
            break;
        case Level::NOTICE:
			if(containsStatus(status, invalidCode)){
				socketFile.open(errorPath, ios::app);
				socketFile << "[" << getDate(convertDateTime(getCurrentDatetime())) << ", " << getTime(convertDateTime(getCurrentDatetime())) << "] " << "\"status\": " 
				<< to_string(status) << ", \"message\": \"" << messages[msg] << "\", " << commandName;
				for(int i = 0; i < (int)args.size(); i++){
					socketFile << "," << args[i];
				}
				socketFile << endl;
				socketFile.close();
			}
            break;
        default:
            break;
    }

    return body;
}

//Receives and translates the messages received from PHP client
int Five::receiveMsg(sockaddr_in address, int server_fd) {
    vector<string> args;
    char *ptr;
    ValueID valueID;

    string output("");
    string myFunc("");
    int counter(0);
    int new_socket(0);
    int valread(0);
    char buffer[1024] = {0};
    int addrlen = sizeof(address);

    for (int i = 0; i < 1024; i++) {
        buffer[i] = 0;
    }

    new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    valread = read(new_socket, buffer, 1024);
    cout << valread << endl;

    ptr = strtok(buffer, ",");
	cout << ptr << endl;

    while (ptr != NULL) {
        if (counter++ == 0) {
			myFunc = ptr;
		} else {
			args.push_back(ptr);
		}

        ptr = strtok(NULL, ",");
    }
	int running = 0;
    string msg = buildPhpMsg(myFunc, args, &running);

    char fMessage[msg.length() + 1];
    strcpy(fMessage, msg.c_str());

    for (int i = 0; i < (int)msg.length(); i++) {
        fMessage[i] = msg[i];
    }

    send(new_socket, fMessage, strlen(fMessage), 0);
	return running;
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
    cout << "[server] listening on 127.0.0.1:" << port << endl;

	int running = 0;
    while (running == 0) {
        running = receiveMsg(address, server_fd);
    }

	Manager::Get()->RemoveDriver(driverPath);
	Manager::Get()->RemoveWatcher(Five::onNotification, NULL);
	Manager::Destroy();
	
	for (auto i = nodes->begin(); i != nodes->end(); i++) {
		NodeInfo *node = *i;
		node->m_values.clear();
		delete node;
	}
	close(server_fd);
	pthread_mutex_destroy(&g_criticalSection);

	switch (running) {
		case 1: // reset
			cout << system("sh install.sh");		
		case 2: // reboot
			cout << system("sh reboot.sh");
		case 3: // shutdown
			cout << system("sh shutdown.sh");
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
    serv_addr.sin_addr.s_addr = inet_addr(address);

    if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return EXIT_FAILURE;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed \n");
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

//Converts a char array into a string
string convertToString(char* a, int size)
{
    int i;
    string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

//Converts the information of a node into json format suitable to send
string Five::nodeToJson(NodeInfo* node) {
    string msg = "";
	uint8* association;
	uint32 assoNb = Manager::Get()->GetAssociations(homeID, node->m_nodeId, 2, &association);
    msg += "{ \"nodeId\": " + to_string(node->m_nodeId);
    msg += ", \"productName\": \"" + node->m_name;
    msg += "\", \"nodeDead\": " + to_string(node->m_isDead);
	msg += ", \"groupsNumber\": " + to_string(Manager::Get()->GetNumGroups(homeID, node->m_nodeId));
	msg += ", \"nodeAssociations\": " + to_string(assoNb);
	msg += ", \"nodeAssociated\": \"";
	for(int i = 0; i < (int)assoNb; i++){
		if(i != 0) msg += ", ";
		msg += to_string(association[i]);
	}

	delete association;
    msg += "\", \"lastUpdate\": \"";

    list<ValueID> values = node->m_values;
    string date = getDate(convertDateTime(node->m_sync));
    string time = getTime(convertDateTime(node->m_sync));

    msg += date + " " + time;
    msg += "\", \"valueIds\": [ ";

    for (auto it=values.begin(); it!=values.end(); it++) {
        if (it != values.begin()) {
            msg += ", ";
        }

        msg += valueIdToJson(*it);
    }

    if(node->m_nodeId > 1){
         msg += ", \"nodeNeighbors\": \"";
         for(int i = 0; i < 29; ++i){
             if (i != 0) {
                 msg += ", ";
             }
             msg += to_string(*(node->m_neighbors)[i]);
         }
     }

    msg += " ] }";
    return msg;
}

//Converts the information of a ValueID into json format suitable to send
string Five::valueIdToJson(ValueID valueId) {
    vector<string> prop;
    string msg = "";
    msg += "{ \"id\": \"";
    msg += to_string(valueId.GetId());
    msg += "\", \"readonly\": ";
    msg += to_string(Manager::Get()->IsValueReadOnly(valueId));
    msg += ", \"label\": \"";
    msg += Manager::Get()->GetValueLabel(valueId);
    msg += "\", \"value\": ";

    ValueID::ValueType valType = valueId.GetType();
    bool isNumeric(false);
    string value;

    Manager::Get()->GetValueAsString(valueId, &value);

    for (int i=0; i<(int)(sizeof(NUMERIC_TYPES)/sizeof(int)); i++) {
        if (NUMERIC_TYPES[i] == valType) {
            isNumeric = true;
            break;
        }
    }

    if (valType == ValueID::ValueType_Bool) {
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

    if(valType == ValueID::ValueType_List){
        msg += ", \"options\": [ \"";
        Manager::Get()->GetValueListItems(valueId, &prop);
        for(auto it = prop.begin(); it != prop.end(); it++){
            if(it == prop.begin()){
                msg += (*it) + "\"";
            }else{
                msg += ", \"" + (*it) + "\"";
            }
        }
        msg += " ]";
    }

    msg += " }";

    return msg;
}

string Five::buildNotifMsg(Notification const *notification) {
	cout << "buildNotifMsg 0" << endl;

    string msg = "";
    msg += "\"notificationType\": \"";
    msg += NOTIFICATIONS[(int)notification->GetType()];
    // msg += "\", \"object\": { \"nodeId\": ";
    msg += "\", \"object\": ";
    msg += nodeToJson(getNode(notification->GetNodeId(), nodes));

    msg += " }";

    msg = ", " + msg;
    string header = "{ \"msgLength\": ";
    int msgLength = msg.length() + header.length();

    msg = to_string(msgLength + to_string(msgLength).length()) + msg;
    msg = header + msg;
    return msg;

	cout << "buildNotifMsg 1" << endl;
}

// Read the APP_ENV linux variable and update the global mode.
// **container is the global mode which will be filled.
void Five::getMode(Mode *container) {
	cout << "getMode 0" << endl;

	ifstream f("cpp/examples/cache/config.json");
	json j = {};
	string out = "";

	ifstream t("cpp/examples/cache/config.json");
	stringstream buffer;
	buffer << t.rdbuf();
	
	if (buffer.str().length()) {
		f >> j;
	}

	if (j["mode"] != nullptr) {
		cout << "Mode is not null" << endl;

		unsigned long i;

		for (i = 0; i < sizeof(modes)/sizeof(modes[0]); i++) {
			if (modes[i]->name == j["mode"]["name"]) {
				currentMode = modes[i];
				break;
			}
		}

		for (i = 0; i < sizeof(levels)/sizeof(levels[0]); i++) {
			if (levels[i] == j["mode"]["log"]) break;
		}
		
		switch (i) {
			case 0:
				currentMode->log = Level::ERROR;
				break;
			case 1:
				currentMode->log = Level::NOTICE;
				break;
			case 2:
				currentMode->log = Level::INFO;
				break;
			case 3:
				currentMode->log = Level::DEBUG;
				break;
		}
		
		currentMode->pollInterval = j["mode"]["pollInterval"];
	} else {
		cout << "Mode is null" << endl;
		currentMode = modes[0];
		j["mode"]["name"] = currentMode->name;
		j["mode"]["log"] = levels[currentMode->log];
		j["mode"]["pollInterval"] = currentMode->pollInterval;
	}

	ofstream o("cpp/examples/cache/config.json");
	o << setw(2) << j << endl;

	writeLog(Level::DEBUG, __FILE__, __LINE__, "mode", currentMode->name);
	cout << "getMode 1" << endl;
}

void Five::statusObserver(list<NodeInfo*> *nodes) {
	cout << "statusObserver 0" << endl;
	
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
			cout << "[driver] state = " << STATES[newState] << "\n";
			driverState = newState;
		}

		for (it = nodes->begin(); it != nodes->end(); it++) {
			list<ValueID> valueIDs = (*it)->m_values;

			for (it2 = valueIDs.begin(); it2 != valueIDs.end(); it2++) {
				string label = Manager::Get()->GetValueLabel(*it2);

				if (label.find("Wake-up Interval") != string::npos) { // The object has a sleep intervale.
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

	cout << "statusObserver 1" << endl;
}

void Five::onNotification(Notification const* notification, void* context) {
	cout << "onNotification 0" << endl;

	ofstream myfile;
	NodeInfo* node;
	string valueLabel;
	ValueID valueID(notification->GetValueID());
	uint8 cc_id = valueID.GetCommandClassId();
	Notification::NotificationType nType(notification->GetType());
	string cc_name = Manager::Get()->GetCommandClassName(cc_id);
	// string path = NODE_LOG_PATH + "node_" + to_string(notification->GetNodeId()) + ".log";
	string container;
	string notifType = "";
	string log = "";


	if (nType == Notification::Type_ValueChanged) {
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

	switch (nType) {
		case Notification::Type_ValueAdded:
			writeLog(Level::DEBUG, __FILE__, __LINE__, "valueAdded","node=" + to_string(notification->GetNodeId()) + "&id=" + to_string(valueID.GetId()) + "&label=\"" + Manager::Get()->GetValueLabel(valueID) + '\"');

			for (it = nodes->begin(); it != nodes->end(); it++) {
				if (notification->GetNodeId() == (*it)->m_nodeId) {
					(*it)->m_name = Manager::Get()->GetNodeProductName(homeID, notification->GetNodeId());
				}
			}

			addValue(valueID, getNode(notification->GetNodeId(), Five::nodes));
			break;
		case Notification::Type_ValueRemoved:
			writeLog(Level::DEBUG, __FILE__, __LINE__, "valueRemoved", "node=" + to_string(notification->GetNodeId()) + "&id=" + to_string(valueID.GetId()));
			removeValue(valueID);
			break;
		case Notification::Type_ValueChanged:
			Manager::Get()->GetValueAsString(valueID, &container);
			Manager::Get()->GetNodeStatistics(notification->GetHomeId(), valueID.GetNodeId(), ptr_nodeData);
			valueID = notification->GetValueID();
			writeLog(Level::INFO, __FILE__, __LINE__, "valueChanged", "node=" + to_string(notification->GetNodeId()) + "&id=" + to_string(valueID.GetId()) + "&label=\"" + Manager::Get()->GetValueLabel(valueID) + "\"&value=" + container);
			break;
		case Notification::Type_ValueRefreshed:
			Manager::Get()->GetValueAsString(valueID, &container);
			writeLog(Level::INFO, __FILE__, __LINE__, "valueRefreshed", "node=" + to_string(notification->GetNodeId()) + "&id=" + to_string(valueID.GetId()) + "&label=\"" + Manager::Get()->GetValueLabel(valueID) + "\"&value=" + container);
			break;
		case Notification::Type_NodeAdded:
			pushNode(notification, Five::nodes);
			writeLog(Level::INFO, __FILE__, __LINE__, "nodeAdded", "node=" + to_string(notification->GetNodeId()));
			break;
		case Notification::Type_NodeRemoved:
			writeLog(Level::INFO, __FILE__, __LINE__, "nodeRemoved", "node=" + to_string(notification->GetNodeId()));
			// removeFile(path);
			removeNode(notification, Five::nodes);
			break;
		case Notification::Type_NodeNaming:
			writeLog(Level::DEBUG, __FILE__, __LINE__, "nodeNaming", "node=" + to_string(notification->GetNodeId()));
			break;
		case Notification::Type_DriverReady:
			writeLog(Level::DEBUG, __FILE__, __LINE__, "driver", "READY");
			break;
		case Notification::Type_EssentialNodeQueriesComplete:
			writeLog(Level::DEBUG, __FILE__, __LINE__, "essentialNodeQueriesComplete", "node=" + to_string(notification->GetNodeId()));
			break;
		case Notification::Type_NodeQueriesComplete:
			writeLog(Level::DEBUG, __FILE__, __LINE__, "nodeQueriesComplete", "node=" + to_string(notification->GetNodeId()));
			node = getNode(valueID.GetNodeId(), nodes);
			Manager::Get()->GetNodeNeighbors(homeID, valueID.GetNodeId(), node->m_neighbors);
			break;
		case Notification::Type_AllNodesQueriedSomeDead:
			writeLog(Level::DEBUG, __FILE__, __LINE__, "allNodeQueriedSomeDead", "node=" + to_string(valueID.GetNodeId()));
			break;
		case Notification::Type_AllNodesQueried:
			writeLog(Level::DEBUG, __FILE__, __LINE__, "allNodesQueried", "node=" + to_string(valueID.GetNodeId()));
			break;
		case Notification::Type_ManufacturerSpecificDBReady:
			writeLog(Level::DEBUG, __FILE__, __LINE__, "manufacturerSpecificDBReady", "null");
			break;
		default:
			break;
	}

	if (containsType(nType, Five::AliveNotification) || notification->GetNodeId() == 1) {
		// if ((containsType(nType, Five::AliveNotification) || (nodes->size() == 1 && nType == Notification::Type_AllNodesQueried)) && menuLocked) {
		// 	thread t1(menu);
		// 	t1.detach();
		// 	menuLocked = false;
		// }

		if (containsNodeID(notification->GetNodeId(), (*Five::nodes))) {
			NodeInfo* n = getNode(notification->GetNodeId(), Five::nodes);
			if (n->m_isDead) {
				n->m_isDead = false;
			}
			n->m_sync = chrono::high_resolution_clock::now();
		}
	}

	// vector<Notification::NotificationType> _notifs = {
	// 	Notification::Type_ValueAdded,
	// 	Notification::Type_AllNodesQueried,
	// 	Notification::Type_NodeAdded,
	// 	Notification::Type_ValueChanged,
	// 	Notification::Type_ValueRefreshed,
	// 	Notification::Type_NodeQueriesComplete,
	// };

	// myfile.open(path, ios::app);

	// switch (logs) {
	// 	case Level::DEBUG:
	// 		myfile << "[" << getDate(convertDateTime(getCurrentDatetime())) << ", " << getTime(convertDateTime(getCurrentDatetime())) << "] "
	// 				<< NOTIFICATIONS[nType] << ", " << cc_name << " --> "
	// 				<< to_string(valueID.GetIndex()) << "(" << valueLabel << ")\n";
	// 		break;
	// 	case Level::INFO:
	// 		if (containsType(nType, _notifs)) {
	// 			myfile << "[" << getDate(convertDateTime(getCurrentDatetime())) << ", " << getTime(convertDateTime(getCurrentDatetime())) << "] "
	// 				   << NOTIFICATIONS[nType] << ", " << cc_name << " --> "
	// 				   << to_string(valueID.GetIndex()) << "(" << valueLabel << ")\n";
	// 		}
	// 		break;
	// 	case Level::NOTICE:
	// 		if (nType == Notification::Type_DriverFailed) {
	// 			myfile << "[" << getDate(convertDateTime(getCurrentDatetime())) << ", " << getTime(convertDateTime(getCurrentDatetime())) << "] "
	// 				   << NOTIFICATIONS[nType] << ", " << cc_name << " --> "
	// 				   << to_string(valueID.GetIndex()) << "(" << valueLabel << ")\n";
	// 		}			
	// 		break;
	// 	default:
	// 		break;
	// }

	// myfile.close();

	cout << "onNotification 1" << endl;
	pthread_mutex_unlock(&g_criticalSection); // Unlock the critical section.
}

void Five::watchState(uint32 homeID, int loopTimeOut) {
	cout << "watchState 0" << endl;

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

		writeLog(Level::NOTICE, __FILE__, __LINE__, "driverState", Five::STATES[currentState]);

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
	cout << "watchState 1" << endl;
}

void Five::menu() {
	cout << "menu 0" << endl;

    uint8 *bitmap[29];
	bool menuRun(1);

	while (menuRun)
	{
		string response;
		bool isOk = false;
		list<string>::iterator sIt;
		int choice{0};
		int lock(0);
		int stateInt(0);
		int counterNode{0};
		int counterValue{0};
		list<NodeInfo *>::iterator it;
		list<ValueID>::iterator it2;
		string container;
		string fileName{""};

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

		try
		{
			choice = stoi(response);
		}
		catch (const std::exception &e)
		{
			std::cerr << e.what() << '\n';
		}

		int milliCounter{0};

		switch (choice)
		{
		case 11:
			nodeChoice(&choice, it);

			Manager::Get()->RequestAllConfigParams(homeID, choice);
			break;
		case 10:
			cout << "Choose what node to check: " << endl;

			// Printing node names and receiving user's choice
			for (it = Five::nodes->begin(); it != Five::nodes->end(); it++)
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
			for (it = nodes->begin(); it != nodes->end(); it++)
			{
				cout << "[" << to_string((*it)->m_nodeId) << "] "
					 << (*it)->m_name << "\n";
			}

			cout << "\nSelect a node ('q' to exit): ";
			cin >> response;
			for (it = nodes->begin(); it != nodes->end(); it++)
			{
				if (response == to_string((*it)->m_nodeId))
				{
					while (true)
					{
						cout << "Is awake: ";
						if (Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId))
						{
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

			if (response == "1")
			{
				cout << "\n>>───|PING|───<<\n\n";

				for (it = nodes->begin(); it != nodes->end(); it++)
				{
					cout << "       [ " << to_string((*it)->m_nodeId) << " ]\n";
				}
				cout << "\nChoose ('q' to exit): ";
				cin >> response;

				for (it = nodes->begin(); it != nodes->end(); it++)
				{
					cout << response << " " << to_string((*it)->m_nodeId) << '\n';
					if (response == to_string((*it)->m_nodeId))
					{
						for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++)
						{
							if (Manager::Get()->GetValueLabel(*it2) == "Library Version")
							{
								cout << "Ping sent...\n";
								int counter{60};
								while (counter-- > 0)
								{
									Manager::Get()->RefreshValue(*it2);

									this_thread::sleep_for(chrono::milliseconds(500));
									cout << "ask..." << Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId) << "\n";
								}
								cout << "Done\n";
							}
						}
					}
				}
			}
			else if (response == "5")
			{
				// cout << "Hello\n";
				// cout << nodes->size() <<  "Hello\n";
				// cout << lastNode << "Hello\n";
				// it = nodes->end();
				// uint8 lastNode = (*it)->m_nodeId;

				int nCounter(nodes->size());
				int matrix[nCounter][nCounter];

				for (int i = 0; i < nCounter; i++)
				{
					for (int j = 0; j < nCounter; j++)
					{
						matrix[i][j] = 0;
					}
				}

				// cout << "Setup matrix:\n";
				for (int i = 0; i < nCounter; i++)
				{
					for (int j = 0; j < nCounter; j++)
					{
						// cout << matrix[i][j] << " ";
					}
					// cout << "\n";
				}

				// int nodeIds[nCounter];

				for (int i = 0; i < nCounter; i++)
				{
					it = nodes->begin();
					advance(it, i);
					// nodeIds[i] = (*it)->m_nodeId;
				}

				// cout << "Length: " << to_string(nCounter) << "\n";

				for (int i = 0; i < nCounter; i++)
				{ // Nodes loop.
					it = nodes->begin();
					advance(it, i);
					Manager::Get()->GetNodeNeighbors(homeID, (*it)->m_nodeId, bitmap);
					// cout << "Node " << to_string((*it)->m_nodeId) << "\n";

					if (!(*it)->m_isDead && (*it)->m_nodeId != 1)
					{ // Check if the node is not the controller and can returns its neighbors.
						cout << "\n";
						for (int j = 0; j < 29; j++)
						{ // Neigbors loop.
							cout << to_string((*bitmap)[j]) << " ";
						}
						cout << "\n";

						// cout << "\n";

						// cout << "Neigbors: ";
						for (int j = 0; j < 29; j++)
						{ // Neigbors loop.
							for (int k = 0; k < (int)nodes->size(); k++)
							{
								auto jt = nodes->begin();
								advance(jt, k);
								if ((*jt)->m_nodeId == (*bitmap)[j])
								{
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

				for (int i = 0; i < nCounter; i++)
				{
					for (int j = 0; j < nCounter; j++)
					{
						cout << matrix[i][j] << " ";
					}
					cout << "\n";
				}
			}
			else if (response == "2")
			{
				cout << "\n>>─────|BROADCAST|─────<<\n\n";

				for (it = nodes->begin(); it != nodes->end(); ++it)
				{
					for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++)
					{
						if (Manager::Get()->GetValueLabel(*it2) == "Library Version")
						{
							cout << "Ping sent...\n";
							int counter{60};
							while (counter-- > 0)
							{
								Manager::Get()->RefreshValue(*it2);

								this_thread::sleep_for(chrono::milliseconds(500));
								if (Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId))
								{
									cout << (*it)->m_name << ": OK" << endl;
									break;
								}
								else
								{
									cout << (*it)->m_name << ": not OK" << endl;
								}
							}
						}
					}
				}
			}
			else if (response == "3")
			{
				cout << "\n>>─────|NEIGHBORS|─────<<\n\n";

				for (it = nodes->begin(); it != nodes->end(); it++)
				{
					if ((*it)->m_nodeId != 1 && !(*it)->m_isDead)
					{ // The driver doesn't have neighbors property
						cout << "[" << to_string((*it)->m_nodeId) << "] " << (*it)->m_name << "\n";
					}
				}
				cout << "\nSelect a node ('q' to exit): ";
				cin >> response;
				cout << "\nNeighbor chain: ";
				for (it = nodes->begin(); it != nodes->end(); it++)
				{
					if (to_string((*it)->m_nodeId) == response)
					{
						Manager::Get()->GetNodeNeighbors(homeID, (*it)->m_nodeId, bitmap);
						for (int i = 0; i < 29; i++)
						{
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

						if (response == "1")
						{
							Manager::Get()->SyncronizeNodeNeighbors(homeID, (*it)->m_nodeId);
							cout << "Done.\n";
						}
						else if (response == "2")
						{
							thread ttemp(watchState, homeID, LOOP_TIMEOUT);
							ttemp.detach();

							Manager::Get()->RequestNodeNeighborUpdate(homeID, (*it)->m_nodeId)
								? cout << "Update request sent\n"
								: cout << "Update request failed\n";
						}
						else if (response == "3")
						{
							Manager::Get()->HealNetwork(homeID, true);
						}
						break;
					}
				}
			}
			else if (response == "4")
			{
				cout << "\n>>─────|POLLS|─────<<\n\n"
					 << "/!\\ If you set the poll intensity, you must restart the ZWave key to add modification.\n\n"
					 << "[1] Set intensity\n"
					 << "[2] Set interval for all devices (PLEASE BE CAREFUL)\n\n"
					 << "Choose ('q' to exit): ";

				cin >> response;
				cout << "\n";

				if (response == "1")
				{
					for (it = nodes->begin(); it != nodes->end(); it++)
					{
						cout << "[" << to_string((*it)->m_nodeId) << "] " << (*it)->m_name << "\n";
					}
					cout << "\nSelect a node ('q' to exit): ";
					cin >> response;
					cout << '\n';

					for (it = nodes->begin(); it != nodes->end(); it++)
					{
						if (response == to_string((*it)->m_nodeId))
						{
							for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++)
							{
								cout << "[" << to_string(counterValue++) << "] " << Manager::Get()->GetValueLabel(*it2)
									 << ", " << to_string(Manager::Get()->GetPollIntensity(*it2)) << "\n";
							}

							cout << "\nSelect a valueID (press 'q' to exit): ";
							cin >> response;
							counterValue = 0;

							for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++)
							{
								if (response == to_string(counterValue++))
								{
									cout << "\nValueID \"" << Manager::Get()->GetValueLabel(*it2)
										 << "\", current: " << to_string(Manager::Get()->GetPollIntensity(*it2))
										 << " poll(s)/interval\n";

									cout << "\nSet intensity (0=none, 1=every time through the list, 2-every other time, 'q' to exit): ";
									cin >> response;

									if (response != "q")
									{
										Manager::Get()->SetPollIntensity(*it2, stoi(response));
										cout << "Done\n";
									}
									break;
								}
							}
						}
					}
				}
				else if (response == "2")
				{
					cout << "/!\\ [PLEASE READ] Set the time period between polls of a node's state. Due to patent concerns, some devices do not report state changes automatically to the controller. These devices need to have their state polled at regular intervals. The length of the interval is the same for all devices. To even out the Z-Wave network traffic generated by polling, OpenZWave divides the polling interval by the number of devices that have polling enabled, and polls each in turn. It is recommended that if possible, the interval should not be set shorter than the number of polled devices in seconds (so that the network does not have to cope with more than one poll per second).\n\nCurrent poll interval: " << Manager::Get()->GetPollInterval() << "ms\n\nSet interval in milliseconds ('q' to exit): ";
					cin >> response;

					if (response != "q")
					{
						Manager::Get()->SetPollInterval(stoi(response), false);
						cout << "Done.\n";
					}
				}
			}
			break;
		case 1:
		{
			// Putting the driver into a listening state
			Manager::Get()->AddNode(Five::homeID, false);

			// Printing progression messages
			thread t3(nodeSwitch, stateInt, &lock);
			t3.detach();
			stateInt = Manager::Get()->GetDriverState(Five::homeID);

			break;
		}
		case 2:
		{
			// Putting the driver into a listening state
			Manager::Get()->RemoveNode(Five::homeID);

			// Printing progression messages
			//  thread t3(statusObserver, nodes);
			//  t3.join();

			// thread t3(nodeSwitch, stateInt, &lock);
			// t3.detach();
			// stateInt = Manager::Get()->GetDriverState(Five::homeID);

			break;
		}
		case 3:
			// Printing node names and receiving user's choice
			nodeChoice(&choice, it);

			if (choice == -1)
			{ // Impossible value except if user chooses to quit
				break;
			}

			// Printing all the node's values with name, id and ReadOnly state
			printValues(&choice, &it, it2, true);
			break;
		case 4:
			// Printing node names and receiving user's choice

			nodeChoice(&choice, it);

			if (choice == -1)
			{ // Impossible value except if user chooses to quit
				break;
			}
			// counterNode = 0;

			// Printing value names and receiving user's choice
			for (it = Five::nodes->begin(); it != Five::nodes->end(); it++)
			{
				counterNode++;
				if ((*it)->m_nodeId == choice)
				{
					for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++)
					{
						if (!Manager::Get()->IsValueReadOnly(*it2))
						{
							counterValue++;
							cout << counterValue << ". " << Manager::Get()->GetValueLabel(*it2) << endl;
						}
					}

					cout << "\nChoose a valueID: ";
					cin >> response;
					counterValue = 0;
					choice = stoi(response);

					for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++)
					{
						// Manager::Get()->GetValueAsString(*valueIt, &ptr_container);
						// cout << Manager::Get()->GetValueLabel(*valueIt) << ": " <<  &ptr_container << endl;
						if (!Manager::Get()->IsValueReadOnly(*it2))
						{
							counterValue++;
						}

						if (choice == counterValue)
						{

							// Printing the current value
							cout << Manager::Get()->GetValueLabel(*it2) << it2->GetAsString() << endl;
							Manager::Get()->GetValueAsString((*it2), &container);
							cout << "Current value: " <<  container << endl;

							// Asking the user to set the wanted value
							cout << "Set to what ? ";
							cin >> response;
							int tempCounter{100};
							// int test = 0;
							// int* testptr = &test;
							// setUnit((*valueIt));

							// Sending the value until current value is identical, or until timeout (for sleeping nodes)
							while (response != container && tempCounter-- > 0)
							{
								// if (response.find('.')) {
								// 	float temp = stof(response);
								// 	Manager::Get()->SetValue(*it2, temp);
								// } else {
								// }

								Manager::Get()->SetValue((*it2), response);
								Manager::Get()->GetValueAsString((*it2), &container);
								this_thread::sleep_for(chrono::milliseconds(100));
								cout << "send " << response << ", " <<  container << "\n";
							}

							// Manager::Get()->GetValueAsInt((*valueIt), testptr);
							// cout << *testptr;
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
			// 			Manager::Get()->GetValueAsString(*valueIt, &ptr_container);
			// 			cout << "The current value is: " << &ptr_container << endl;
			// 			cout << "Enter the new value: " << endl;
			// 			cin >> response;
			// 			Manager::Get()->SetValue(*valueIt, response);
			// 		}
			// }

			break;
		case 5:
			cout << "Choose between: " << endl;
			cout << "1. Hard Reset (Z-Wave Network will be deleted during reset)\n"
				 << "2. Soft Reset (Z-Wave Network will be kept during reset)\n";
			while (!isOk)
			{
				cin >> response;
				choice = stoi(response);
				if (choice == 1)
				{
					Manager::Get()->ResetController(Five::homeID);
					isOk = true;
				}
				else if (choice == 2)
				{
					Manager::Get()->SoftReset(Five::homeID);
					isOk = true;
				}
				else
				{
					cout << "Please enter 1 or 2\n";
				}
			}
			menuRun = 0;
			break;
		case 6:
			break;
		case 7:
			// Printing node names and receiving user's choice
			nodeChoice(&choice, it);

			// Ask the node to recheck his neighboors
			for (it = Five::nodes->begin(); it != Five::nodes->end(); it++)
			{
				if ((*it)->m_nodeId == choice)
				{
					Manager::Get()->HealNetworkNode((*it)->m_homeId, (*it)->m_nodeId, true);
				}
			}
			break;
		case 8:
			// Printing node names and receiving user's choice
			nodeChoice(&choice, it);
			// counterNode = 0;
			if (choice == -1)
			{ // Impossible value except if user chooses to quit
				break;
			}

			// Printing value names and receiving user's choice
			printValues(&choice, &it, it2, false);

			if (choice == -1)
			{ // Impossible value except if user chooses to quit
				break;
			}

			// Calling appropriate method depending on user's choice
			newSetValue(&choice, &it, it2, isOk);
			break;
		default:
			cout << "You must enter 1, 2, 3 or 4." << endl;
			break;
		}

		if (fileName.size() > 0)
		{
			char arr[fileName.length()];
			strcpy(arr, fileName.c_str());
			for (int i = 0; i < int(fileName.length()); i++)
			{
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

	cout << "menu 1" << endl;
}

// Function printing progression messages during Add Node and Remove Node
void Five::nodeSwitch(int stateInt, int *lock) {
	cout << "nodeSwitch 0" << endl;

	int counter(500);

	while (counter-- > 0) {
		switch (stateInt)
		{
		case 1:
			if (*lock != stateInt)
			{
				cout << "STARTING" << endl;
			}
			*lock = 1;
			break;
		case 4:
			if (*lock != stateInt)
			{
				cout << "WAITING" << endl;
			}
			*lock = 4;
			break;
		case 7:
			if (*lock != stateInt)
			{
				cout << "COMPLETED" << endl;
			}
			*lock = 7;
			break;
		default:
			break;
		}
		this_thread::sleep_for(chrono::milliseconds(20));
	}

	cout << "nodeSwitch 1" << endl;
}

// Function looping in the background to check if nodes are failed
void Five::CheckFailedNode(string path) {
	cout << "checkFailedNode 0" << endl;

	while (true) {
		list<NodeInfo *>::iterator it;
		fstream file;
		bool isIn(0);
		this_thread::sleep_for(chrono::seconds(FAILED_NODE_INTERVAL));

		for (it = n.begin(); it != n.end(); it++)
		{
			// cout << "in node for" << endl;
			uint8 nodeId = (*it)->m_nodeId;
			string line;
			string nodeName = (*it)->m_name;
			string nodeType = (*it)->m_nodeType;
			if (Manager::Get()->IsNodeFailed(homeID, nodeId))
			{
				file.open(path, ios::in);
				while (getline(file, line))
				{
					if (line.find("Label: " + nodeName) != string::npos)
					{
						isIn = 1;
					}
				}
				file.close();
				// cout << "NODE HAS FAILED: " << nodeName << " id: " << unsigned(nodeId) << endl;
				if (!isIn)
				{
					file.open(path, ios::app);
					file << "Label: " << nodeName << " Id: " << unsigned(nodeId) << " Type: " << nodeType << endl;
					file.close();
				}
			}
			else if (!(Manager::Get()->IsNodeFailed(homeID, nodeId)))
			{
				// cout << "node not failed" << "id: " << unsigned(nodeId) << endl;
				file.open(path, ios::in);
				fstream temp;
				temp.open("temp.txt", ios::app);
				while (getline(file, line))
				{
					cout << line << endl;
					string s = "Label: " + nodeName;
					// cout << s << endl;
					if (line.find(s) == string::npos)
					{
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

	cout << "checkFailedNode 1" << endl;
}

void Five::writeLog(Level level, string file, int line, string key, string value) {
	cout << "writeLog 0" << endl;

	if (level == currentMode->log || currentMode->log == Level::DEBUG) {
		auto dateTime = convertDateTime(getCurrentDatetime());
		string log = "[" + getDate(dateTime) 
					+ "," + getTime(dateTime) + "]"
					+ levels[level] + ";"
					+ file + ":" + to_string(line) + ";"
					+ key + ":" + value + '\n';
		fstream f;
		f.open(JOURNAL, ios::app);
		f << log;
		f.close();
	}

	cout << "writeLog 1" << endl;
}

void Five::getDriver(string *driverPath) {
	const char *cmd02 = "ls -l /dev/ttyACM* | awk {'print($10)'} > cpp/examples/cache/.driver";
	cout << system(cmd02) << endl;
	writeLog(Level::INFO, __FILE__, __LINE__, "bash", (string)cmd02);

	fstream my_file;
	my_file.open("cpp/examples/cache/.driver");
	if (my_file.is_open()) {
		while(my_file.good()) {
			my_file >> *driverPath;
		}
	}
	my_file.close();

	const char *cmd01 = "rm cpp/examples/cache/.driver";
	cout << system(cmd01);
	writeLog(Level::INFO, __FILE__, __LINE__, "bash", (string)cmd01);
}