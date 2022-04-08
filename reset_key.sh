echo -ne "\x01\x04\x00\x42\x01\xB8" > /dev/ttyACM0;
rm cpp/examples/cache/nodes/node_*;
rm cpp/examples/cache/ozwcache*;
echo "" > cpp/examples/cache/OZW.log;
echo "key restored";