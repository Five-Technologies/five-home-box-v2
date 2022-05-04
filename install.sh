#!/bin/sh

cd /srv/app/zwave/

RUNNING="$(ps | grep -c MinOZW)"
PIDS="$(ps | grep MinOZW | awk {'print($1)'} | xargs)"
MINOZW="$(ls -l /etc/init.d/ | grep -c minozw)"
STATUS="$(rc-status -a | grep minozw | awk {'print($3)'})"
CACHE='/srv/app/zwave/cpp/examples/cache'
LOGS="$(ls -la $CACHE/nodes | grep -c node_)"

### CHECK IF THE SERVICE ALREADY EXISTS ###
if [ $MINOZW -gt 1 ]
then
    echo "[bash] minozw service already installed"
    
    ### CHECK IF THE SERVICE IS RUNNING ###
    if [[ $STATUS == "stopped" ]]
    then
        echo "[bash] minozw service already stopped"
    else
        ### STOP THE SERVICE ###
        rc-service minozw stop
        rc-service del minozw
    fi
else
    echo "[bash] install minozw open-rc service"

    ### WRITE CODE IN THE FUTURE OPEN-RC SERVICE ###
    echo "#!/sbin/openrc-run

    name=\"minozw\"
    description=\"minozw service\"
    supervisor=\"supervise-daemon\"
    command=\"sh\"
    command_args=\"-c 'cd /srv/app/zwave && ./MinOZW'\"" > /etc/init.d/minozw

    ### GRANT PRIVILEGES ###
    chmod 777 /etc/init.d/minozw
    rc-update add minozw

    echo "[bash] minozw service installed"
fi

### CHECK IF SOME GARBAGES EXIST AND KILL THEM ###
if [ $RUNNING -gt 1 ]
then
    kill -SIGTERM $PIDS
fi

### RESTORE THE DRIVER ###
echo -ne "\x01\x04\x00\x42\x01\xB8" > /dev/ttyACM0
sleep 1

### REMOVE ALL OLD LOG FILES ###
for VARIABLE in "ozwcache*" "journal.log" "OZW.log" "zwscene.xml"
do
    LOGS="$(ls -la $CACHE | grep -c $VARIABLE)"
    if [ $LOGS -gt 0 ]
    then
        echo "[bash] delete \"$VARIABLE\""
        rm $CACHE/$VARIABLE
    fi
done

### COMPILE OPENZWAVE PROJECT ###
make

### START THE OPEN-RC SERVICE ###
rc-service minozw start
echo "[bash] minozw started"