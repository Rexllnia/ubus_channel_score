.PHONY:all clean 

all:
	$(CC) $(CFLAGS) -g -luci -lubus -lubox -lpthread -ljson-c  device_list.h main.c tools.c channel_scan.c channel_bitmap.c tipc_scan_msg.c kv_nav_task.c -o cpe_channel_score
clean:
	rm *.o cpe_channel_score

