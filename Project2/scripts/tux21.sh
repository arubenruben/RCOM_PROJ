#!/bin/bash
ifconfig eth0 down
ifconfig eth1 down
ifconfig eth0 up
ifconfig eth0 10.10.0.1/24
route add default gw 10.10.0.254
route -n
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts
