echo -ne "\x01\x04\x00\x42\x01\xB8" > /dev/ttyACM0

ABS_PATH=/home/maximilien/five-home-box-v2

rm $ABS_PATH/cpp/examples/cache/nodes/node_*
rm $ABS_PATH/cpp/examples/cache/ozwcache*
rm $ABS_PATH/cpp/examples/cache/failed_nodes.log
rm $ABS_PATH/cpp/examples/cache/OZW.log

sudo systemctl restart minozw.service