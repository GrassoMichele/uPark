#! /usr/bin/python3
import socket
import sys
import time
from functools import partial
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, \
    QPushButton, QVBoxLayout, QFileDialog, QMessageBox, QLabel, \
    QDesktopWidget
from PyQt5.QtCore import Qt

CHUNK_SIZE = 10240

def get_image_size(file):
    file.seek(0,2)        # 2 means seek relative to the file's end
    size = file.tell()
    file.seek(0)          # default absolute file positioning
    print(f"Total picture size: {size} ")
    return size


def send_image(socket, path):
    packet_index = 1

    try:
        with open(path, "rb") as f:
            print("Sending picture size... ")
            get_image_size(f).to_bytes(4, 'little')
            socket.sendall(get_image_size(f).to_bytes(4, 'little'))
            print(len(str(get_image_size(f)).encode('utf-8')))

            # receiving verification signal
            bytes_read = socket.recv(2)

            if len(bytes_read) == 0 :
                return 1

            print(f"Bytes read: {len(bytes_read)} ")
            print(f"Received data in socket: {bytes_read.decode('utf-8')} ")
            print("Sending picture... ")

            for chunck in iter(partial(f.read, CHUNK_SIZE), b''):
                socket.sendall(chunck)
                print(f"Packet Size: {CHUNK_SIZE} ")
                print(f"Packet Number: {packet_index} ")
                packet_index += 1

    except IOError as e:
        print(f"Some error in image opening: {e} ")


class MainWindow(QMainWindow):
    def __init__(self, socket):
        super().__init__()

        self.setWindowTitle("Parking Lot Client")
        self.setFixedSize(500,125)
        # to center the window
        qtRectangle = self.frameGeometry()
        centerPoint = QDesktopWidget().availableGeometry().center()
        qtRectangle.moveCenter(centerPoint)
        self.move(qtRectangle.topLeft())

        label = QLabel(f"<b>Server Host</b>: {socket.getsockname()[0]} <br> <b>Port</b>: {socket.getsockname()[1]}")
        label.setTextFormat(Qt.RichText)

        button = QPushButton("Select Vehicle Image")

        layout = QVBoxLayout()
        layout.addWidget(label)
        layout.addWidget(button)

        widget = QWidget()
        widget.setLayout(layout)
        self.setCentralWidget(widget)

        self.socket = socket
        button.clicked.connect(lambda : self.open(self.socket))

        self.show()

    def open(self, socket):
        path = QFileDialog.getOpenFileName(self, 'Select Vehicle Image', './vehicles', 'All Files (*.jpg)')
        if path[0] != '':                    # path contain a tuple of type (fileName, selectedFilter)
            print(f"File path : {path[0]}\n")

            if send_image(socket, path[0]) == 1:
                QMessageBox.critical(self, "Notification", " Processing Server has gone away!\n Bye!")
                sys.exit(1)

            # reading "open" command
            open_command = socket.recv(2).decode('utf-8')

            if open_command[:-1] == "1":
                QMessageBox.information(self, "Notification", "CROSSING ALLOWED!")
            else:
                QMessageBox.critical(self, "Notification", "CROSSING NOT ALLOWED!")


def main():
    # check if port argument was provided
    if len(sys.argv) < 2:
        print("ERROR, no port provided! ")
        sys.exit(1)

    # Connection info
    serverHost = '127.0.0.1'  # The server's hostname or IP address
    serverPort = 50051        # The port used by the server

    clientHost = '127.0.0.1'
    clientPort = int(sys.argv[1])

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sckt:

        try:
            sckt.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            sckt.bind((clientHost, clientPort))
            sckt.connect((serverHost, serverPort))

            # GUI
            app = QApplication(sys.argv)
            window = MainWindow(sckt)
            sys.exit(app.exec_())

        except OSError as e:
            print(f"Error on socket: {e} ")
        except KeyboardInterrupt as e:
            print("\n(CTRL+C) Closing Parking Lot Client...")
            sckt.close()
            sys.exit(0);


if __name__ == "__main__":
    main()
