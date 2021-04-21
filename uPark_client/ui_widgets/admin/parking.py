#!/usr/bin/python
from PyQt5.QtWidgets import QWidget, QPushButton, QVBoxLayout, QLabel, QHBoxLayout, QDialog, QFormLayout, QDialogButtonBox, \
                            QLineEdit, QMessageBox, QSpacerItem, QSizePolicy, QProgressDialog, QTabWidget, QFormLayout, QScrollArea, \
                            QCheckBox, QComboBox, QListWidget, QAbstractItemView, QListWidgetItem

from PyQt5.QtCore import Qt, QThread, QObject, pyqtSignal
from convenience.server_apis import make_http_request
from convenience.loading_indicator import LoadingIndicator
from entities.user_category import UserCategory

from ..common.park import Park
from time import sleep


class ListWidgetItemNumeric(QListWidgetItem):
    def __lt__(self, other):
        try:
            return int(self.text()) < int(other.text())
        except Exception:
            return QListWidgetItem.__lt__(self, other)


class LoadingWorker(QObject):
    finished = pyqtSignal(str)
    https_session = None
    parking_lot_dict = None         # useful to add_worker
    parking_lot = None          # useful to delete_worker

    def run(self):
        pass

    def add_worker(self):
        response = make_http_request(self.https_session, "post", "parking_lots", json = self.parking_lot_dict)
        if response:
            self.finished.emit(response.json()["message"])
        else:
            self.finished.emit("")

    def delete_worker(self):
        response = make_http_request(self.https_session, "delete", "parking_lots/" + str(self.parking_lot.get_id()))
        if response:
            self.finished.emit(response.text)
        else:
            self.finished.emit("")


class AddDialog(QDialog):
    def __init__(self, https_session, vehicle_types, parent=None):
        super().__init__(parent=parent)
        self.https_session = https_session
        self.vehicle_types = vehicle_types

        self.setWindowTitle("Add Parking Lot")

        QBtn = QDialogButtonBox.Ok | QDialogButtonBox.Cancel
        self.buttonBox = QDialogButtonBox(QBtn)
        self.buttonBox.accepted.connect(self.form_accepted)
        self.buttonBox.rejected.connect(self.reject)

        layout = QVBoxLayout()

        layout.addWidget(QLabel("<b><i>Please, insert new parking lot info</i></b>"), 0, Qt.AlignCenter)
        layout.addWidget(QLabel("<b><i>Parking lot info: </i></b>"), 0, Qt.AlignCenter)

        form_layout = QFormLayout()

        self.name_le = QLineEdit()
        self.street_le = QLineEdit()
        self.num_parking_slots = QLineEdit()

        self.vehicle_types_cmb = QComboBox()
        self.vehicle_types_cmb.addItems([vehicle_type.get_name() for vehicle_type in self.vehicle_types])

        form_layout.addRow("<b>Name: </b> ", self.name_le)
        form_layout.addRow("<b>Street: </b> ",  self.street_le)
        form_layout.addRow("<b>#N. Parking slots: </b>",  self.num_parking_slots)
        form_layout.addRow("<b>Slots default vehicle type: </b>", self.vehicle_types_cmb)

        layout.addLayout(form_layout)
        layout.addSpacing(20)

        self.hbox = QHBoxLayout()
        self.hbox.addWidget(self.buttonBox)

        layout.addLayout(self.hbox)
        self.setLayout(layout)


    def form_accepted(self):
        index = self.vehicle_types_cmb.currentIndex()

        if index != -1:             # -1 means empty qcombobox (no items)
            vehicle_type_id = self.vehicle_types[index].get_id()

            if len(self.name_le.text()) == 0 or len(self.street_le.text()) == 0:
                QMessageBox.information(self, "uPark tip", "Form inputs cannot be blank!")
                return
            try:
                if int(self.num_parking_slots.text()) <= 0:
                    QMessageBox.information(self, "uPark tip", "Num Parking slots must be greater than zero!")
                    return
            except ValueError:
                QMessageBox.information(self, "uPark tip", "Num Parking slots must be whole number!")
                return
        else:
            QMessageBox.critical(self, "uPark", "No vehicle types available!")
            return

        self.show_loading()
        self.waiting_for_add_response(vehicle_type_id)


    def show_loading(self):
        self.buttonBox.setEnabled(False)
        self.loading_indicator = LoadingIndicator()
        self.hbox.insertWidget(1, self.loading_indicator)


    def add_parking_lot(self, message):
        # to reset the loader
        self.buttonBox.setEnabled(True)
        self.loading_indicator.setParent(None)

        if message:
            QMessageBox.information(self, "Server response", message)
            self.done(0)
        else:
            self.done(1)


    def waiting_for_add_response(self, vehicle_type_id):
        parking_lot = {
                        "name": self.name_le.text(),
                        "street": self.street_le.text(),
                        "num_parking_slots": int(self.num_parking_slots.text()),
                        "vehicle_type_id": vehicle_type_id
                        }

        self.thread = QThread()         # Create a QThread object
        self.worker = LoadingWorker()   # Create a worker object
        self.worker.run = self.worker.add_worker
        self.worker.https_session = self.https_session
        self.worker.parking_lot_dict = parking_lot
        self.worker.moveToThread(self.thread)       # Move worker to the thread

        # started signal: emitted from the associated thread when it starts executing
        self.thread.started.connect(self.worker.run)
        self.worker.finished.connect(self.thread.quit)
        self.worker.finished.connect(self.add_parking_lot)
        self.worker.finished.connect(self.worker.deleteLater)
        self.thread.finished.connect(self.thread.deleteLater)

        self.thread.start()


