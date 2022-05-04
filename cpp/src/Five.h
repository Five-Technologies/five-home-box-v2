#ifndef _FIVE_H
#define _FIVE_H

#include "Manager.h"
#include "Notification.h"
#include <chrono>
#include <netinet/in.h>
using namespace OpenZWave;

namespace Five {

    pthread_mutex_t g_criticalSection;
    pthread_cond_t initCond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t initMutex;


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
        uint8* m_neighbors[29];
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

    enum StatusCode {
        VALID_ok=200,
        VALID_created=201,
        VALID_accepted=202,
        VALID_noContent=204,
        INVALID_badRequest=400,
        INVALID_unauthorized=401,
        INVALID_forbidden=403,
        INVALID_notFound=404,
        INVALID_methodNotAllowed=405,
        INVALID_notAcceptable=406,
        INVALID_requestTimeout=408,
        INVALID_imATeapot=418,
        SERVER_notImplemented=501,
        SERVER_unavailable=503,
        SERVER_bandWidthLimitExceeded=509,
    };

    enum Message { 
        ArgumentError, 
        ValueTypeError, 
        ValueNotFoundError, 
        NodeNotFoundError, 
        InvalidArgument,
        None,
        InvalidCommand,
        ArgumentWrongType
    };

    const string messages[] = {
        "ArgumentError",
        "ValueTypeError",
        "ValueNotFoundError",
        "NodeNotFoundError",
        "InvalidArgument",
        "",
        "InvalidCommand",
        "ArgumentWrongType"
    };

    struct Command {
        string name;
        vector<string> arguments;
        string description;
    };

    list<NodeInfo*> n{};
    list<NodeInfo*>* nodes = &n;
    uint32 homeID{ 0 }; // Hexadecimal
    logLevel LEVEL;
    Driver::ControllerState driverState;

    const list<string> TYPES{ "Color", "Switch", "Level", "Duration", "Volume", "Wake-up" };
    const string CACHE_PATH{ "cpp/examples/cache/" };
    const string NODE_LOG_PATH{ CACHE_PATH + "nodes/" };
    const string SOCKET_LOG_PATH{ CACHE_PATH + "socket/"};
    const string FAILED_NODE_PATH{ CACHE_PATH + "failed_nodes.log" };
    const string CPP_PATH{ "cpp/" };
    const string CONFIG_PATH{ "config/" };
    
    string DRIVER_PATH;
    bool menuLocked = true;

    const int FAILED_NODE_INTERVAL{ 20 }; // Seconds
    const int NEIGHBOR_BITMAP_LENGTH{ 29 }; // Bits
    const int OBSERVER_PERIOD{ 50 }; // Milliseconds
    const int STATE_PERIOD{ 100 }; // Milliseconds
    const int LOOP_TIMEOUT{ 100 }; // Loop counter

    const Command COMMANDS[] = {
        Command{"setValue", {"id", "newValue"}, "Send the value on to update the specific option."},
        Command{"include", {}, "Set the driver in inclusion mode."},
        Command{"exclude", {}, "Set the driver in exclusion mode."},
        Command{"getNode", {"id"}, "Get all node information."},
        Command{"reset", {"level"}, "Soft/Hard reset the driver."},
        Command{"heal", {"(nodeIdd)"}, "Heal the node id if specified, otherwise heal the hole network."},
        Command{"isFailed", {"nodeId"}, "Check if the node is able to return a response."},
        Command{"ping", {}, "No description"},
        Command{"help", {}, "Command list documentation."},
        Command{"brdcast", {}, "Pings every node to see how many respond"},
        Command{"_restart", {}, "Restart the process with Bash."},
        Command{"_reset", {}, "Remove log files, reset the ZWave driver and restart the process with Bash."},
        Command{"_setLvl", {"level"}, "[NONE, WARNING, INFO, DEBUG] Restart the ZWave driver with the selected level with Bash."},
        Command{"map", {}, "Returns the Neighbors Map as response"},

    };
    
    const ValueID::ValueType NUMERIC_TYPES[] = {
        ValueID::ValueType::ValueType_Bool,
        ValueID::ValueType::ValueType_Byte,
        ValueID::ValueType::ValueType_Decimal,
        ValueID::ValueType::ValueType_Int,
        ValueID::ValueType::ValueType_Short,
    };

    const string STATES[]{
        "Normal", "Starting", "Cancel", "Normal",
        "Waiting", "Sleeping", "InProgress", "Completed",
        "Failed", "NodeOK", "NodeFailed"
    };

    const vector<Driver::ControllerState> ADD_RM_STATES = {Driver::ControllerState_Starting, Driver::ControllerState_Waiting, Driver::ControllerState_InProgress};

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

    void CheckFailedNode(string path);
    void nodeSwitch(int stateInt, int *lock);
    void menu();
    void onNotification(Notification const* notification, void* context);
    void watchState(uint32 homeID, int loopTimeOut);
    void statusObserver(list<NodeInfo*> *nodes);

    // JSON Serializer

    string nodeToJson(NodeInfo* node);
    string valueIdToJson(ValueID valueId);

    // Config method
    
    bool setSwitch(ValueID valueID, string answer);
    bool setIntensity(ValueID valueID, int intensity);
    bool setHexColor(ValueID valueID, string hexColor);
    bool setList(ValueID valueID);
    bool setVolume(ValueID valueID, int intensity);
    bool setDuration(ValueID valueID, int duration);
    bool setInt(ValueID valueId);
    bool setBool(ValueID valueId);
    bool setButton(ValueID valueId, string input);

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
    bool containsControllerType(Driver::ControllerState needle, vector<Driver::ControllerState> haystack);

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
    string buildNotifMsg(Notification const *notification);
    bool containsStatus(StatusCode needle, vector<StatusCode> haystack);

    // Client
    
    string buildPhpMsg(string commandName, vector<string> args);

    // Unit Tests

    bool UT_isInt(string arg);
    bool UT_isValueIdExists(string id, ValueID* ptr_valueID);
    bool UT_isNodeIdExists(string id);
    bool UT_isDecimal(string arg);
    bool UT_isBoolean(string arg);
    bool UT_isButton(string arg);

    auto startedAt = getCurrentDatetime();
}

#endif