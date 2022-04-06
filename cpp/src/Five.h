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
    
    bool SetSwitch(ValueID valueID, bool state);
    bool SetIntensity(ValueID valueID, IntensityScale intensity);
    bool SetColor(ValueID valueID);
    bool SetList(ValueID valueID);
    bool SetVolume(ValueID valueID, IntensityScale intensity);
    bool SetDuration(ValueID valueID);

    // Node methods
    
    bool IsNodeAlive(Notification notif, list<NodeInfo*> *nodes, vector<Notification::NotificationType> aliveNotifications);
    bool IsNodeNew(uint8 nodeID, list<NodeInfo*> *nodes);
    int DeadNodeSum(list<NodeInfo*> *nodes);
    int AliveNodeSum(list<NodeInfo*> *nodes);
    void RefreshNode(ValueID valueID, NodeInfo *oldNodeInfo);
    bool ContainsNodeID(uint8 needle, list<NodeInfo*> haystack);
    
    void PushNode(Notification const *notification, list<NodeInfo*> *nodes);
    void RemoveNode(Notification const *notification, list<NodeInfo*> *nodes);

    NodeInfo* CreateNode(Notification const* notification);
    NodeInfo* GetNode(uint8 nodeID, list<NodeInfo*> *nodes);
    NodeInfo GetNodeConfig(uint32 homeID, uint8 nodeID, list<NodeInfo *> *nodes);
    
    // Notification methods
    
    bool ValueAdded(Notification const *notification, list<NodeInfo *> *nodes);
    bool ValueRemoved(Notification const *notification, list<NodeInfo *> *nodes);
    bool ValueChanged(Notification const *notification, list<NodeInfo *> *nodes);
    bool ValueRefreshed(Notification const *notification, list<NodeInfo *> *nodes);
    bool ContainsType(Notification::NotificationType needle, vector<Notification::NotificationType> haystack);

    // Value methods
    
    bool RemoveValue(ValueID valueID);
    bool AddValue(ValueID valueID, NodeInfo *node);

    //File methods
    
    bool RemoveFile(string path);
    void Stoc(string chain, char *output);

    // Driver methods
    
    string GetDriverData(uint32 homeID);

    // Time methods
    
    chrono::high_resolution_clock::time_point GetCurrentDatetime();
    tm* ConvertDateTime(chrono::high_resolution_clock::time_point datetime);
    string GetTime(tm *datetime);
    string GetDate(tm *datetime);
    double Difference(chrono::high_resolution_clock::time_point datetime01, chrono::high_resolution_clock::time_point datetime02);
}

#endif