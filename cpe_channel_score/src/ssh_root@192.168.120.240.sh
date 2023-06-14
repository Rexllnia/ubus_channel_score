#!/bin/sh

ipv4_addr=10.44.77.254
port=54133
pass=04c8dcb13191ed65
name=root
remote_path=/root/
local_file=/home/zyf/buildroot/bin/ramips/est350-v2/packages/base/cpe_channel_score_1.0_ramips_24kec.ipk
remote_file=/root/cpe_channel_score_1.0_ramips_24kec.ipk
run_file=/etc/cpe_channel_score
expect -c "
    spawn ssh -l $name $ipv4_addr -p$port
    expect {
        \"password:\" {set timeout 300; send \"$pass\r\"; exp_continue;}
        \"~#\" {send \"opkg install $remote_file && $run_file\r\";}
    }
    interact
" 
