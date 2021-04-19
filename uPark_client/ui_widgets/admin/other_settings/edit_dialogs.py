from PyQt5.QtWidgets import QDialog, QDialogButtonBox, QLabel, QMessageBox, QFormLayout, \
                            QVBoxLayout, QHBoxLayout, QLineEdit, QComboBox

from PyQt5.QtCore import Qt

from convenience.server_apis import make_http_request

from .add_dialogs import AddHourlyRateDialog, AddVehicleTypeDialog

from .base_dialog import BaseDialog


class EditHourlyRateDialog(AddHourlyRateDialog):
    def __init__(self, https_session, entity_type, selected_item_id, parent=None):
        super().__init__(https_session, entity_type, parent=parent)
        self.selected_item_id = selected_item_id
        self.initUI()


    def initUI(self):
        entity_name = self.entity_type.replace("_", " ")
        self.setWindowTitle("Edit " + entity_name)

        self.text_lbl.setText("<i>Please, edit " + entity_name + " info:</i>")
        self.buttonBox.disconnect()                           # disconnect all object signals from their slots. We need it to remove BaseDialog slot for accepted signal.
        self.buttonBox.accepted.connect(self.edit_hourly_rate)


    def edit_hourly_rate(self):
        hourly_rate = {"amount": self.amount_le.text()}
        response = make_http_request(self.https_session, "put", "hourly_rates/" + str(self.selected_item_id), json = hourly_rate)
        if response:
            QMessageBox.information(self, "Server response", response.text)
            self.done(0)
        else:
            self.done(1)


class EditVehicleTypeDialog(AddVehicleTypeDialog):
    def __init__(self, https_session, entity_type, selected_item_id, parent=None):
        super().__init__(https_session, entity_type, parent=parent)
        self.selected_item_id = selected_item_id
        self.initUI()


    def initUI(self):
        entity_name = self.entity_type.replace("_", " ")
        self.setWindowTitle("Edit " + entity_name)

        self.text_lbl.setText("<i>Please, edit " + entity_name + " info:</i>")
        self.buttonBox.disconnect()                           # disconnect all object signals from their slots. We need it to remove BaseDialog slot for accepted signal.
        self.buttonBox.accepted.connect(self.edit_vehicle_type)


    def edit_vehicle_type(self):
        vehicle_type = {
                        "name": self.name_le.text(),
                        "rate_percentage": self.rate_percentage_le.text()
                        }
        response = make_http_request(self.https_session, "put", "vehicle_types/" + str(self.selected_item_id), json = vehicle_type)
        if response:
            QMessageBox.information(self, "Server response", response.text)
            self.done(0)
        else:
            self.done(1)


class EditUserCategoryDialog(BaseDialog):
    def __init__(self, https_session, entity_type, selected_item_id, hourly_rates, parent=None):
        super().__init__(https_session, entity_type, parent=parent)
        self.selected_item_id = selected_item_id
        self.hourly_rates = hourly_rates
        self.initUI()


    def initUI(self):
        entity_name = self.entity_type.replace("_", " ")
        self.setWindowTitle("Edit " + entity_name)

        self.text_lbl.setText("<i>Please, edit " + entity_name + " info:</i>")
        self.buttonBox.accepted.connect(self.edit_user_category)

        self.hourly_rate_cmb = QComboBox()
        self.hourly_rate_cmb.addItems([str(hourly_rate.get_amount()) for hourly_rate in self.hourly_rates])
        self.form_layout.addRow("<b>Hourly rate: </b> ", self.hourly_rate_cmb)


    def edit_user_category(self):
        cmb_index = self.hourly_rate_cmb.currentIndex()
        if cmb_index != -1:
            hourly_rate = self.hourly_rates[cmb_index]
            user_category = {
                            "id_hourly_rate": hourly_rate.get_id(),
                            "service_validity_start": None,
                            "service_validity_end": None
                            }
            response = make_http_request(self.https_session, "put", "user_categories/" + str(self.selected_item_id), json = user_category)
            if response:
                QMessageBox.information(self, "Server response", response.text)
                self.done(0)
            else:
                self.done(1)
        else:
            QMessageBox.information(self, "uPark tip", "Select an hourly rate first!")
