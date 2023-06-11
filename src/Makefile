.PHONY:all clean 

all:
	$(CC) $(CFLAGS) -g -luci -lubus -lubox -lpthread -ljson-c  main.c ubus_thread.c country_channel.c usock_udp.c device_list.c -o channel_score
clean:
	rm *.o channel_score

