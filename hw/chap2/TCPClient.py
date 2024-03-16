from socket import *
import sys, pdb

serverName = "localhost"
serverPort = int(sys.argv[1])

clientSocket = socket(AF_INET, SOCK_STREAM)
breakpoint()
clientSocket.connect((serverName, serverPort))

sentence = input("Input lowercase sentence:")
breakpoint()
clientSocket.send(sentence.encode())
modifiedSentence = clientSocket.recv(1024)

print("From Server: ", modifiedSentence.decode())
breakpoint()
clientSocket.close()
