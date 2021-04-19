from PyQt5.QtWidgets import QDialog, QDialogButtonBox, QLabel, QMessageBox, QFormLayout, \
                            QVBoxLayout, QHBoxLayout, QLineEdit, QComboBox

from PyQt5.QtCore import Qt

from convenience.server_apis import make_http_request


class BaseDialog(QDialog):
    def __init__(self, https_session, entity_type, parent=None):
        super().__init__(parent=parent)
        self.https_session = https_session
        self.entity_type = entity_type

        BaseDialog.initUI(self)


    def initUI(self):

        entity_name = self.entity_type.replace("_", " ")
        self.setWindowTitle("Add " + entity_name)

        QBtn = QDialogButtonBox.Ok | QDialogButtonBox.Cancel
        self.buttonBox = QDialogButtonBox(QBtn)
        self.buttonBox.rejected.connect(self.reject)

        layout = QVBoxLayout()
        self.text_lbl = QLabel("<i>Please, insert new " + entity_name + " info:</i>")
        layout.addWidget(self.text_lbl, 0, Qt.AlignCenter)

        self.form_layout = QFormLayout()
        layout.addLayout(self.form_layout)
        layout.addSpacing(20)
        layout.addWidget(self.buttonBox)
        self.setLayout(layout)
