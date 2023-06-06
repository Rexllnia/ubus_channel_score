#!/bin/sh

ipv4_addr=192.168.120.201
port=54133
pass=24377d9689943a58
name=root
remote_path=/root/
local_file=/home/zyf/buildroot/bin/ramips/est350-v2/packages/base/channel_score_1.0_ramips_24kec.ipk
remote_file=/root/channel_score_1.0_ramips_24kec.ipk
run_file=/etc/channel_score
expect -c "
    spawn scp -P $port -r $local_file $name@$ipv4_addr:$remote_path
    expect {
        \"*assword\" {set timeout 300; send \"$pass\r\"; exp_continue;}
        \"yes/no\" {send \"yes\r\";}
    }
" 
