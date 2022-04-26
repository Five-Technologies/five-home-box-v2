PROCESS_NAME="./MinOZW"
COUNTER=$(ps -aux| grep -c $PROCESS_NAME)
PIDS=$(ps -aux | grep $PROCESS_NAME | awk '{print($2)}')
OLD_PATH=$PWD

if [[ $COUNTER -gt 1 ]]
then
  echo "Process $PROCESS_NAME found"
  kill $PIDS

  while [ $COUNTER -gt 1 ]
  do
    COUNTER=$(ps -aux| grep -c $PROCESS_NAME)
  done

  echo "Process $PROCESS_NAME killed"
else
  echo "Process $PROCESS_NAME is not running"
fi

cd $HOME/five-home-box-v2
nohup ./MinOZW &
echo "Process $PROCESS_NAME restarted"
cd $OLD_PATH

# echo "PIDS: $($PIDS)"

# PIDS=$(ps -a | grep MinOZW | awk '{print($1)}')
# echo "PIDs"
# echo $PIDS

# kill %1

# ps -e | grep MinOZW | awk '{print($1)}' | xargs kill
# make ~/five-home-box-v2
# . ~/five-home-box-v2/MinOZW &
