from PyQt5.QtCore import Qt, QObject, QThread, pyqtSignal, QSize, QRect
from PyQt5.QtWidgets import QApplication, QLabel, QPushButton, QVBoxLayout, QWidget, QDialog
from PyQt5.QtGui import QMovie


class LoadingIndicator(QLabel):

    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        movie = QMovie("utility/pictures/loader.gif")
        movie.setScaledSize(QSize(20,20))

        self.setMovie(movie)
        movie.start()
        self.show()
