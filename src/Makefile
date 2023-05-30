.PHONY:all clean
all:
	$(CC) $(CFLAGS) -g -luci -lubus -lubox -lunifyframe -ljson-c  main.c ubus_thread.c country_channel.c -o channel_score 
clean:
	rm *.o channel_score

