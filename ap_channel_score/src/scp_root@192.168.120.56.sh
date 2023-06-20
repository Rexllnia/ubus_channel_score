#!/bin/sh

ipv4_addr=192.168.120.56
port=54133
pass=4a4e46ce1e497ea3
name=root
remote_path=/root/
local_file=/home/zyf/buildroot/bin/ramips/est350-v2/packages/base/ap_channel_score_1.0_ramips_24kec.ipk
remote_file=/root/ap_channel_score_1.0_ramips_24kec.ipk
run_file=/etc/ap_channel_score
expect -c "
    spawn scp -P $port -r $local_file $name@$ipv4_addr:$remote_path
    expect {
        \"*assword\" {set timeout 300; send \"$pass\r\"; exp_continue;}
        \"yes/no\" {send \"yes\r\";}
    }
    exit

" && ls && expect -c "
    spawn ssh -l $name $ipv4_addr -p$port
    expect {
        \"password:\" {set timeout 300; send \"$pass\r\"; exp_continue;}
        \"~#\" {send \"opkg install $remote_file && $run_file\r\";}
    }
    interact
" 
