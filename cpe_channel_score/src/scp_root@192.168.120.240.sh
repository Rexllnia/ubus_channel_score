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
    spawn scp -P $port -r $local_file $name@$ipv4_addr:$remote_path
    expect {
        \"*assword\" {set timeout 300; send \"$pass\r\"; exp_continue;}
        \"yes/no\" {send \"yes\r\";}
    }
" 
