from PyQt5.QtWidgets import QDialog, QDialogButtonBox, QLabel, QMessageBox, QFormLayout, \
                            QVBoxLayout, QHBoxLayout, QLineEdit, QComboBox
from PyQt5.QtCore import Qt

from convenience.server_apis import make_http_request

from .base_dialog import BaseDialog


class AddHourlyRateDialog(BaseDialog):
    def __init__(self, https_session, entity_type, parent=None):
        super().__init__(https_session, entity_type, parent=parent)
        AddHourlyRateDialog.initUI(self)

    def initUI(self):
        self.buttonBox.accepted.connect(self.add_hourly_rate)
        self.amount_le = QLineEdit()
        self.form_layout.addRow("<b>Amount: </b> ", self.amount_le)


    def form_validator(self, input_field):
        try:
            if float(input_field) <= 0 or float(input_field) > 9.99:
                QMessageBox.information(self, "uPark tip", "Amount must be greater than 0 and lower than 10!")
                return False
        except ValueError:
           QMessageBox.information(self, "uPark tip", "Amount must be a number!")
           return False
        return True


    def add_hourly_rate(self):
        if self.form_validator(self.amount_le.text()):
            hourly_rate = {"amount": self.amount_le.text()}
            response = make_http_request(self.https_session, "post", "hourly_rates", json = hourly_rate)
            if response:
                QMessageBox.information(self, "Server response", response.json()["message"])
                self.done(0)
            else:
                self.done(1)


class AddVehicleTypeDialog(BaseDialog):
    def __init__(self, https_session, entity_type, parent=None):
        super().__init__(https_session, entity_type, parent=parent)
        AddVehicleTypeDialog.initUI(self)

    def initUI(self):
        self.buttonBox.accepted.connect(self.add_vehicle_type)
        self.name_le = QLineEdit()
        self.rate_percentage_le = QLineEdit()
        self.form_layout.addRow("<b>Name: </b> ", self.name_le)
        self.form_layout.addRow("<b>Rate percentage: </b> ",  self.rate_percentage_le)


    def form_validator(self, input_str, input_float):
        if len(input_str) == 0:
            QMessageBox.information(self, "uPark tip", "Name can't be blank!")
            return False
        try:
            if float(input_float) <= 0 or float(input_float) > 9.99:
                QMessageBox.information(self, "uPark tip", "Rate percentage must be greater than 0 and lower than 10!")
                return False
        except ValueError:
           QMessageBox.information(self, "uPark tip", "Rate percentage must be a number!")
           return False
        return True


    def add_vehicle_type(self):
        if self.form_validator(self.name_le.text(), self.rate_percentage_le.text()):
            vehicle_type = {
                            "name": self.name_le.text(),
                            "rate_percentage": self.rate_percentage_le.text()
                            }
            response = make_http_request(self.https_session, "post", "vehicle_types", json = vehicle_type)
            if response:
                QMessageBox.information(self, "Server response", response.json()["message"])
                self.done(0)
            else:
                self.done(1)
