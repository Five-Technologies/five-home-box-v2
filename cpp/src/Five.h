#ifndef _FIVE_H
#define _FIVE_H

#include "Manager.h"
#include "Notification.h"
#include <chrono>
#include <netinet/in.h>
using namespace OpenZWave;


namespace Five {

    const int ZWAVE_PORT = 5101;
    const int PHP_PORT = 5100;
    const char *LOCAL_ADDRESS = "127.0.0.1";
    
    struct NodeInfo {
        uint32             m_homeId;
        uint8              m_nodeId;
        list<ValueID>      m_values;
        string             m_name;
        string             m_nodeType;
        chrono::high_resolution_clock::time_point m_sync;
        bool               m_isDead=true;
    };

    const vector<Notification::NotificationType> AliveNotification{
        Notification::Type_ValueChanged,
	    Notification::Type_ValueRefreshed,
        Notification::Type_AllNodesQueried
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

    list<NodeInfo*> n{};
    list<NodeInfo*>* nodes = &n;
    const list<string> TYPES{ "Color", "Switch", "Level", "Duration", "Volume", "Wake-up" };
    const string CACHE_PATH{ "cpp/examples/cache/" };
    const string NODE_LOG_PATH{ CACHE_PATH + "nodes/" };
    const string FAILED_NODE_PATH{ CACHE_PATH + "failed_nodes.log" };
    const string CPP_PATH{ "cpp/" };
    const string CONFIG_PATH{ "config/" };
    const string PORT{ "/dev/ttyACM0" };
    const int failedNodeInterval{ 20 }; // Seconds
    uint32 homeID{ 0 }; // Hexadecimal
    const int NEIGHBOR_BITMAP_LENGTH{ 29 }; // Bits
    const int OBSERVER_PERIOD{ 50 }; // Milliseconds
    const int STATE_PERIOD{ 100 }; // Milliseconds
    const int LOOP_TIMEOUT{ 100 }; // Loop counter
    logLevel LEVEL;
    Driver::ControllerState driverState;
    
    const ValueID::ValueType NUMERIC_TYPES[] = {
        ValueID::ValueType::ValueType_Bool,
        ValueID::ValueType::ValueType_Byte,
        ValueID::ValueType::ValueType_Decimal,
        ValueID::ValueType::ValueType_Int,
        ValueID::ValueType::ValueType_Short,
    };

    const string STATES[]{
        "Normal", "Starting", "Cancel", "Error",
        "Waiting", "Sleeping", "InProgress", "Completed",
        "Failed", "NodeOK", "NodeFailed"
    };

    const string NOTIFICATIONS[] {
        "VALUED_ADDED", "VALUE_REMOVED", "VALUE_CHANGED", "VALUE_REFRESHED", "GROUP",
        "NODE_NEW", "NODE_ADDED", "NODE_REMOVED", "NODE_PROTOCOL_INFO", "NODE_NAMING",
        "NODE_EVENT", "POLLING_DISABLED", "POLLING_ENABLED", "SCENE_EVENT", "CREATE_BUTTON",
        "DELETE_BUTTON", "BUTTON_ON", "BUTTON_OFF", "DRIVER_READY", "DRIVER_FAILED",
        "DRIVER_RESET", "ESSENTIAL_NODE_QUERIES_COMPLETE", "NODE_QUERIES_COMPLETE",
        "AWAKE_NODE_QUERIED", "ALL_NODES_QUERIED_SOME_DEAD", "ALL_NODES_QUERIED",
        "NOTIFICATION", "DRIVER_REMOVED", "CONTROLLER_COMMAND", "NODE_RESET",
        "USER_ALERTS", "MANUFACTURER_SPECIFIC_DB_READY"
    };

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

    bool nodeChoice(int* choice, list<NodeInfo*>::iterator it);
    
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
    bool printValues(int* choice, list<NodeInfo*>::iterator* it, list<ValueID>::iterator it2, bool getOnly);
    bool newSetValue(int* choice, list<NodeInfo*>::iterator* it, list<ValueID>::iterator it2, bool isOk);

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

    // Server

    string convertToString(char* a, int size);
    int sendMsg(const char* address, const int port, string message);
    string receiveMsg(sockaddr_in address, int server_fd);
    void server(int port);
}

#endif