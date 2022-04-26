echo -ne "\x01\x04\x00\x42\x01\xB8" > /dev/ttyACM0
rm ../cache/nodes/node_*
rm ../cache/ozwcache*
rm ../cache/failed_nodes.log
echo "" > ../cache/OZW.log
cd ../../../
make

echo "key restored"