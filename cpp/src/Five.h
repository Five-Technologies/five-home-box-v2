#ifndef _FIVE_H
#define _FIVE_H

#include "Manager.h"
using namespace OpenZWave;

namespace Five {
    typedef struct {
        uint32			m_homeId;
        uint8			m_nodeId;
        list<ValueID>	m_values;
        string			m_name;
        string			m_nodeType;
    } NodeInfo;

    uint32 homeId;
    list<NodeInfo*> nodes{};
    
    NodeInfo getNodeInfo(Notification const* notification, list<NodeInfo*> nodes);
    bool isAlive(uint8 nodeId);

    bool setState(ValueID valueId, bool state);
    bool setIntensity(ValueID valueId, double intensity);
    bool setColor(ValueID valueId, int hexColor);
    bool setName(ValueID, string name);

    class NotificationService {
        public:
            static string valueAdded(Notification const* notification, list<NodeInfo*> nodes);
            static string valueRemoved(Notification const* notification, list<NodeInfo*> nodes);
            static string valueChanged(Notification const* notification, list<NodeInfo*> nodes);
            static string valueRefreshed(Notification const* notification, list<NodeInfo*> nodes);
    };
}

#endif