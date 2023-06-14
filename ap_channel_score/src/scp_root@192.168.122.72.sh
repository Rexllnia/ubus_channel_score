#!/bin/sh

ipv4_addr=192.168.122.72
port=54133
pass=c93775f4ec5d2378
name=root
remote_path=/root/
local_file=/home/zyf/buildroot/bin/ramips/est310-v2/packages/base/ap_channel_score_1.0_ramips_24kec.ipk
remote_file=/root/ap_channel_score_1.0_ramips_24kec.ipk
expect -c "
    spawn scp -P $port -r $local_file $name@$ipv4_addr:$remote_path
    expect {
        \"*assword\" {set timeout 300; send \"$pass\r\"; exp_continue;}
        \"yes/no\" {send \"yes\r\";}
    }
"
