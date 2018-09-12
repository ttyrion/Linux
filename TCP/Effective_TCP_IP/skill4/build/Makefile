.PHONY : all
all : client server
	@echo "make " $(BIN) $(CLIENT_BIN) " succeed."

client: $(TEMP_DIR)/tcpclient.o $(TEMP_DIR)/utility.o
	$(CC) -o $(CLIENT_BIN) $^

server: $(TEMP_DIR)/tcpserver.o $(TEMP_DIR)/Server.o $(TEMP_DIR)/utility.o
	$(CC) -o $(BIN) $^

.PHONY : clean
clean:
	-@rm $(BIN)
	-@rm $(CLIENT_BIN)