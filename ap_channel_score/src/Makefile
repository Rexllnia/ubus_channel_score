.PHONY:all clean 

all:
	$(CC) $(CFLAGS) -g -luci -lubus -lubox -lpthread -ljson-c  main.c tipc_func.c ubus_thread.c country_channel.c usock_udp.c device_list.c blobmsg_json_ex.c -o ap_channel_score
clean:
	rm *.o ap_channel_score