class DeleteDialog(QDialog):
    def __init__(self, https_session, parking_lot, parent=None):
        super().__init__(parent=parent)
        self.https_session = https_session
        self.parking_lot = parking_lot

        self.setWindowTitle("Delete Parking Lot")

        QBtn = QDialogButtonBox.Ok | QDialogButtonBox.Cancel
        self.buttonBox = QDialogButtonBox(QBtn)
        self.buttonBox.accepted.connect(self.show_loading)
        self.buttonBox.accepted.connect(self.waiting_for_delete_response)
        self.buttonBox.rejected.connect(self.reject)

        layout = QVBoxLayout()

        layout.addWidget(QLabel("<i>Are you sure to delete</i><b> " + parking_lot.get_name().capitalize() + " </b><i>parking lot?</i>"), 0, Qt.AlignCenter)
        layout.addSpacing(20)

        self.hbox = QHBoxLayout()
        self.hbox.addWidget(self.buttonBox)

        layout.addLayout(self.hbox)
        self.setLayout(layout)


    def show_loading(self):
        self.buttonBox.setEnabled(False)
        self.loading_indicator = LoadingIndicator()
        self.hbox.insertWidget(1, self.loading_indicator)


    def delete_parking_lot(self, message):
        #to reset the loader
        self.buttonBox.setEnabled(True)
        self.loading_indicator.setParent(None)

        if message:
            QMessageBox.information(self, "Server response", message)
            self.done(0)
        else:
            self.done(1)


    def waiting_for_delete_response(self):
            self.thread = QThread()         # Create a QThread object
            self.worker = LoadingWorker()   # Create a worker object
            self.worker.run = self.worker.delete_worker
            self.worker.https_session = self.https_session
            self.worker.parking_lot = self.parking_lot
            self.worker.moveToThread(self.thread)   # Move worker to the thread

            self.thread.started.connect(self.worker.run)
            self.worker.finished.connect(self.thread.quit)
            self.worker.finished.connect(self.delete_parking_lot)
            self.worker.finished.connect(self.worker.deleteLater)
            self.thread.finished.connect(self.thread.deleteLater)

            self.thread.start()


class UpdateDisabilityWidget(QWidget):
    def __init__(self, parking_lot, parent=None):
        super().__init__(parent=parent)
        self.parking_lot = parking_lot
        self.initUI()

    def initUI(self):
        self.setStyleSheet("QCheckBox {margin-left: 50%;}")
        vbox = QVBoxLayout()

        vbox.addWidget(QLabel("<i>Check the boxes for which you want to reserve the disability: </i>"))
        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)

        hbox = QHBoxLayout()
        hbox.addStretch(1)
        form_layout = QFormLayout()

        self.checkboxes_list = []
        self.checkbox_changed = False        #this variable becomes True if at least one checkbox changed its status

        for parking_slot in self.parking_lot.get_parking_slots():
            checkbox = QCheckBox()
            checkbox.setObjectName(str(parking_slot.get_number()))
            checkbox.setChecked(True if parking_slot.get_reserved_disability() else False)
            checkbox.stateChanged.connect(self.set_checkbox_changed)
            form_layout.addRow(f"Slot #{parking_slot.get_number()}", checkbox)
            self.checkboxes_list.append(checkbox)

        scroll_area_content = QWidget()
        hbox.addLayout(form_layout, 1)
        hbox.addStretch(1)
        scroll_area_content.setLayout(hbox)
        scroll_area.setWidget(scroll_area_content)
        vbox.addWidget(scroll_area)

        self.setLayout(vbox)
        self.show()


    def set_checkbox_changed(self):
        self.checkbox_changed = True


