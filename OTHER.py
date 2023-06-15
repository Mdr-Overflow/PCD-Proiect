import socket
import os
import json
from threading import Thread
import glob

def performGET(socket_desc, file_name):
    # Check if the file exists at the server
    if not os.path.exists(file_name):
        print("File doesn't exist at server. Aborting.")
        return

    # Send GET request to server
    request_msg = "GET " + file_name
    socket_desc.sendall(request_msg.encode())

    # Receive reply from server
    reply_msg = socket_desc.recv(1024).decode()

    # If server replied with "OK", receive the file
    if reply_msg == "OK":
        # Open file for writing
        with open(file_name, 'wb') as file:
            print("File opened")
            while True:
                data = socket_desc.recv(1024)
                if not data:
                    break
                # write data to a file
                file.write(data)

        print(f"{file_name} File successfully received")
    else:
        print(f"Failed to get {file_name} from server")



def performPUT(socket_desc, file_name):
    # Check if the file exists locally
    if not os.path.exists(file_name):
        print("File doesn't exist locally. Aborting.")
        return

    # Send PUT request to server
    request_msg = "PUT " + file_name
    socket_desc.sendall(request_msg.encode())


    reply_msg = socket_desc.recv(1024).decode()
    print(reply_msg)
    
    if reply_msg == "OK":

        with open(file_name, 'rb') as file:
            socket_desc.sendall(file.read())
        print(f"{file_name} file sent successfully")
    elif reply_msg == "FP":
        # File exists on server, ask for overwrite permission
        overwrite_permission = input("File exists on server. Overwrite? (yes/no): ")
        if overwrite_permission.lower() == "yes":
            socket_desc.sendall("Y".encode())
            with open(file_name, 'rb') as file:
                socket_desc.sendall(file.read())
            print(f"{file_name} file sent successfully")
        else:
            socket_desc.sendall("N".encode())
    else:
        print("An error occurred.")

def performMGET(socket_desc):
    ext = input("Type Extension: ").strip()
    request_msg = "MGET " + ext
    socket_desc.sendall(request_msg.encode())

    while True:
        file_name = socket_desc.recv(1024).decode()
        if file_name == "END":
            break
        
        print(f"Receiving {file_name}")
        
        if os.path.exists(file_name):
            overwrite = int(input("File already exists. Press 1 to overwrite. Press any other key to abort."))
            if overwrite != 1:
                socket_desc.sendall("NO".encode())
                print(f"Not overwriting {file_name}")
                continue
            print(f"Overwriting {file_name}")
        
        socket_desc.sendall("OK".encode())
        reply = socket_desc.recv(1024).decode()

        if reply == "OK":
            with open(file_name, 'wb') as file:
                data = socket_desc.recv(1024)
                while data:
                    file.write(data)
                    data = socket_desc.recv(1024)
            print(f"File {file_name} received")
        else:
            print(f"An error occurred with {file_name}")

    print("MGET Complete")


def performMPUT(socket_desc):
    print("Performing MPUT")
    ext = input("Type Extension: ").strip()
    files = glob.glob(f"*.{ext}")
    for file_name in files:
        performPUT(socket_desc, file_name)
    print("MPUT Complete")

def performSHOW(socket_desc):
    # Send a request to the server to show something
    request_msg = "SHOW"
    socket_desc.sendall(request_msg.encode())

 
    reply = socket_desc.recv(1024).decode()  
    print(reply)

def performSELECT(socket_desc):
    
    request_msg = "SELECT"
    socket_desc.sendall(request_msg.encode())

 
    reply = socket_desc.recv(1024).decode()  
    print(reply)
def main():
 
    import sys
    if len(sys.argv) != 3:
        print("Invalid arguments")
        return

    SERVER_IP = sys.argv[1]
    SERVER_PORT = int(sys.argv[2])

    socket_desc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    server = (SERVER_IP, SERVER_PORT)


    socket_desc.connect(server)

    while True:
        choice = int(input("Enter a choice:\n1- GET\n2- PUT\n3- MGET\n4- MPUT\n5- SHOW\n6- SELECT\n7- EXIT\n"))
        if choice == 1:
            file_name = input("Enter file_name to get: ")
            performGET(socket_desc, file_name)
        elif choice == 2:
            file_name = input("Enter file_name to put: ")
            performPUT(socket_desc, file_name)
        elif choice == 3:
            performMGET(socket_desc)
        elif choice == 4:
            performMPUT(socket_desc)
        elif choice == 5:
            performSHOW(socket_desc)
        elif choice == 6:
            performSELECT(socket_desc)
        elif choice == 7:
            socket_desc.sendall(b"EXIT")
            return
        else:
            print("Incorrect command")


if __name__ == "__main__":
    main()