import socket
import time
import os

from socket import socket as sock_t


#(
#File prefix,
#File suffix
#Data encoding
#Format suffix
#Data size
#)
data_global_config = {
	"txt": ("txt ", ".txt", "utf-8", "t", 1024 * 1024),
	"pic": ("pic ", ".jpg", "", "b", 1024 * 1024 * 32),
	"mp4": ("mp4 ", ".mp4", "", "b", 1024 * 1024 * 32),
	}

file_counters = {}
folder_name = "%X" % int(time.time())
os.mkdir(folder_name)

MAX_ZERO_RECEIVES = 32

def discard_connection(sock: sock_t, reason: str):
	sock.send(b"err")
	sock.close()
	raise Exception(reason)

def discard_connection_because(sock: sock_t, reason: str):
	sock.send(b"err")
	sock.send(bytes(reason, "utf-8"))
	sock.close()
	raise Exception(reason)

def _log_info(addr, msg):
	print("[", addr, "][Info] ", str(msg), sep='')

def _log_error(addr, msg):
	print("[", addr, "][Error] ", str(msg), sep='')

def _log_excp(addr, msg):
	print("[", addr, "][Exception] ", str(msg), sep='')

def worker(sock: sock_t):
	#log_info log_error
	disconnected_by_exception = False

	log_info = lambda msg : _log_info(addr, msg)
	log_error = lambda msg : _log_error(addr, msg)
	log_excp = lambda msg : _log_excp(addr, msg)

	addr = sock.getpeername()[0]
	data_config = data_global_config

	file_counter = file_counters.get(str(addr), 0)
	
	log_info("Connection established")

	while True:
		try:

			data_name = sock.recv(32)
			if len(data_name) == 32:
				discard_connection_because(sock, "Too many bytes for data descriptor")
				continue
			if len(data_name) == 0:
				discard_connection(sock, "Disconnected")
			data_name = data_name.decode("utf-8")
			data_name = data_name.split()
			data_length = int(data_name[1])
			data_name = data_name[0]
			if len(data_name) == 0: discard_connection("Disconnected")
			try:
				type_info = data_config[data_name]
			except Exception as excp:
				discard_connection_because(sock, "Unknown type " + str(excp))

			#sock.send(bytes('Lelei La Lelena top waifu.', 'utf-8'))
			sock.send(bytes('All fine.', 'utf-8'))
			
			config_prefix = type_info[0]
			config_suffix = type_info[1]
			config_encoding = type_info[2]
			config_bt = type_info[3]
			config_data_limit = type_info[4]

			filename = folder_name + "/" + config_prefix + str(addr) + "_" + str(file_counter) + config_suffix
			file = open(filename, "w" + config_bt)
			data = bytes()
			
			data_length_current = 0
			zero_receives = 0
			while data_length_current < data_length:
				temp_data = sock.recv(config_data_limit)
				got_length = len(temp_data)

				if got_length == 0:
					zero_receives += 1
					if zero_receives == MAX_ZERO_RECEIVES:
						discard_connection_because(sock, "Not enough bytes of data type " + data_name + "\nGot " + str(len(data)) + " of " + str(data_length))

				data_length_current += got_length
				data += temp_data

			if len(data) >= config_data_limit - 1:
				discard_connection_because(sock, "Too many bytes of data type " + data_name)

			if len(data) != data_length:
				discard_connection_because(sock, "Not enough bytes of data type " + data_name + "\nGot " + str(len(data)) + " of " + str(data_length))

			sock.send(bytes('All fine.', 'utf-8'))
			
			if config_encoding != "":
				data = data.decode(config_encoding)

			file.write(data)
			#print(data, file = file, sep = "")
			
			file.close()
			file_counter = file_counter + 1

		except Exception as excp:
			log_excp(excp)
			disconnected_by_exception = True
			break

	file_counters[str(addr)] = file_counter
	sock.close()
	if not disconnected_by_exception: log_info("Disconnected")
