import socket
import threading
import math

listener = socket.socket()


def worker(sock: socket):
	addr = sock.getpeername()[0]
	
	print('Connected', addr)
	while True:
		try:
			data = sock.recv(1024)
			if len(data) <= 0:
				break
		except Exception:
			break
		print('From', str(addr), ': "', data.decode('utf-8'), '"')
		sock.send(b'PETON NAHYI')
	
	print('Disconnected', addr)



def main():
	listener.bind(('', 9090))
	listener.listen(socket.SOMAXCONN)
	
	threads = []
	while True:
		#(sock, ) = listener.accept()
		sock = listener.accept()[0]
		thread = threading.Thread(target = worker, args = (sock,))
		thread.start()
		threads.append(thread)
		#threads[len(threads) - 1].start()

	for thread in threads:
		thread.join()
		
main()
