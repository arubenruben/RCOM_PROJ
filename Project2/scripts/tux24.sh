#!/bin/bash
ifconfig eth0 down
ifconfig eth1 down
ifconfig eth0 up
ifconfig eth0 172.16.20.254/24
ifconfig eth1 up
ifconfig eth1 172.16.21.253/24
route add default gw 172.16.21.254
route -n
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts

# iptables -t nat -A POSTROUTING -s 172.16.20.1 -o eth1 -j SNAT --to-source 10.10.0.1
# iptables -t nat -A PREROUTING -d 10.10.0.1 -i eth1 -j DNAT --to-destination 172.16.20.1