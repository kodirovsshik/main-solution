import threading
import socket

import module_worker_data_reciever as frontend





def log_info(message):
	print("[CORE][Info] " + str(message))

def log_error(message):
	print("[CORE][Error] " + str(message))

def log_excp(message):
	print("[CORE][Excepton] " + str(message))

def core():

	try:
		listener = socket.socket()
		listener.bind(('', 9090))
		listener.listen(socket.SOMAXCONN)
	except Exception as excp:
		log_error(excp);
		exit(code = -1)

	log_info("Waiting for connections")

	threads = []
	while True:
		try:
			sock = listener.accept()[0]
			thread = threading.Thread(target = frontend.worker, args = (sock,))
			thread.start()
			threads.append(thread)
		except Exception as excp:
			log_error(excp);
			break;

	for thread in threads:
		thread.join()
		
try:
	core()
except Exception as excp:
	log_error("UNHANDLED EXCEPTION\n" + str(excp))
