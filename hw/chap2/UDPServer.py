from socket import *
import sys, pdb

serverPort = int(sys.argv[1])
serverSocket = socket(AF_INET, SOCK_DGRAM)
serverSocket.bind(("", serverPort))

print("The server is ready to receive")

while True:
    message, clientAddress = serverSocket.recvfrom(2048)
    capitalizedSentence = message.decode().upper()
    serverSocket.sendto(capitalizedSentence.encode(), clientAddress)
