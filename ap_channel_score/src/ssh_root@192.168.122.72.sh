#!/bin/sh

ipv4_addr=192.168.122.72
port=54133
pass=c93775f4ec5d2378
name=root
remote_path=/root/
local_file=/home/zyf/buildroot/bin/ramips/est310-v2/packages/base/channel_score_1.0_ramips_24kec.ipk
remote_file=/root/channel_score_1.0_ramips_24kec.ipk
run_file=/etc/channel_score
expect -c "
    spawn ssh -l $name $ipv4_addr -p$port
    expect {
        \"password:\" {set timeout 300; send \"$pass\r\"; exp_continue;}
        \"~#\" {send \"opkg install $remote_file && $run_file\r\";}
    }
    interact
" 
