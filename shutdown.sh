RUNNING="$(ps | grep -c MinOZW)"
PIDS="$(ps | grep MinOZW | awk {'print($1)'} | xargs)"

rc-service minozw stop

### CHECK IF SOME GARBAGES EXIST AND KILL THEM ###
if [ $RUNNING -gt 1 ]
then
    kill -SIGTERM $PIDS
fi