#include "Five.h"
#include "Notification.h"
#include "Manager.h"
#include "command_classes/CommandClass.h"

using namespace Five;
using namespace OpenZWave;
using namespace std;

NodeInfo getNodeInfo(Notification const* notification, list<NodeInfo*> nodes) {
    uint32 homeId{ notification->GetHomeId() };
    uint8 nodeId{ notification->GetNodeId() };
    string nodeType{ Manager::Get()->GetNodeType(homeId, nodeId) };
    string nodeName{ Manager::Get()->GetNodeName(homeId, nodeId) };
    
    NodeInfo node = {
        homeId,
        nodeId,
        { notification->GetValueID() },
        nodeName,
        nodeType
    };

    return node;
}

NodeInfo nodeConfig(uint32 homeId, uint8 nodeId, list<NodeInfo*> g_nodes)
{
    list<NodeInfo*>::iterator it;
    NodeInfo node;
    
    node.m_homeId = homeId;
    node.m_name = Manager::Get()->GetNodeProductName(node.m_homeId, nodeId);
    node.m_nodeId = nodeId;
    node.m_nodeType = Manager::Get()->GetNodeType(node.m_homeId, nodeId);
    for(it = g_nodes.begin(); it != g_nodes.end(); ++it)
    {
        if((*it)->m_nodeId == nodeId)
        {
            node.m_values = (*it)->m_values;
        }
    }
    return node;
    
}
string NotificationService::valueAdded(Notification const* notification, list<NodeInfo*> nodes) {
    return "[VALUE ADDED] " + notification->GetValueID().GetAsString() + '\n';
}

string NotificationService::valueRefreshed(Notification const* notification, list<NodeInfo*> nodes) {
    return "value refreshed";
}

string NotificationService::valueRemoved(Notification const* notification, list<NodeInfo*> nodes) {
    return "value removed";
}

/// A value has changed on the Z-Wave network and this is a different value.
string NotificationService::valueChanged(Notification const* notification, list<NodeInfo*> nodes) {
    bool state;
    bool* ptr = &state;
    
    ValueID v{ notification->GetValueID() };
    string output{ "[VALUE CHANGED]\n" };
    list<NodeInfo*>::iterator it;

    for (it = nodes.begin(); it != nodes.end(); it++) {
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

    return output;
}

bool Five::setSwitch(ValueID valueId, bool state)
{   
    if(state)
    {
        cout << "if" << endl;
        Manager::Get()->SetValue(valueId, state);
    }
    else
    {
        cout << "else" << endl;
        Manager::Get()->SetValue(valueId, state);
    }
    return true;
}



bool Five::setIntensity(ValueID valueId, IntensityScale intensity) {
    Manager::Get()->SetValue(valueId, to_string(intensity));
    return true;
}

bool Five::setColor(ValueID valueId)
{
    list<string> clist = {"red", "green", "blue", "yellow", "purple", "cyan"};
    list<string>::iterator it;
    int count{0};
    for (it = clist.begin(); it != clist.end(); ++it)
    {
        count++;
        cout << count << ". " << (*it) << endl;
    }
    cout << "Choose a color:" << endl;
    string answer;
    int choice;
    cin >> answer;
    choice = stoi(answer);
    string hexColor;
    if(choice == 1){
        hexColor = "#FF00000000";
    }else if(choice == 2){
        hexColor = "#00FF000000";
    }else if(choice == 3){
        hexColor = "#0000FF0000";
    }else if(choice == 4){
        hexColor = "#FFFF000000";
    }else if(choice == 5){
        hexColor = "#FF00FF0000";
    }else if(choice == 6){
        hexColor = "#00FFFF0000";
    }
    Manager::Get()->SetValue(valueId, hexColor);
    return true;
}

bool Five::setList(ValueID valueId){
    vector<string> vectS;
    vector<string> *vectSPtr = &vectS;
    int counter{0};
    string s;
    string* ptr = &s;
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

bool setVolume(ValueID valueId, IntensityScale intensity){
    Manager::Get()->SetValue(valueId, to_string(intensity));
    return true;
}

bool setDuration(ValueID valueId){
    string response;
    cout << "Please enter a duration in seconds:" << endl;
    cin >> response;
    Manager::Get()->SetValue(valueId, response);
    return true;
}


