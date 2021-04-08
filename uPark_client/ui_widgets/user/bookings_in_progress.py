#!/usr/bin/python

from PyQt5.QtWidgets import QWidget, QLabel, QMessageBox, QTableWidget, QAbstractItemView, QPushButton, \
                            QVBoxLayout, QDialog, QDialogButtonBox, QTableWidgetItem, QHeaderView

from PyQt5.QtCore import Qt

from datetime import timedelta, datetime, timezone

from convenience_functions.server_apis import make_http_request
from convenience_functions.datetime_management import datetime_UTC_to_local
from entities.user import User
from entities.parking_lot import ParkingLot
from entities.parking_slot import ParkingSlot

from entities.booking import Booking

from entities.vehicle import Vehicle

class DetailsDialog(QDialog):
    def __init__(self, booking, vehicle_info, parking_slot_info, https_session, parent = None):
        super().__init__(parent=parent)

        booking_info_dialog = [("Entry time:", booking.get_datetime_start()),
                                ("Exit time:", booking.get_datetime_end()),
                                ("Amount:", booking.get_amount()),
                                ("", ""),
                                ("Vehicle info:", ""),
                                ("License plate:", vehicle_info.get_license_plate()),
                                ("Brand:", vehicle_info.get_brand()),
                                ("Model:", vehicle_info.get_model()),
                                ("", ""),
                                ("Parking slot info:", ""),
                                ("Parking lot name:", parking_slot_info[0]),
                                ("Parking lot street:", parking_slot_info[1]),
                                ("Parking slot number:",parking_slot_info[2]),
                                ("", ""),
                                ("Note:", booking.get_note())]

        self.setWindowTitle("Booking details")

        self.buttonBox = QDialogButtonBox(QDialogButtonBox.Ok)

        self.buttonBox.addButton("Delete", QDialogButtonBox.RejectRole)

        self.buttonBox.accepted.connect(self.accept)        # QDialog slot
        self.buttonBox.rejected.connect(lambda: self.delete_booking(https_session,booking))

        self.layout = QVBoxLayout()
        self.layout.addWidget(QLabel("<b><i>Booking info: </i></b>"), 0, Qt.AlignCenter)

        for info in booking_info_dialog:
            self.layout.addWidget(QLabel(f"<b>{info[0]} </b>" + str(info[1])))

        self.layout.addSpacing(20)

        self.layout.addWidget(self.buttonBox)
        self.setLayout(self.layout)


    def delete_booking(self, https_session, booking):
        response = make_http_request(https_session, "delete", "bookings/" + str(booking.get_id()))
        if response:
            QMessageBox.information(self, "Server response", response.text)
            self.reject()