class UpdateCategoriesWidget(QWidget):
    def __init__(self, user_categories, categories_allowed, parent=None):
        super().__init__(parent=parent)
        self.user_categories = user_categories
        self.categories_allowed = categories_allowed
        self.initUI()

    def initUI(self):
        self.setStyleSheet("QCheckBox {margin-left: 50%;}")
        vbox = QVBoxLayout()

        vbox.addWidget(QLabel("<i>Check the boxes for each category you want to reserve the parking lot: </i>"))
        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)

        hbox = QHBoxLayout()
        hbox.addStretch(1)
        form_layout = QFormLayout()

        self.checkboxes_list = []
        self.checkbox_changed = False        #this variable becomes True if at least one checkbox changed its status

        for user_category in self.user_categories:
            checkbox = QCheckBox()
            checkbox.setObjectName(str(user_category.get_id()))
            checkbox.setChecked(True if user_category.get_id() in self.categories_allowed else False)
            checkbox.stateChanged.connect(self.set_checkbox_changed)
            form_layout.addRow(user_category.get_name(), checkbox)
            self.checkboxes_list.append(checkbox)

        scroll_area_content = QWidget()
        hbox.addLayout(form_layout, 1)
        hbox.addStretch(1)
        scroll_area_content.setLayout(hbox)
        scroll_area.setWidget(scroll_area_content)
        vbox.addWidget(scroll_area)

        self.setLayout(vbox)
        self.show()


    def set_checkbox_changed(self):
        self.checkbox_changed = True


class UpdateVehicleTypesWidget(QWidget):
    def __init__(self, vehicle_types, parking_lot, parent=None):
        super().__init__(parent=parent)
        self.vehicle_types = vehicle_types
        self.parking_lot = parking_lot
        self.items_moved = False
        self.initUI()

    def initUI(self):
        self.initialize()      # create a list for each vehicle type

        vbox_main = QVBoxLayout()
        vbox_main.addWidget(QLabel("<i>Move parking slots between categories:</i>"), 0, Qt.AlignHCenter)

        hbox = QHBoxLayout()

        #left
        vbox_to_assign_slots = QVBoxLayout()
        vbox_to_assign_slots.addWidget(QLabel("<i>Not assigned slots:</i>"))
        self.to_assign_slots_list = QListWidget()
        self.to_assign_slots_list.setSortingEnabled(True)
        self.to_assign_slots_list.setSelectionMode(QAbstractItemView.ExtendedSelection)

        vbox_to_assign_slots.addWidget(self.to_assign_slots_list)

        #center
        vbox_center = QVBoxLayout()
        move_left_btn = QPushButton("<")
        move_left_btn.clicked.connect(lambda: self.move_items(self.actual_slots_list, self.to_assign_slots_list))
        move_right_btn = QPushButton(">")
        move_right_btn.clicked.connect(lambda: self.move_items(self.to_assign_slots_list, self.actual_slots_list))

        vbox_center.addWidget(move_left_btn)
        vbox_center.addWidget(move_right_btn)

        #right
        vbox_actual_slots = QVBoxLayout()
        vbox_actual_slots.addWidget(QLabel("<i> Slots assigned to:"))
        self.actual_slots_list = QListWidget()
        self.actual_slots_list.setSortingEnabled(True)
        self.actual_slots_list.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.vehicle_types_cmb = QComboBox()
        self.vehicle_types_cmb.currentIndexChanged.connect(self.update_vehicle_type_items)
        self.vehicle_types_cmb.addItems([vehicle_type.get_name() for vehicle_type in self.vehicle_types])

        vbox_actual_slots.addWidget(self.vehicle_types_cmb)
        vbox_actual_slots.addWidget(self.actual_slots_list)

        hbox.addLayout(vbox_to_assign_slots)
        hbox.addLayout(vbox_center)
        hbox.addLayout(vbox_actual_slots)

        vbox_main.addLayout(hbox)
        self.setLayout(vbox_main)

        self.show()


    def initialize(self):      # create a list for each vehicle type and the list "to assign"  {"to_assing": [], "vehicle_type": [], ... }
        self.parking_slots = {"to_assign": []}
        for vehicle_type in self.vehicle_types:
            self.parking_slots[vehicle_type.get_id()] = [slot for slot in self.parking_lot.get_parking_slots() if slot.get_id_vehicle_type() == vehicle_type.get_id()]


    def update_vehicle_type_items(self, index):
        self.actual_slots_list.clear()
        for slot in self.parking_slots[self.vehicle_types[index].get_id()]:
            self.actual_slots_list.addItem(ListWidgetItemNumeric(str(slot.get_number())))


    def move_items(self, start_list, end_list):
        self.items_moved = True
        selected_indexes = start_list.selectedIndexes()

        for index in reversed(selected_indexes):
            selected_vehicle_type_id = self.vehicle_types[self.vehicle_types_cmb.currentIndex()].get_id()
            if start_list == self.actual_slots_list:     # left <- right
                self.parking_slots["to_assign"].append(self.parking_slots[selected_vehicle_type_id][index.row()])
                del self.parking_slots[selected_vehicle_type_id][index.row()]
            else:     # left -> right
                self.parking_slots[selected_vehicle_type_id].append(self.parking_slots["to_assign"][index.row()])
                del self.parking_slots["to_assign"][index.row()]

            end_list.addItem(ListWidgetItemNumeric(start_list.takeItem(index.row())))      # remove the item from the right list and insert it into the left one


