#ifndef _FIVE_H
#define _FIVE_H

#include "Manager.h"
#include "Notification.h"
#include <chrono>
using namespace OpenZWave;

namespace Five
{
    struct NodeInfo {
        uint32        m_homeId;
        uint8         m_nodeId;
        list<ValueID> m_values;
        string        m_name;
        string        m_nodeType;
        time_t        m_sync;
        bool          m_isDead=true;
    };

    const vector<Notification::NotificationType> AliveNotification{
        Notification::Type_ValueChanged,
	    Notification::Type_ValueRefreshed
    };

    enum IntensityScale {
        VERY_HIGH=99,
        HIGH=30,
        MEDIUM=20,
        LOW=10,
        VERY_LOW=2,
        OFF=0
    };

    enum logLevel {
        NONE,
        WARNING,
        INFO,
        DEBUG
    };

    list<NodeInfo*> n;
    list<NodeInfo*>* nodes = &n;
    const list<string> TYPES{ "Color", "Switch", "Level", "Duration", "Volume" };
    const string CACHE_PATH{ "cpp/examples/cache/" };
    const string NODE_LOG_PATH{ "cpp/examples/cache/nodes/" };
    const string CPP_PATH{ "cpp/" };
    const string CONFIG_PATH{ "config/" };
    const string PORT{ "/dev/ttyACM0" };
    uint32 homeID{ 0 };
    logLevel LEVEL;

    // Config method
    
    bool setSwitch(ValueID valueID, bool state);
    bool setIntensity(ValueID valueID, IntensityScale intensity);
    bool setColor(ValueID valueID);
    bool setList(ValueID valueID);
    bool setVolume(ValueID valueID, IntensityScale intensity);
    bool setDuration(ValueID valueID);
    bool setInt(ValueID valueId);
    bool setBool(ValueID valueId);
    bool setButton(ValueID valueId);

    // Node methods
    
    bool isNodeAlive(Notification notif, list<NodeInfo*> *nodes, vector<Notification::NotificationType> aliveNotifications);
    bool isNodeNew(uint8 nodeID, list<NodeInfo*> *nodes);
    int deadNodeSum(list<NodeInfo*> *nodes);
    int aliveNodeSum(list<NodeInfo*> *nodes);
    void refreshNode(ValueID valueID, NodeInfo *oldNodeInfo);
    bool containsNodeID(uint8 needle, list<NodeInfo*> haystack);
    
    void pushNode(Notification const *notification, list<NodeInfo*> *nodes);
    void removeNode(Notification const *notification, list<NodeInfo*> *nodes);

    NodeInfo* createNode(Notification const* notification);
    NodeInfo* getNode(uint8 nodeID, list<NodeInfo*> *nodes);
    NodeInfo getNodeConfig(uint32 homeID, uint8 nodeID, list<NodeInfo *> *nodes);
    
    // Notification methods
    
    bool valueAdded(Notification const *notification, list<NodeInfo *> *nodes);
    bool valueRemoved(Notification const *notification, list<NodeInfo *> *nodes);
    bool valueChanged(Notification const *notification, list<NodeInfo *> *nodes);
    bool valueRefreshed(Notification const *notification, list<NodeInfo *> *nodes);
    bool containsType(Notification::NotificationType needle, vector<Notification::NotificationType> haystack);

    // Value methods
    
    bool removeValue(ValueID valueID);
    bool addValue(ValueID valueID, NodeInfo *node);

    //File methods
    
    bool removeFile(string path);
    void stoc(string chain, char *output);

    // Driver methods
    
    string getDriverData(uint32 homeID);

    // Time methods
    
    chrono::high_resolution_clock::time_point getCurrentDatetime();
    tm* convertDateTime(chrono::high_resolution_clock::time_point datetime);
    string getTime(tm *datetime);
    string getDate(tm *datetime);
    double difference(chrono::high_resolution_clock::time_point datetime01, chrono::high_resolution_clock::time_point datetime02);
}

#endif