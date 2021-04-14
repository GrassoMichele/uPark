#!/usr/bin/python
from PyQt5.QtWidgets import QWidget, QLabel, QMessageBox, QAbstractItemView, QPushButton, \
                            QVBoxLayout, QHBoxLayout, QDialog, QDialogButtonBox, QListWidget, QWidgetItem, \
                            QFormLayout, QLineEdit, QComboBox, QScrollArea

from PyQt5.QtCore import Qt


from convenience_functions.server_apis import make_http_request
from entities.user import User

from entities.vehicle import Vehicle
from entities.vehicle_type import VehicleType

class AddVehicleDialog(QDialog):
    def __init__(self, https_session, id_user, vehicle_types, parent=None):
        super().__init__(parent=parent)

        self.vehicle_types = vehicle_types
        self.https_session = https_session
        self.id_user = id_user

        self.setWindowTitle("Add Vehicle")

        QBtn = QDialogButtonBox.Ok | QDialogButtonBox.Cancel

        self.buttonBox = QDialogButtonBox(QBtn)
        self.buttonBox.accepted.connect(self.add_vehicles)        # if accepted add vehicle
        self.buttonBox.rejected.connect(self.reject)

        self.layout = QVBoxLayout()

        self.layout.addWidget(QLabel("<b><i>Please, insert vehicle info</i></b>"), 0, Qt.AlignCenter)
        self.layout.addWidget(QLabel("<b><i>Vehicle info: </i></b>"), 0, Qt.AlignCenter)

        self.formLayout = QFormLayout()
        self.license_plate_le = QLineEdit()
        self.brand_le = QLineEdit()
        self.model_le = QLineEdit()

        self.vehicle_type_cmb = QComboBox()

        self.formLayout.addRow("License plate: ", self.license_plate_le)
        self.formLayout.addRow("Brand: ",  self.brand_le )
        self.formLayout.addRow("Model: ",  self.model_le)
        self.formLayout.addRow("Vehicle types: ",  self.vehicle_type_cmb)

        self.layout.addLayout(self.formLayout)

        self.vehicle_type_cmb.addItems([f"{vehicle_type.get_name()}" for vehicle_type in self.vehicle_types])

        if self.vehicle_type_cmb.count() == 0:
            self.buttonBox = QDialogButtonBox(QDialogButtonBox.Cancel)
            self.buttonBox.setCenterButtons(True)
            self.buttonBox.rejected.connect(self.reject)


        self.layout.addWidget(self.vehicle_type_cmb)

        self.layout.addSpacing(20)
        self.layout.addWidget(self.buttonBox)

        self.setLayout(self.layout)


    def add_vehicles(self):
        try:
            selected_vehicle_type_index = self.vehicle_types.index(VehicleType(name = self.vehicle_type_cmb.currentText()))
        except ValueError:
            QMessageBox.critical(self, "Alert", "Server error on vehicle types")
            self.reject()
        else:
            selected_vehicle_type = self.vehicle_types[selected_vehicle_type_index]

            license_plate = self.license_plate_le.text()

            if len(license_plate) != 0:

                vehicle =   {
                    "license_plate": self.license_plate_le.text(),
                    "brand": self.brand_le.text(),
                    "model": self.model_le.text(),
                    "id_vehicle_type": selected_vehicle_type.get_id()
                }

                response = make_http_request(self.https_session, "post", "users/" + str(self.id_user) + "/vehicles", json = vehicle )
                if response.json():
                    QMessageBox.information(self, "Server response", response.json()["message"])
                    #close the dialog and sets its result code to 0, emit the finished signal
                    self.done(0)
                else:
                    self.close()
            else:
                QMessageBox.information(self, "uPark tip", "Add a license plate first!")


    def hideEvent(self, event):
        self.license_plate_le.clear()
        self.brand_le.clear()
        self.model_le.clear()
        self.vehicle_type_cmb.setCurrentIndex(0)