class EditDialog(QDialog):
    def __init__(self, https_session, parking_lot, vehicle_types, parent=None):
        super().__init__(parent=parent)
        self.https_session = https_session
        self.parking_lot = parking_lot
        self.vehicle_types = vehicle_types

        self.setWindowTitle("Edit dialog")

        QBtn = QDialogButtonBox.Ok | QDialogButtonBox.Cancel
        buttonBox = QDialogButtonBox(QBtn)
        buttonBox.accepted.connect(self.edit_parking_lot)
        buttonBox.rejected.connect(self.reject)

        layout = QVBoxLayout()

        layout.addWidget(QLabel("<b><i>Please, edit selected parking lot info</i></b>"), 0, Qt.AlignCenter)
        layout.addWidget(QLabel("<b><i>Parking lot info: </i></b>"), 0, Qt.AlignCenter)

        tab_widget = QTabWidget()
        update_name_widget = QWidget()

        # 1) name: hbox(Qlabel, QLineEdit)
        hbox = QHBoxLayout()
        self.name_le = QLineEdit()
        hbox.addWidget(QLabel("Name: "))
        hbox.addWidget(self.name_le)
        update_name_widget.setLayout(hbox)

        tab_widget.addTab(update_name_widget, "Name")

        # 2) slots reserved for disability using checkboxes {"slot_number":true/false}
        self.update_disability_widget = UpdateDisabilityWidget(self.parking_lot)
        tab_widget.addTab(self.update_disability_widget, "Disability slots")

        # 3) slots reserved per vehicle types
        self.update_vehicle_types_widget = UpdateVehicleTypesWidget(self.vehicle_types, self.parking_lot)
        tab_widget.addTab(self.update_vehicle_types_widget, "Vehicle Types")

        # 4) categories allowed {"user_categories" : true/false}
        response_user_categories = make_http_request(self.https_session, "get", "user_categories", show_messagebox = False)

        if response_user_categories:
            user_categories = [UserCategory(**user_category) for user_category in response_user_categories.json() if user_category["name"] != "Admin"]
            response_categories_allowed = make_http_request(self.https_session, "get", "parking_lots/" + str(self.parking_lot.get_id()) + "/categories_allowed", show_messagebox = False)

            if response_categories_allowed:
                categories_allowed = [user_category["id_user_category"] for user_category in response_categories_allowed.json()]
        else:
            user_categories, categories_allowed = [], []

        self.update_categories_widget = UpdateCategoriesWidget(user_categories, categories_allowed)
        tab_widget.addTab(self.update_categories_widget, "User categories")

        layout.addWidget(tab_widget)
        layout.addSpacing(20)
        layout.addWidget(buttonBox)

        self.setLayout(layout)


    def edit_parking_lot(self):
        # The parking lot edit option consists of 4 parts:
        # 1) name, 2) slots_reserved_disability, 3) slots_reserved_per_vehicle_types, 4) categories_allowed
        parking_lot_info = {}

        # Server endpoint expects a json with all the key elements even if they are none, "null"
        parking_lot_info["name"] = None
        parking_lot_info["slots_reserved_disability"] = None
        parking_lot_info["slots_reserved_per_vehicle_types"] = None
        parking_lot_info["categories_allowed"] = None

        # 1) name
        if self.name_le.text():                           # if self.name_le is empty == ""
            parking_lot_info["name"] = self.name_le.text()

        # 2) slots_reserved_disability
        if self.update_disability_widget.checkbox_changed:        # if at least one checkbox changed its status
            slots_diability_info = {checkbox.objectName() : checkbox.isChecked() for checkbox in self.update_disability_widget.checkboxes_list}
            parking_lot_info["slots_reserved_disability"] = slots_diability_info

        # 3) slots_reserved_per_vehicle_types - reading vehicle_types, for-each vehicle type(key)     "1", "2" in  {"1":[number,], "2":[], ....}
        if self.update_vehicle_types_widget.to_assign_slots_list.count() == 0 and self.update_vehicle_types_widget.items_moved:
            parking_slots_dict = self.update_vehicle_types_widget.parking_slots
            del parking_slots_dict["to_assign"]

            dict = {}
            for slot_key in parking_slots_dict:
                dict[slot_key] = [slot.get_number() for slot in parking_slots_dict[slot_key]]

            parking_lot_info["slots_reserved_per_vehicle_types"] = dict
        elif self.update_vehicle_types_widget.to_assign_slots_list.count() != 0:
            QMessageBox.information(self, "uPark tip", "Please, assign all slots in 'Not assigned slots' section first!")
            return

        # 4) categories_allowed
        if self.update_categories_widget.checkbox_changed:
            category_allowed_info = {checkbox.objectName() : checkbox.isChecked() for checkbox in self.update_categories_widget.checkboxes_list}
            parking_lot_info["categories_allowed"] = category_allowed_info

        response = make_http_request(self.https_session, "put", "parking_lots/" + str(self.parking_lot.get_id()), json = parking_lot_info)
        if response:
            QMessageBox.information(self, "Server response", response.text)
            self.done(0)
        else:
            self.done(1)


