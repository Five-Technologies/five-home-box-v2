#!/bin/sh

PATH=$PWD
USER=maximilien
echo "$(sudo systemctl list-units | grep -c minozw.service)"

echo -ne "\x01\x04\x00\x42\x01\xB8" > /dev/ttyACM0

OZW="$(ls -la /home/maximilien/five-home-box-v2/cpp/examples/cache/ | grep -c OZW.log)"

if [[ $OZW == "1" ]]
then
	echo '[bash] removing log files'

	CACHE=/home/$USER/five-home-box-v2/cpp/examples/cache
	rm $CACHE/nodes/node_*
	rm $CACHE/ozwcache*
	rm $CACHE/failed_nodes.log
	rm $CACHE/OZW.log
fi

cd /home/$USER/five-home-box-v2/
make
cd $PATH

if [[ $COUNTER == 0 ]]
then
	echo '[bash] minozw.service creation'

	echo "[Unit]
	Description='minozw service'

	[Service]
	ExecStart=/bin/bash -c cd \"/home/$USER/five-home-box-v2 && ./MinOZW\"
	Restart=always
	KillSignal=SIGQUIT
	Type=notify
	NotifyAccess=all

	[Install]
	WantedBy=multi-user.target" > /etc/systemd/system/minozw.service

#	systemctl daemon-reload
#	systemctl enable minozw.service
#	systemctl start minozw.service &
else
	echo '[bash] minozw.service already created'
#	systemctl restart minozw.service
fi

echo '[bash] minozw.service up'