class BookingsInProgress(QWidget):

    def __init__(self, https_session, user):
        super().__init__()
        self.https_session = https_session
        self.user = user
        self.initUI()

    def initUI(self):

        booking_info = ["Datetime start", "Datetime end", "License plate", "Parking lot", "Slot number"]

        vbox = QVBoxLayout()

        text = QLabel("Bookings in progress")
        text.setStyleSheet("font-family: Ubuntu; font-size: 30px;")
        vbox.addWidget(text, 1, Qt.AlignTop | Qt.AlignHCenter)
        self.user_bookings_table = QTableWidget(0, len(booking_info)+1)
        hheader_labels = list(booking_info)
        hheader_labels.append("Details")
        self.user_bookings_table.setHorizontalHeaderLabels(hheader_labels)
        self.user_bookings_table.setEditTriggers(QAbstractItemView.NoEditTriggers)			# no editable

        self.user_bookings_table.horizontalHeader().setStretchLastSection(True);
        self.user_bookings_table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch);

        vbox.addWidget(self.user_bookings_table, 5)

        vbox.addStretch(5)
        #vbox.setStretchFactor(self.user_bookings_table, 5)

        self.setLayout(vbox)

        self.get_user_vehicles()
        self.get_parking_lots()
        self.get_user_bookings()

        self.setWindowTitle("Bookings in progress")
        self.show()



    def get_user_vehicles(self):
        response = make_http_request(self.https_session, "get", "users/" + str(self.user.get_id()) + "/vehicles")
        if response:
            self.user_vehicles = [Vehicle(**user_vehicle) for user_vehicle in response.json()]
        else:
            self.user_vehicles = []


    def get_vehicle_info(self, id_vehicle, only_license_plate = False):
        try:
            vehicle_index = self.user_vehicles.index(Vehicle(id = id_vehicle))
        except ValueError:
            return
        else:
            vehicle = self.user_vehicles[vehicle_index]
            if only_license_plate:
                return vehicle.get_license_plate()
            else:
                return vehicle


    def get_parking_lots(self):
        response = make_http_request(self.https_session, "get", "parking_lots")
        if response:
            self.parking_lots = [ParkingLot(**parking_lots) for parking_lots in response.json()]
        else:
            return

        if self.parking_lots:                       # if list is empty -> false
            #print(self.parking_lots)
            for parking_lot in self.parking_lots:
                # get info on parking slots of parking lot
                response = make_http_request(self.https_session, "get", "parking_lots/" + str(parking_lot.get_id()) + "/parking_slots")
                if response:
                    parking_lot.set_parking_slots([ParkingSlot(**parking_slot) for parking_slot in response.json()])


    def get_parking_slot_info(self, id_parking_slot):
        for parking_lot in self.parking_lots:
            try:
                parking_lot_slots = parking_lot.get_parking_slots()
                slot_index = parking_lot_slots.index(ParkingSlot(id = id_parking_slot))
                slot = parking_lot_slots[slot_index]
                # get parking lot name, street and parking slot number
                return (parking_lot.get_name(), parking_lot.get_street(), slot.get_number())
            except ValueError:
                continue


    def get_user_bookings(self):
        self.user_bookings_table.clearContents()
        self.user_bookings_table.setRowCount(0)

        now = datetime.now(timezone.utc)
        #print(now, now.date())

        response = make_http_request(self.https_session, "get", "bookings", params = {"since":now.date(), "until":None, "id_user":self.user.get_id()})
        if response.json():
            self.bookings = [Booking(**booking) for booking in response.json()]

            for i, booking in enumerate(self.bookings):
                booking_methods = [booking.get_datetime_start, booking.get_datetime_end, self.get_vehicle_info, self.get_parking_slot_info]

                # from UTC to local timezone
                booking.set_datetime_start(datetime_UTC_to_local(booking.get_datetime_start()))
                booking.set_datetime_end(datetime_UTC_to_local(booking.get_datetime_end()))

                self.user_bookings_table.insertRow(i)
                for j in range(self.user_bookings_table.columnCount()):
                    method = booking_methods[j]
                    if j not in [2,3,4]:                    # 2 = vehicle_info, 3,4 = parking lot name and parking slot number
                        item = QTableWidgetItem(method())
                        item.setTextAlignment(Qt.AlignCenter)
                        self.user_bookings_table.setItem(i, j, item)
                    elif j == 2:
                        item = QTableWidgetItem(method(booking.get_id_vehicle(), only_license_plate = True))
                        item.setTextAlignment(Qt.AlignCenter)
                        self.user_bookings_table.setItem(i, j, item)
                    else:
                        slot_info = method(booking.get_id_parking_slot())
                        item = QTableWidgetItem(slot_info[0])
                        item.setTextAlignment(Qt.AlignCenter)
                        self.user_bookings_table.setItem(i, j, item)
                        item = QTableWidgetItem(str(slot_info[2]))
                        item.setTextAlignment(Qt.AlignCenter)
                        self.user_bookings_table.setItem(i, j+1, item)
                        break


        for row in range(self.user_bookings_table.rowCount()):
            item = QPushButton("Show")
            self.user_bookings_table.setCellWidget(row, 5, item)
            item.clicked.connect(lambda _,row=row: self.show_booking_details(row))      # da fare collegamento a dialog


    def show_booking_details(self, row):
        booking = self.bookings[row]
        vehicle_info = self.get_vehicle_info(booking.get_id_vehicle())
        parking_slot_info = self.get_parking_slot_info(booking.get_id_parking_slot())
        details_dialog = DetailsDialog(booking, vehicle_info, parking_slot_info, self.https_session, self)
        # if clicked OK button
        if details_dialog.exec_():
            pass
        else:
            self.get_user_bookings()
