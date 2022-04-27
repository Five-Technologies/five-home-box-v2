echo '[Unit]
Description=minozw service

[Service]
ExecStart=/bin/bash -c "cd /home/maximilien/five-home-box-v2 && ./MinOZW"
Restart=always
KillSignal=SIGQUIT
Type=notify
NotifyAccess=all

[Install]
WantedBy=multi-user.target' > /etc/systemd/system/minozw.service

systemctl daemon-reload
systemctl enable minozw.service
systemctl start minozw.service &
systemctl status minozw.service
