# Five Home Zwave Library

<p align="center">
<img src="https://five-tech.be/wp-content/uploads/2019/05/Five-Home-Picto-5@2x.png" alt="drawing" width="250"/>

## Introduction

This project implements [OpenZWave](https://github.com/OpenZWave/open-zwave) library which manages the ZWave protocol. The main goal is to provide an API to store ZWave object data and change some object status.

## Installation
You only have to execute the [install.sh](https://git.five-tech.be/fivegituser/five-home-box-v2/src/branch/develop/zwave/install.sh) file.

```bash
sh install.sh
```

Once the installaction gets completed, execute the following command.
```bash
rc-service status -a   # You're supposed to see the service "minozw" running.
```

✅ It's easy to run `install.sh` :
- Stop the zwave server (if running)
- Kill all garbage processes
- Reset the driver
- Remove log files
- Start the server

However, if you want stop/start/restart process, you also can use these commands.  

```bash
rc-service minozw stop     # Stop the service.
rc-service minozw start    # Start the service.
rc-service minozw restart  # Restart the service.
```
⚠️ If you're using `stop` or `restart` command, you must check if there is no more process running in background.
```bash
>> ps
41593 root      0:00 /bin/sh
47920 root      0:00 supervise-daemon minozw --start sh -- -c cd /srv/app/zwave && ./MinOZW
47922 root      0:00 {MinOZW} /bin/sh ./MinOZW
47926 root      0:02 .lib/MinOZW
48691 root      0:00 ps
>> rc-service minozw stop           # or restart
...
>> ps
41593 root      0:00 /bin/sh
47926 root      0:02 .lib/MinOZW    # you still have a process in background
48797 root      0:00 ps
>> kill 47926
>> ps
41593 root      0:00 /bin/sh
48836 root      0:00 ps
```
You can see that a process was running in background, when you start the **minozw process**, it executes `./MinOZW` which executes itself the `.lib/MinOZW` file. If you stop the process, you only kill the `./MinOZW` file.  


## Features
First of all, you must know that all zwave instructions are executed via the cmdline_client executable file. Every command line entered must respect the following syntax.  
The first parameter is the executable file.  
The second one is the function you want to call.  
The next arguments are the function parameters.  

### 0. Get all commands and their descriptions
Before you start reading all features you can try to execute the `help` function. it will return all available function with their respective arguments.

```bash
./cmdline_client help
```

### 1. Add a new node
To set the driver in inclusion mode, you only have to execute with the `include` function. On your application, check regularly the node list and the driver state.

```bash
./cmdline_client include
```

### 2. Remove an old node
To remove a node, pass the driver in exclusion mode with the `exclude` function.

```bash
./cmdline_client exclude
```

### 3. Get information from a node

```bash
./cmdline_client getNode     # Get all information from all nodes.
./cmdline_client getNode 3   # Get information from a specific node with its node ID.
```

### 4. Update a node value
You must enter the value IDentifier as the third argument and its new value as the fourth.  
If you're not sure which value you must enter, you can get the node you want to update and check the value type.

```bash
./cmdline_client setValue 2321893719 '#FF00000000'  # Set the color.
./cmdline_client setValue 1619613132 50             # Set the intensity.
./cmdline_client setValue 8497411350 false          # Change the state.
./cmdline_client setValue 5168413513 'Farhenheit'   # Change the unit of measurement
...
```

### 6. Get the associated matrix to map the ZWave network
The number of rows/columns is the number of nodes on the ZWave network. if you have 2 nodes, you'll get a __3 x 3__ matrix (+1 for the driver).

```bash
./cmdline_client map

>> 0 0 0 0    # The first row/column is the driver, it doesn't have the "neighbor" property.
   1 0 1 1    # first node
   1 1 0 0    # second node
   1 1 0 0    # third node
```

The first node `1 0 1 1` is linked with the driver, the second and the third node.  
The second node `1 1 0 0` is linked with the driver and the first node.  
The third node `1 1 0 0` is linked with the driver and the first node.

### 7. Heal the network
If you want update the ZWave network map, you must **heal** it. The driver sends a signal to every node to update their neighbor maps.  
If you have no sleeping objects, it can be faster. Each sleeping object has a sleeping duration (duration interval) and they don't send any reply during this interval. It is possible to change this interval with the `setValue` function.

```bash
./cmdline_client heal
```

### 8. Restart and reset the ZWave server
It stops the server and execute the `install.sh` file. You must wait few seconds to send a message to the server again.

```bash
./cmdline_client install
```

### 9. Set the process mode
There are different process modes :
- DEV
- TEST
- PROD

Each mode has a specific configuration like the log level or the zwave communication speed.  
You can find your zwave configuration in the `cpp/examples/cache/config.json` file.

To update this mode, send the following request :
```bash
./cmdline_client setLvl PROD
```

It will store the new mode in the `config.json` file thus if you restart your zwave server, it keeps your configurations.

---

If you have any question, you can send a mail to support@five.be.
<p align="right">
Enjoy your code !
<p align="right">
The FiveHome Crew