This project written in c++, need to make it complete work flow with this project: https://github.com/lastcow/retrocow-lora-gw-python

This project is modified based on WAZIUP project, removed uncecessary features.

###Make project
```
make retrocow_lora_gateway
```

###Script to start
start.sh contain the command to start lora gateway and passing data to post-processing python script, if you like to everything just print on script, just remove post processing pythong script.

###Start gateway as service
create a file called lora.service as following:

```
[Unit]
Description=rp3 button push to shutdown service
After=network.target

[Service]
ExecStart=/home/pi/retrocow-lora-gw/start.sh
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

setup service:
```
sudo mv lora.service /etc/systemd/system/.
sudo systemctl enable lora.service
sudo systemctl start lora.service
```
if device restart, the service will start automatically, if you see the screen is on and message display, means it start.