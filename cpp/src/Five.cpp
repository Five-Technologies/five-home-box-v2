#include "Five.h"
#include "Notification.h"
#include "Manager.h"

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