class UserVehicles(QWidget):

    def __init__(self, https_session, user):
        super().__init__()
        self.https_session = https_session
        self.user = user
        self.initUI()

    def initUI(self):
        self.vbox = QVBoxLayout()

        text = QLabel("Your vehicles")
        text.setStyleSheet("font-family: Ubuntu; font-size: 30px;")
        self.vbox.addWidget(text, 1, Qt.AlignTop | Qt.AlignHCenter)

        self.setLayout(self.vbox)

        self.setWindowTitle("Your vehicles")
        self.show()


    def show_vehicles(self):
        self.scroll_area = QScrollArea()
        self.scroll_area.setWidgetResizable(True)
        #self.scroll_area.setFixedHeight(800)

        self.scroll_area_content = QWidget()
        self.vbox_grp = QVBoxLayout()

        self.qlistwidgets_list = []

        self.get_user_vehicles()
        self.get_vehicle_types()

        self.user_vehicle_types_id = {vehicle.get_id_vehicle_type() for vehicle in self.user_vehicles}

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
                vehicle_list.setFocus(Qt.MouseFocusReason)

                filtered_vehicles = [f"{vehicle.get_license_plate()} - {vehicle.get_brand()} - {vehicle.get_model()}" for vehicle in self.user_vehicles if vehicle.get_id_vehicle_type() == vehicle_type_id]

                vehicle_list.addItems(filtered_vehicles)
                self.vbox_grp.addWidget(vehicle_list, 1, Qt.AlignTop | Qt.AlignVCenter)

                delete_button = QPushButton("Delete")
                delete_button.setStyleSheet("padding: 10px 30px; margin-bottom: 30px;")
                delete_button.clicked.connect(self.delete_vehicle)

                self.vbox_grp.addWidget(delete_button, 1, Qt.AlignTop | Qt.AlignHCenter)

                self.scroll_area_content.setLayout(self.vbox_grp)
                self.scroll_area.setWidget(self.scroll_area_content)

                self.qlistwidgets_list.append((vehicle_type_id, vehicle_list, delete_button))
                #print(vehicle_list.currentItem().text())

        #add a scroll area
        self.vbox.addWidget(self.scroll_area, 8)

        #"add vehicle" button
        add_button = QPushButton("Add Vehicle")
        add_button.setStyleSheet("padding: 10px 30px; margin-top: 20px;")
        add_button.clicked.connect(self.show_vehicle_dialog)
        self.vbox.addWidget(add_button, 1, Qt.AlignTop)


    def mouseDoubleClickEvent(self, event):
        for i, tuple in enumerate(self.qlistwidgets_list):
            if tuple[1].hasFocus() and tuple[1].currentItem() != None:
                tuple[1].clearSelection()
                tuple[1].setCurrentItem(None)


    def get_user_vehicles(self):
        response = make_http_request(self.https_session, "get", "users/" + str(self.user.get_id()) + "/vehicles")
        if response.json():
            self.user_vehicles = [Vehicle(**user_vehicle) for user_vehicle in response.json()]
        else:
            self.user_vehicles = []


    def get_vehicle_types(self):
        response = make_http_request(self.https_session, "get", "vehicle_types")
        if response.json():
            self.vehicle_types = [VehicleType(**vehicle_type) for vehicle_type in response.json()]
        else:
            self.vehicle_types = []


    def show_vehicle_dialog(self):
        self.get_vehicle_types()
        id_user = self.user.get_id()
        get_vehicle_dialog = AddVehicleDialog(self.https_session, id_user, self.vehicle_types)   #parent = None

        if get_vehicle_dialog.exec_() == 0:
            self.reload_widget()
        else:
            pass


    def delete_vehicle(self):
        id_vehicle_to_delete = 0
        for i, tuple in enumerate(self.qlistwidgets_list):
            if tuple[2] == self.sender() and tuple[1].currentItem() != None:
                license_plate = tuple[1].currentItem().text().split(" ")[0]
                id_vehicle = [vehicle.get_id() for vehicle in self.user_vehicles if license_plate == vehicle.get_license_plate()][0]
                #print(id_vehicle)
                id_vehicle_to_delete = id_vehicle

        if id_vehicle_to_delete != 0:
            # qmessabox to confirm delete
            reply = QMessageBox.question(self, 'Delete vehicle',
                                         "Are you sure to delete your vehicle?", QMessageBox.Yes |
                                         QMessageBox.No, QMessageBox.No)
            if reply == QMessageBox.Yes:
                response = make_http_request(self.https_session, "delete", "users/" + str(self.user.get_id()) + "/vehicles/" + str(id_vehicle_to_delete))
                if response:
                    QMessageBox.information(self, "Server response", response.text)
                    self.reload_widget()
            else:
                return
        else:
            QMessageBox.information(self, "uPark tip:", "Select a vehicle to delete first!")


    def reload_widget(self):
        #print("init: verticalbox element #n --> ", self.vbox.count())
        for i in reversed(range(self.vbox.count())):
            #print(self.vbox.itemAt(i))
            if i!=0:                          #i=0 is the QLabel of the windowtitle in the QVBoxLayout
                if isinstance(self.vbox.itemAt(i),QWidgetItem):
                    self.vbox.itemAt(i).widget().setParent(None)
        self.show_vehicles()
        #print("after show: verticalbox element #n -->", self.vbox.count())


    def showEvent(self, event):
        self.reload_widget()