class Parking(Park):
    def __init__(self, https_session, user):
        super().__init__(https_session, user)
        self.initUI()

    def initUI(self):
        hbox = QHBoxLayout()
        add_btn = QPushButton("+")
        add_btn.clicked.connect(self.show_add_dialog)

        delete_btn = QPushButton("-")
        delete_btn.clicked.connect(self.show_delete_dialog)

        edit_btn = QPushButton("Edit")
        edit_btn.clicked.connect(self.show_edit_dialog)

        hbox.addWidget(add_btn)
        hbox.addWidget(delete_btn)
        hbox.addWidget(edit_btn)

        self.pl_vbox.addLayout(hbox)
        spacer_item = QSpacerItem(1, 1, vPolicy = QSizePolicy.Expanding)
        self.pl_vbox.addSpacerItem(spacer_item)


    def show_delete_dialog(self):
        if self.parking_lots:
            delete_dialog = DeleteDialog(self.https_session, self.parking_lot, self)
            if delete_dialog.exec_() == 0:
                self.get_parking_lots()
        else:
            return


    def show_add_dialog(self):
        add_dialog = AddDialog(self.https_session, self.vehicle_types, self)
        if add_dialog.exec_() == 0:
            self.get_parking_lots()
        else:
            return


    def show_edit_dialog(self):
        if self.parking_lot:
            add_dialog = EditDialog(self.https_session, self.parking_lot, self.vehicle_types, self)
            if add_dialog.exec_() == 0:
                self.get_parking_lots()
            else:
                return
