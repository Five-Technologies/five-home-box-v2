#include <iostream>
#include "Manager.h"
#include "Options.h"
#include "Driver.h"
#include <thread>
#include <fstream>
#include "Five.h"

#include "../../../json/single_include/nlohmann/json.hpp"

using namespace OpenZWave;
using namespace Five;
using namespace std;
using namespace nlohmann;

bool g_menuLocked{true};
bool g_checkLocked(1);

int main(int argc, char const *argv[]) {
	fstream my_file;
	string response;

	// my_file.open("cpp/examples/cache/configs/test.yaml");
	// if (my_file.is_open()) {
	// 	string output = "";
	// 	while (my_file.good()) {
	// 		my_file >> output;
	// 	}
	// }
	// my_file.close();

	// auto j = json::parse(output);
	// cout << j.dump(4) << endl;

	Five::startedAt = getCurrentDatetime();
	
	pthread_mutexattr_t mutexattr;

	// Initialize the mutex ATTRIBUTES with the default value.
	pthread_mutexattr_init(&mutexattr);

	/* PTHREAD_MUTEX_NORMAL: it does not detect dead lock.
	   - If you want lock a locked mutex, you will get a deadlock mutex.
	   - If you want unlock an unlocked mutex, you will get an undefined behavior.

	   PTHREAD_MUTEX_ERRORCHECK: it returns an error if you you want lock a locked mutex or unlock an unlocked mutex.

	   PTHREAD_MUTEX_RECURSIVE:
	   - If you want lock a locked mutex, it stays at locked state an returns a success.
	   - If you want unlock an unlocked mutex, it stays at unlocked state an returns a success.

	   PTHREAD_MUTEX_DEFAULT: if you lock a locked mutex or unlock an unlocked mutex, you will get an undefined behavior.
	*/
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);

	/* Initialize the mutex referenced by mutex with attributes specified by attr.
	   If the attribute is NULL, the default mutex attributes are used.
	   Upon initialization is successful, the mutex becomes INITIALIZED and UNLOCKED.

	   /!\ Initialization the same mutex twice will get an undefined behavior. /!\
	*/
	pthread_mutex_init(&g_criticalSection, &mutexattr);

	// You only can use the mutex ATTR one time, it does not affect any other mutexes and you should detroy it to uninitialize it.
	pthread_mutexattr_destroy(&mutexattr);

	pthread_mutex_lock(&initMutex);

	Options::Create(CONFIG_PATH, CACHE_PATH, "");
	Options::Get()->Lock();
	Manager::Create();
	Manager::Get()->AddWatcher(onNotification, NULL);

	Five::getMode(currentMode);
	writeLog(Level::INFO, __FILE__, __LINE__, "mode", currentMode->name);
	Five::getDriver(&driverPath);
	writeLog(Level::DEBUG, __FILE__, __LINE__, "driver", driverPath);

	json j;
	j["driver"] = driverPath;
	j["mode"]["name"] = currentMode->name;
	j["mode"]["log"] = levels[currentMode->log];
	j["mode"]["pollInterval"] = currentMode->pollInterval;
	ofstream o("cpp/examples/cache/config.json");
	cout << o.is_open() << endl;
	o << setw(2) << j << endl;


	Manager::Get()->AddDriver(driverPath);

	thread t3(Five::statusObserver, nodes);
	thread t4(server, ZWAVE_PORT);
	// thread t2(Five::CheckFailedNode, FAILED_NODE_PATH);
	
	t3.detach();
	t4.detach();
	// t2.detach();
	
	pthread_cond_wait(&initCond, &initMutex);
	pthread_mutex_unlock(&g_criticalSection);
	
	return EXIT_SUCCESS;
}
