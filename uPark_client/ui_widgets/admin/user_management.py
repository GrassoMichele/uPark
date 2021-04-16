#!/usr/bin/python

from PyQt5.QtWidgets import QWidget, QLabel, QMessageBox, QTableWidget, QAbstractItemView, QPushButton, QCheckBox, \
                            QVBoxLayout, QDialog, QDialogButtonBox, QTableWidgetItem, QHeaderView, QFormLayout, \
                            QTabWidget, QLineEdit, QScrollArea, QListWidget

from PyQt5.QtCore import Qt

from convenience_functions.server_apis import make_http_request
from ui_widgets.user.user_vehicles import UserVehicles

from entities.user import User
from entities.vehicle import Vehicle
from entities.user_category import UserCategory
from entities.vehicle_type import VehicleType


class DetailsDialog(QDialog):
    def __init__(self, user, https_session, parent = None):
        super().__init__(parent=parent)
        self.user = user
        self.https_session = https_session
        self.initUI()

    def initUI(self):

        self.state_changed = False

        self.setWindowTitle("User details")

        QBtn = QDialogButtonBox.Ok | QDialogButtonBox.Cancel

        buttonBox = QDialogButtonBox(QBtn)
        buttonBox.accepted.connect(self.edit_user)
        buttonBox.rejected.connect(self.reject)

        layout = QVBoxLayout()

        tab_widget = QTabWidget()

        update_info_widget = QWidget()

        form_layout = QFormLayout()

        self.password_le = QLineEdit()
        form_layout.addRow("<b>Password: </b> ", self.password_le)
        self.disability_cb = QCheckBox()
        self.disability_cb.setChecked(self.user.get_disability())
        self.disability_cb.stateChanged.connect(self.set_state_changed)
        form_layout.addRow("<b>Disability: </b> ", self.disability_cb)
        self.active_account_cb = QCheckBox()
        self.active_account_cb.setChecked(self.user.get_active_account())
        self.active_account_cb.stateChanged.connect(self.set_state_changed)
        form_layout.addRow("<b>Active account: </b> ", self.active_account_cb)
        self.delete_btn = QPushButton("Delete")
        self.delete_btn.clicked.connect(self.delete_user)
        form_layout.addRow("<b>Delete user: </b> ", self.delete_btn)

        update_info_widget.setLayout(form_layout)

        tab_widget.addTab(update_info_widget, "User info")
        tab_widget.addTab(UserVehiclesWidget(self.user, self.https_session), "Vehicles")

        layout.addWidget(tab_widget)

        layout.addSpacing(20)

        layout.addWidget(buttonBox)
        self.setLayout(layout)


    def set_state_changed(self):
        self.state_changed = True


    def edit_user(self):
        if self.state_changed or self.password_le.text() != "":
            user = {
                    "disability": self.disability_cb.isChecked(),
                    "password": None if self.password_le.text() == "" else self.password_le.text(),
                    "active_account": self.active_account_cb.isChecked()
                    }
            response = make_http_request(self.https_session, "put", "users/" + str(self.user.get_id()), json = user)
            if response:
                QMessageBox.information(self, "Server response", response.text)
                self.accept()
                return
        self.reject()


    def delete_user(self):
        reply = QMessageBox.question(self, 'Delete user', f"Are you sure to delete {self.user.get_name()}?",
                                    QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
        if reply == QMessageBox.Yes:
            response = make_http_request(self.https_session, "delete", "users/" + str(self.user.get_id()))
            if response:
                QMessageBox.information(self, "Server response", response.text)
                self.accept()


class UserVehiclesWidget(QWidget):
    def __init__(self, user, https_session, parent = None):
        super().__init__(parent=parent)
        self.user = user
        self.https_session = https_session
        self.initUI()

    def initUI(self):
        self.vbox = QVBoxLayout()

        self.scroll_area = QScrollArea()
        self.scroll_area.setWidgetResizable(True)

        self.scroll_area_content = QWidget()
        self.vbox_grp = QVBoxLayout()

        UserVehicles.get_user_vehicles(self)            # return self.user_vehicles
        UserVehicles.get_vehicle_types(self)            # return self.vehicle_types

        self.user_vehicle_types_id = {vehicle.get_id_vehicle_type() for vehicle in self.user_vehicles}          # set comprehension; no order
        self.user_vehicle_types_id = sorted(self.user_vehicle_types_id)

        for vehicle_type_id in self.user_vehicle_types_id:
            try:
                vehicle_type_index = self.vehicle_types.index(VehicleType(id = vehicle_type_id))
            except ValueError:
                continue
            else:
                vehicle_type = self.vehicle_types[vehicle_type_index]

                vehicle_type_label = QLabel(f"Type: <b>{vehicle_type.get_name()}</b>")
                self.vbox_grp.addWidget(vehicle_type_label, 1, Qt.AlignBottom | Qt.AlignHCenter)

                vehicle_list = QListWidget()
                vehicle_list.setStyleSheet("margin-bottom: 20px;")
                vehicle_list.setFocus(Qt.MouseFocusReason)

                filtered_vehicles = [f"{vehicle.get_license_plate()} - {vehicle.get_brand()} - {vehicle.get_model()}" for vehicle in self.user_vehicles if vehicle.get_id_vehicle_type() == vehicle_type_id]

                vehicle_list.addItems(filtered_vehicles)
                self.vbox_grp.addWidget(vehicle_list, 1, Qt.AlignTop | Qt.AlignVCenter)

                self.scroll_area_content.setLayout(self.vbox_grp)
                self.scroll_area.setWidget(self.scroll_area_content)

        self.vbox.addWidget(self.scroll_area, 8)

        self.setLayout(self.vbox)
        self.show()


class UserManagement(QWidget):

    def __init__(self, https_session):
        super().__init__()
        self.https_session = https_session
        self.initUI()

    def initUI(self):

        user_info = ["Email", "Name", "Surname", "Password", "Wallet", "Disability", "Active account", "User category"]

        vbox_main = QVBoxLayout()

        title = "User management"

        title_lbl = QLabel(title)
        title_lbl.setStyleSheet("font-family: Ubuntu; font-size: 30px;")
        vbox_main.addWidget(title_lbl, 1, Qt.AlignTop | Qt.AlignHCenter)

        self.users_table = QTableWidget(0, len(user_info)+1)
        hheader_labels = list(user_info)
        hheader_labels.append("Details")
        self.users_table.setHorizontalHeaderLabels(hheader_labels)
        self.users_table.setEditTriggers(QAbstractItemView.NoEditTriggers)			# no editable

        self.users_table.horizontalHeader().setStretchLastSection(True);

        vbox_main.addWidget(self.users_table, 9)

        vbox_main.addStretch(1)

        self.setLayout(vbox_main)

        self.setWindowTitle(title)

        self.show()


    def get_user_categories(self):
        response = make_http_request(self.https_session, "get", "user_categories")
        if response.json():
            self.user_categories = [UserCategory(**user_category) for user_category in response.json()]
        else:
            self.user_categories = []


    def get_user_category(self, user):
        try:
            id_user_category = user.get_id_user_category()
            user_category_index = self.user_categories.index(UserCategory(id = id_user_category))
            return self.user_categories[user_category_index].get_name()
        except ValueError:
            return "N/A"


    def get_users(self):
        self.users_table.clearContents()
        self.users_table.setRowCount(0)

        admin_row = None

        response = make_http_request(self.https_session, "get", "users")
        if response.json():
            self.users = [User(**user) for user in response.json()]
        else:
            self.users = []

        for row, user in enumerate(self.users):

            user_methods = [user.get_email, user.get_name, user.get_surname, user.get_password, user.get_wallet,
                            user.get_disability, user.get_active_account, self.get_user_category]

            self.users_table.insertRow(row)

            for column in range(self.users_table.columnCount() - 1):
                method = user_methods[column]
                if column != len(user_methods) -1:
                    item = QTableWidgetItem(str(method()))
                    item.setTextAlignment(Qt.AlignCenter)
                    self.users_table.setItem(row, column, item)
                else:
                    user_category_name = method(user)
                    if user_category_name == "Admin":
                        admin_row = row
                    item = QTableWidgetItem(user_category_name)
                    item.setTextAlignment(Qt.AlignCenter)
                    self.users_table.setItem(row, column, item)


        for row in range(self.users_table.rowCount()):
            item = QPushButton("Show")
            self.users_table.setCellWidget(row, self.users_table.columnCount() - 1, item)
            item.clicked.connect(lambda _,row=row: self.show_user_details(row))

        if admin_row != None:
            self.users_table.removeRow(admin_row)


    def show_user_details(self, row):
        user = self.users[row]
        details_dialog = DetailsDialog(user, self.https_session, self)
        # if clicked OK button
        if details_dialog.exec_():
            self.get_users()


    def showEvent(self, event):
        self.get_user_categories()
        self.get_users()
