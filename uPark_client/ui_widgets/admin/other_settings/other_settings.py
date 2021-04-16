#!/usr/bin/python
from PyQt5.QtWidgets import QWidget, QLabel, QMessageBox, QPushButton, QFormLayout, \
                            QVBoxLayout, QHBoxLayout, QLineEdit, QListWidget, \
                            QSpacerItem, QSizePolicy, QComboBox

from PyQt5.QtCore import Qt

from convenience_functions.server_apis import make_http_request

from .add_dialogs import AddHourlyRateDialog, AddVehicleTypeDialog
from .edit_dialogs import EditHourlyRateDialog, EditVehicleTypeDialog, EditUserCategoryDialog

from entities.hourly_rate import HourlyRate
from entities.vehicle_type import VehicleType
from entities.user_category import UserCategory


class OptionsKeypad(QWidget):
    def __init__(self, https_session, entity_type, hourly_rates = None):          # buttons_behaviour_funcs is a tuple: (add_func, del_func, edit_func)
        super().__init__()
        self.https_session = https_session
        self.entity_type = entity_type
        self.hourly_rates = hourly_rates
        OptionsKeypad.initUI(self)

    def initUI(self):
        vbox_main = QVBoxLayout()
        vbox_main.setSpacing(0)
        self.name_lbl = QLabel(self.entity_type.replace("_", " ").capitalize())
        self.name_lbl.setStyleSheet("font-size: 14px;")
        vbox_main.addWidget(self.name_lbl, 1, Qt.AlignBottom)
        self.items_list = QListWidget()
        self.items_list.setSpacing(10)
        self.items_list.setStyleSheet("font: 11pt Arial;")
        self.items_list.setMinimumHeight(250)
        vbox_main.addWidget(self.items_list, 2, Qt.AlignTop)

        hbox_buttons = QHBoxLayout()
        self.add_btn = QPushButton("+")
        self.add_btn.clicked.connect(self.show_add_dialog)
        self.delete_btn = QPushButton("-")
        self.delete_btn.clicked.connect(self.delete_item)
        self.edit_btn = QPushButton("Edit")
        self.edit_btn.clicked.connect(self.show_edit_dialog)

        if self.entity_type != "user_categories":
            hbox_buttons.addWidget(self.add_btn)
            hbox_buttons.addWidget(self.delete_btn)
        hbox_buttons.addWidget(self.edit_btn)

        vbox_main.addLayout(hbox_buttons)

        spacer_item = QSpacerItem(1, 1, vPolicy = QSizePolicy.Expanding)
        vbox_main.addSpacerItem(spacer_item)

        self.setLayout(vbox_main)
        self.get_items()


    def get_items(self):
        self.items_list.clear()
        self.entity_objs = []
        response = make_http_request(self.https_session, "get", self.entity_type)
        if response.json():
            if self.entity_type == "hourly_rates":
                self.entity_objs = [HourlyRate(**hourly_rate) for hourly_rate in response.json()]
            elif self.entity_type == "vehicle_types":
                self.entity_objs = [VehicleType(**vehicle_type) for vehicle_type in response.json()]
            elif self.entity_type == "user_categories":
                self.entity_objs = [UserCategory(**user_category) for user_category in response.json() if user_category["name"] != "Admin"]
            else:
                return
        else:
            return

        if self.entity_objs:                       # if list is empty -> false
            for entity_obj in self.entity_objs:
                if self.entity_type == "hourly_rates":
                    self.items_list.addItem(f"ID: {entity_obj.get_id()} \n" + " "*5 + f"- Amount: {entity_obj.get_amount()}")
                elif self.entity_type == "vehicle_types":
                    self.items_list.addItem(f"Name: {entity_obj.get_name()} \n" + " "*5 + f"- Rate percentage: {entity_obj.get_rate_percentage()}")
                elif self.entity_type == "user_categories":
                    self.items_list.addItem(f"Name: {entity_obj.get_name()} \n" + " "*5 + f"- Hourly rate id: {entity_obj.get_id_hourly_rate()}")


    def show_add_dialog(self):
        if self.entity_type == "hourly_rates":
            add_dialog = AddHourlyRateDialog(self.https_session, self.entity_type, self)
        elif self.entity_type == "vehicle_types":
            add_dialog = AddVehicleTypeDialog(self.https_session, self.entity_type, self)

        if add_dialog.exec_() == 0:
            self.get_items()
        else:
            return


    def delete_item(self):
        selected_item_index = self.items_list.currentRow()

        if selected_item_index != -1:               # one item selected

            entity_name = self.entity_type.replace("_", " ").capitalize()

            reply = QMessageBox.question(self, 'Delete ' + entity_name, f"Are you sure to delete this {entity_name}?",
                                        QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            if reply == QMessageBox.Yes:
                response = make_http_request(self.https_session, "delete", self.entity_type + "/" + str(self.entity_objs[selected_item_index].get_id()))
                if response:
                    QMessageBox.information(self, "Server response", response.text)
                    self.get_items()
        else:
            QMessageBox.information(self, "uPark tip", "Select an item first!")


    def show_edit_dialog(self):
        selected_item_index = self.items_list.currentRow()

        if selected_item_index != -1:               # one item selected

            selected_item_id = self.entity_objs[selected_item_index].get_id()

            if self.entity_type == "hourly_rates":
                edit_dialog = EditHourlyRateDialog(self.https_session, self.entity_type, selected_item_id, self)
            elif self.entity_type == "vehicle_types":
                edit_dialog = EditVehicleTypeDialog(self.https_session, self.entity_type, selected_item_id, self)
            elif self.entity_type == "user_categories":
                edit_dialog = EditUserCategoryDialog(self.https_session, self.entity_type, selected_item_id, self.hourly_rates, self)

            if edit_dialog.exec_() == 0:
                self.get_items()
        else:
            QMessageBox.information(self, "uPark tip", "Select an item first!")


class OtherSettings(QWidget):

    def __init__(self, https_session):
        super().__init__()
        self.https_session = https_session
        self.initUI()


    def initUI(self):

        title = "Other settings"

        vbox_main = QVBoxLayout()

        text = QLabel(title)
        text.setStyleSheet("font-family: Ubuntu; font-size: 30px;")
        vbox_main.addWidget(text, 1, Qt.AlignTop | Qt.AlignHCenter)

        hbox_body = QHBoxLayout()

        hourly_rate_widget = OptionsKeypad(self.https_session, "hourly_rates")
        self.hourly_rates = hourly_rate_widget.entity_objs
        vehicle_type_widget = OptionsKeypad(self.https_session, "vehicle_types")
        user_category_widget = OptionsKeypad(self.https_session, "user_categories", self.hourly_rates)

        hbox_body.addWidget(hourly_rate_widget)
        hbox_body.addWidget(vehicle_type_widget)
        hbox_body.addWidget(user_category_widget)

        vbox_main.addLayout(hbox_body, 1)
        vbox_main.addStretch(1)

        self.setLayout(vbox_main)

        self.setWindowTitle(title)
        self.show()
