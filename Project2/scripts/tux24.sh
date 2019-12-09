#!/bin/bash
ifconfig eth0 down
ifconfig eth1 down
ifconfig eth0 up
ifconfig eth0 172.16.20.254
ifconfig eth1 up
ifconfig eth1 172.16.21.253
route add default gw 172.16.21.254
route -n
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts
