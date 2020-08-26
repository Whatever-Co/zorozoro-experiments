from socket import socket, AF_INET, SOCK_DGRAM

HOST = '0.0.0.0'
PORT = 12322

s = socket(AF_INET, SOCK_DGRAM)
s.bind((HOST, PORT))

buffers = {}

while True:
    data, (address, port) = s.recvfrom(8192)
    data = data.decode('utf-8')
    if not address in buffers:
        buffers[address] = ""
    buffers[address] += data
    lines = buffers[address].split('\n')
    buffers[address] = lines.pop()
    for line in lines:
        print("[{}] {}".format(address, line))

s.close()
