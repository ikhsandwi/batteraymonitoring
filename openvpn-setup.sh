#!/bin/sh
sudo ifconfig tun0 down
docker-compose up --build -d
sudo ifconfig tun0 up
cp fix-routes.sh /etc/openvpn/fix-routes.sh
chmod o+x /etc/openvpn/fix-routes.sh

openvpn --config vpn_config_file --route-up fix-routes.sh

