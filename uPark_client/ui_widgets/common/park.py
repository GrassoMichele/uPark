#!/usr/bin/python

from PyQt5.QtWidgets import QWidget, QCalendarWidget, QLabel, QApplication, QHBoxLayout, QMessageBox, QListWidget, \
                             QVBoxLayout, QTableWidget, QAbstractItemView, QDesktopWidget, QTableWidgetItem, \
                             QDialog, QDialogButtonBox, QComboBox
from PyQt5.QtCore import QDate, Qt, QObject, pyqtSignal, QMetaMethod
from PyQt5.QtGui import QBrush, QColor

import sys
import requests

from datetime import timedelta, datetime

from convenience_functions.server_apis import make_http_request, user_is_admin
from convenience_functions.datetime_management import datetime_to_UTC, datetime_UTC_to_local
from entities.user import User
from entities.user_category import UserCategory
from entities.parking_lot import ParkingLot
from entities.parking_slot import ParkingSlot
from entities.vehicle_type import VehicleType
from entities.booking import Booking
from entities.hourly_rate import HourlyRate
from entities.vehicle import Vehicle


class Table_signal(QObject):
    message = pyqtSignal(int,str, str)      # parking_slot_number, start_time, end_time


class Bookings_table(QTableWidget):

    def __init__(self, rows, columns, parent, signal):
        super().__init__(rows, columns, parent)

        self.signal = signal
        self.init_UI()

    def init_UI(self):
        self.setEditTriggers(QAbstractItemView.NoEditTriggers)			# no editable
        self.horizontalHeader().setDefaultSectionSize(65)               # column width
        self.horizontalHeader().setDefaultAlignment(Qt.AlignLeft)

        # free_parking slots are white
        self.cells_colors = {
                            "user_bookings" : "#FFA200",
                            "other_bookings" : "#A8ADAF",
                            "bookings_separation": "#D2D5D7",
                            "disability": "#138DF3"
                            }

        # table column names
        self.quarters_of_hour = [str(timedelta(minutes=15)*i) for i in range(97)]                           # index 96 = 1 day, 0:00:00 (24:00:00)
        for i, q in enumerate(self.quarters_of_hour):
        	q_split = q.split(":")
        	item = str("0" + q_split[0] if len(q_split[0]) == 1 else q_split[0]) + ":" + str(q_split[1])      # e.g. 1:00:00 -> 01:00
        	self.quarters_of_hour[i] = item

        self.setHorizontalHeaderLabels(self.quarters_of_hour)

        # move scrollbar slider to actual time slot
        now_time = datetime.now().time()
        hour = int(now_time.strftime("%H"))
        minutes = int(now_time.strftime("%M"))
        quarter_of_hour_idx = hour * 4 + minutes//15
        self.horizontalScrollBar().setValue(quarter_of_hour_idx)

        self.show()


    def mouseReleaseEvent(self, event):
        super().mouseReleaseEvent(event)                        # base behaviour on mouse release (e.g. clear selection)

        if event.button() == Qt.LeftButton:                     # Release event only if done with left button
            indexSelection = [(item.row() , item.column()) for item in self.selectedIndexes()]

            print("Selection info: ", len(indexSelection), indexSelection)

            if len(indexSelection) != 0:                        # at least one cell selected

                # check on selection validity, only with multiple cells selected
                error = False
                for i in range(1, len(indexSelection)):
                    # if selection on different rows or non contiguous cells
                    if (indexSelection[i][0] != indexSelection[i-1][0]) | indexSelection[i][1] not in [indexSelection[i-1][1] + 1, indexSelection[i-1][1] - 1]:
                        error = True
                if error == True:
                    QMessageBox.critical(self, "Error", "Not a valid selection!")
                    return

                # if no errors occured, proceed
                # take the first and last cell in selection
                parking_slot_number = indexSelection[0][0] + 1            # parking slot numbers start from one
                booking_start_time = self.quarters_of_hour[indexSelection[0][1]]        # format xx:xx              ADD :00?
                #print(booking_start_time, type(booking_start_time))
                booking_end_time = self.quarters_of_hour[indexSelection[-1][1] + 1]
                #print(booking_end_time, type(booking_end_time))
                self.signal.message.emit(parking_slot_number, booking_start_time, booking_end_time)

        self.clearSelection()


    def item_paint(self, row_index, col_index, color, deactivate = True):

        self.setItem(row_index, col_index, QTableWidgetItem(""))
        cell = self.item(row_index, col_index)

        cell.setBackground(QBrush(QColor(color)))

        # colors handling
        if color == self.cells_colors["other_bookings"]:
            cell.setToolTip("Other user booking.")
        elif color == self.cells_colors["user_bookings"]:
            cell.setToolTip("Your booking.")
        elif color == self.cells_colors["bookings_separation"]:
            cell.setToolTip("Time interval between one booking and another")
        elif color == self.cells_colors["disability"]:
            cell.setToolTip("Parking slot reserved for disability.")

        if deactivate == True:                                  # always true unless when disable user
            cell.setFlags(Qt.NoItemFlags)                       # cell no selectable, editable, ecc.



class BookingDialog(QDialog):
    def __init__(self, start, end, duration, parking_slot, user_vehicles, slot_vehicle_type_id, amount, parent=None):
        super().__init__(parent=parent)

        self.setWindowTitle("Booking")

        QBtn = QDialogButtonBox.Ok | QDialogButtonBox.Cancel

        self.buttonBox = QDialogButtonBox(QBtn)
        self.buttonBox.accepted.connect(self.accept)        # QDialog slot
        self.buttonBox.rejected.connect(self.reject)

        self.layout = QVBoxLayout()
        self.layout.addWidget(QLabel("<b><i>Booking info: </i></b>"), 0, Qt.AlignCenter)
        self.layout.addWidget(QLabel("<b>Start: </b>" + start))
        self.layout.addWidget(QLabel("<b>End: </b>" + end))
        self.layout.addWidget(QLabel("<b>Duration: </b>" + str(duration)))
        self.layout.addWidget(QLabel("<b>Parking slot: </b>" + str(parking_slot)))

        self.layout.addWidget(QLabel("<b>Vehicle: </b>"))
        # filter on user vehicles
        self.user_vehicles_cmb = QComboBox()
        self.user_vehicles_cmb.addItems([f"{vehicle.get_license_plate()} - {vehicle.get_brand()} - {vehicle.get_model()}" for vehicle in user_vehicles if vehicle.get_id_vehicle_type() == slot_vehicle_type_id])
        if self.user_vehicles_cmb.count() == 0:
            self.buttonBox = QDialogButtonBox(QDialogButtonBox.Cancel)
            self.buttonBox.setCenterButtons(True)
            self.buttonBox.rejected.connect(self.reject)

        self.layout.addWidget(self.user_vehicles_cmb)

        self.layout.addWidget(QLabel("<b>Amount: " + amount + "</b>"))

        self.layout.addSpacing(20)

        self.layout.addWidget(self.buttonBox)
        self.setLayout(self.layout)


    def selected_vehicle(self):
        return self.user_vehicles_cmb.currentText()


class Park(QWidget):

    def __init__(self, https_session, user):
        super().__init__()
        self.https_session = https_session
        self.user = user
        Park.initUI(self)

    def initUI(self):
        self.table_signal = Table_signal()
        self.table_signal.message.connect(self.make_booking)

        vbox_main = QVBoxLayout(self)

        table_hbox = QHBoxLayout()
        self.tableWidget = Bookings_table(50,96, self, self.table_signal)           # 50 should be the parking lot number of slots
        table_hbox.addWidget(self.tableWidget, 10)

        self.vehicle_types_table = QTableWidget()
        self.vehicle_types_table.setEditTriggers(QAbstractItemView.NoEditTriggers)			# no editable
        self.vehicle_types_table.setColumnCount(1)
        self.vehicle_types_table.setHorizontalHeaderLabels(["Reserved:   "])
        self.vehicle_types_table.verticalHeader().hide()                                    # possible modification with another column which items are parking slot numbers
        self.vehicle_types_table.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        table_hbox.addWidget(self.vehicle_types_table, 1)

        # synchronize scrolling between slots table and vehicle types table
        self.tableWidget.verticalScrollBar().valueChanged.connect(self.vehicle_types_table.verticalScrollBar().setValue);
        self.vehicle_types_table.verticalScrollBar().valueChanged.connect(self.tableWidget.verticalScrollBar().setValue);

        self.pl_vbox = QVBoxLayout()
        self.pl_vbox.setSpacing(0)
        name_lbl = QLabel("Parking lots: ")
        name_lbl.setStyleSheet("font-size: 14px;")
        self.pl_vbox.addWidget(name_lbl, 1, Qt.AlignBottom)
        self.parking_lots_list = QListWidget()
        self.parking_lots_list.setStyleSheet("font: 11pt Arial;")
        self.parking_lots_list.setMinimumHeight(150)
        self.parking_lots_list.currentItemChanged.connect(lambda : self.get_bookings(self.parking_lots[self.parking_lots_list.currentRow()], True))
        self.pl_vbox.addWidget(self.parking_lots_list, 2, Qt.AlignTop)

        bottom_hbox = QHBoxLayout()
        bottom_hbox.addStretch(1)
        bottom_hbox.addLayout(self.pl_vbox, 1)
        bottom_hbox.addStretch(1)

        self.cal = QCalendarWidget(self)
        self.cal.setGridVisible(True)
        #self.cal.setMinimumDate(QDate.currentDate())		# not show previous days

        bottom_hbox.addWidget(self.cal, 1)
        bottom_hbox.addStretch(1)

        #hbox.addLayout(vbox, 10)

        text = QLabel("Park")
        text.setStyleSheet("font-family: Ubuntu; font-size: 30px;")
        vbox_main.addWidget(text, 2, Qt.AlignTop | Qt.AlignHCenter)

        vbox_main.addLayout(table_hbox, 10)
        vbox_main.addStretch(2)
        vbox_main.addLayout(bottom_hbox, 6)
        vbox_main.addStretch(2)
        self.setLayout(vbox_main)

        self.setWindowTitle("Park")
        #self.full_screen()
        self.show()


    def full_screen(self):
        desktop_resolution = QDesktopWidget().availableGeometry()
        self.setGeometry(desktop_resolution)


    # get info on parking lots
    def get_parking_lots(self):
        self.parking_lots_list.clear()
        response = make_http_request(self.https_session, "get", "parking_lots")
        if response.json():
            self.parking_lots = [ParkingLot(**parking_lots) for parking_lots in response.json()]
        else:
            self.parking_lots = []
            return

        if self.parking_lots:                       # if list is empty -> false
            #print(self.parking_lots)
            for parking_lot in self.parking_lots:
                self.parking_lots_list.addItem(parking_lot.get_name().capitalize())
                # get info on parking slots of parking lot
                response = make_http_request(self.https_session, "get", "parking_lots/" + str(parking_lot.get_id()) + "/parking_slots")
                if response.json():
                    parking_lot.set_parking_slots([ParkingSlot(**parking_slot) for parking_slot in response.json()])
            #print(self.parking_lots)

            # get info on vehicle type of parking_slots
            response = make_http_request(self.https_session, "get", "vehicle_types")
            if response.json():
                self.vehicle_types = [VehicleType(**vehicle_type) for vehicle_type in response.json()]
            #print(self.vehicle_types)

            self.parking_lots_list.setCurrentRow(0)             # first item selected (default)
            self.parking_lot = self.parking_lots[0]             # self.parking_lot = actually selected parking lot (default)


    def disability_check(self):
        disabled_user = self.user.get_disability()
        slots_row_indexes_disability = [slot.get_number() - 1 for slot in self.parking_lot.get_parking_slots() if slot.get_reserved_disability() == True]
        for row_index in slots_row_indexes_disability:
            for col_index in range(96):
                self.tableWidget.item_paint(row_index, col_index, self.tableWidget.cells_colors["disability"], not disabled_user)    # only disabled user can make bookings on reserved slots


    def get_bookings(self, parking_lot, selected_park_changed):
        #GET bookings?since=xxxx-xx-xx&until=xxxx-xx-xx&id_user=x&id_parking_lot=x
        self.parking_lot = parking_lot
        # clear previously selected info (also items background color). Removes all items not in the headers.
        self.tableWidget.clearContents()
        # check on slots reserved for disability and set color
        self.disability_check()

        # selected_park_changed=True when there is a change in selected parking lot. Info on parking slots needs to be updated only in this case. Otherwise in case of date change is equal to False.
        if selected_park_changed:
            self.tableWidget.setRowCount(self.parking_lot.get_num_parking_slots())

            # shows the associated vehicle type for each parking slot
            self.vehicle_types_table.clearContents()
            self.vehicle_types_table.setRowCount(self.tableWidget.rowCount())           # each parking slot has associated exactly one vehicle type
            for i in range(self.vehicle_types_table.rowCount()):

                try:
                    vehicle_type_index = self.vehicle_types.index(VehicleType(id=self.parking_lot.get_parking_slots()[i].get_id_vehicle_type()))
                except ValueError:
                    return
                else:
                    vehicle_type_name = self.vehicle_types[vehicle_type_index].get_name()
                    #print(vehicle_type_name)
                    self.vehicle_types_table.setItem(i, 0, QTableWidgetItem(vehicle_type_name.capitalize()))


        #day = str(self.cal.selectedDate().toString(Qt.ISODate))
        day = self.cal.selectedDate().toString(Qt.ISODate)
        #print(day)

        # start: from local 00:00:00 to utc (e.g. local 2021-04-01 00:00:00 = UTC 2021-03-31 22:00)
        # end: from local 23:59:00 to utc
        bookings_start_datetime = datetime_to_UTC(day, "00:00", True)     #is_query = True
        bookings_end_datetime = datetime_to_UTC(day, "23:59", True)   # 23:59 for include bookings of time xx:45, 23:45 is enough
        # bookings_start_datetime could be different from bookings_end_datetime (max 1 day)
        #print("UTC - Start validity: ", bookings_start_datetime, " ; End validity: ", bookings_end_datetime, " ; Id parking lot: ", self.parking_lot.get_id())

        response = make_http_request(self.https_session, "get", "bookings", params = {"since":bookings_start_datetime, "until":bookings_end_datetime, "id_parking_lot":self.parking_lot.get_id()})
        if response.json():
            bookings = [Booking(**booking) for booking in response.json()]

        # bookings are in UTC
        #if bookings:
            #print([f"{booking.get_datetime_start()} - {booking.get_datetime_end()}" for booking in bookings])

            for booking in bookings:
                print("-----------------\n", booking)

                booking_start = datetime_UTC_to_local(booking.get_datetime_start())
                print("Local booking start: ", booking_start)

                booking_end = datetime_UTC_to_local(booking.get_datetime_end())
                print("Local booking end: ", booking_end)

                booking_start = booking_start if booking_start.split()[0] == day else day + " 00:00:00"      # if booking start day in localtime is different from booking end day in localtime (booking on multiple days) select all table cells.
                print("New start: ", booking_start)

                # Set end time to 24:00:00 for bookings which end day is temporally subsequent to the selected day.
                booking_end = booking_end if booking_end.split()[0] == day else day + " 24:00:00"
                print("New end: ", booking_end)

                # show booking in table
                booking_start_time_split = booking_start.split()[1].split(":")          # list with booking time info
                booking_end_time_split = booking_end.split()[1].split(":")

                # find table row index
                try:
                    slot_index = self.parking_lot.get_parking_slots().index(ParkingSlot(id = booking.get_id_parking_slot()))
                except ValueError:
                    return
                else:
                    table_row_index = self.parking_lot.get_parking_slots()[slot_index].get_number() - 1

                # find table column indexes
                start_table_col_index = int(booking_start_time_split[0])*4 + int(booking_start_time_split[1])//15      # table column index = hour*4 + minutes/15
                end_table_col_index = int(booking_end_time_split[0])*4 + int(booking_end_time_split[1])//15

                # show booking in table, different color for booking owner user
                color = self.tableWidget.cells_colors["user_bookings"] if booking.get_id_user() == self.user.get_id() else self.tableWidget.cells_colors["other_bookings"]
                for i in range(start_table_col_index, end_table_col_index):
                    self.tableWidget.item_paint(table_row_index, i, color)

                # deny bookings 15 minutes before and after existing bookings
                deactivate = not (booking.get_id_user() == self.user.get_id())
                if (start_table_col_index > 0):
                    if self.tableWidget.item(table_row_index, start_table_col_index-1) == None:
                        self.tableWidget.item_paint(table_row_index, start_table_col_index-1, self.tableWidget.cells_colors["bookings_separation"], deactivate)
                if (end_table_col_index < 96):
                    if self.tableWidget.item(table_row_index, end_table_col_index) == None:
                        self.tableWidget.item_paint(table_row_index, end_table_col_index, self.tableWidget.cells_colors["bookings_separation"], deactivate)


    def get_booking_amount(self, utc_start_datetime_string, utc_end_datetime_string, slot_vehicle_type_id):
        # amount = hourly_rate * rate_percentage * time_interval

        # hourly rate
        user_category_id = self.user.get_id_user_category()
        # obtain user_categories
        response = make_http_request(self.https_session, "get", "user_categories")
        if response.json():
            user_categories = [UserCategory(**user_category) for user_category in response.json()]

        if user_categories:             # not empty
            try:
                category_index = user_categories.index(UserCategory(id = user_category_id))
            except ValueError:
                return
            else:
                hourly_rate_id = user_categories[category_index].get_id_hourly_rate()

        # obtain hourly rates and filter
        response = make_http_request(self.https_session, "get", "hourly_rates")
        if response.json():
            hourly_rates = [HourlyRate(**hourly_rate) for hourly_rate in response.json()]

        if hourly_rates:
            try:
                hourly_rate_index = hourly_rates.index(HourlyRate(id = hourly_rate_id))
            except ValueError:
                return
            else:
                hourly_rate_amount = float(hourly_rates[hourly_rate_index].get_amount())

        #print(hourly_rate_amount)

        # rate_percentage
        try:
            vehicle_type_index = self.vehicle_types.index(VehicleType(id = slot_vehicle_type_id))
        except ValueError:
            return
        else:
            rate_percentage = float(self.vehicle_types[vehicle_type_index].get_rate_percentage())

        #print(rate_percentage)

        # time_inverval = utc_end_datetime_string - utc_start_datetime_string in secondi o minuti
        time_interval_seconds = (datetime.strptime(f"{utc_end_datetime_string}", "%Y-%m-%d %H:%M:%S") - datetime.strptime(f"{utc_start_datetime_string}", "%Y-%m-%d %H:%M:%S")).total_seconds()
        #print(time_interval_seconds)

        booking_amount = "{:.2f}".format((hourly_rate_amount/3600) * rate_percentage * time_interval_seconds)
        return booking_amount


    def make_booking(self, parking_slot_number, start_time, end_time):
        if user_is_admin(self.user, self.https_session):
            QMessageBox.information(self, "uPark tip", "Admin can't make bookings!")
            return

        if not self.parking_lots:
            QMessageBox.information(self, "uPark tip", "There aren't parking lots for your category!")
            return

        print("********************")
        day = str(self.cal.selectedDate().toString(Qt.ISODate))

        # booking start and end in UTC
        utc_start_datetime_string = datetime_to_UTC(day, start_time)
        utc_end_datetime_string = datetime_to_UTC(day, end_time)

        #print(utc_start_datetime_string, utc_end_datetime_string)

        # booking duration, work with timezone and daylight saving time
        booking_duration = (datetime.strptime(f"{utc_end_datetime_string}", "%Y-%m-%d %H:%M:%S") - datetime.strptime(f"{utc_start_datetime_string}", "%Y-%m-%d %H:%M:%S"))

        response = make_http_request(self.https_session, "get", "users/" + str(self.user.get_id()) + "/vehicles")
        if response.json():
            user_vehicles = [Vehicle(**user_vehicle) for user_vehicle in response.json()]
        else:
            user_vehicles = []

        # obtain slot vehicle type name
        slot_vehicle_type = self.vehicle_types_table.item(self.tableWidget.currentRow(), 0).text()
        #print(slot_vehicle_type)

        try:
            vehicle_type_index = self.vehicle_types.index(VehicleType(name = slot_vehicle_type.lower()))
        except ValueError:
            return
        else:
            slot_vehicle_type_id = self.vehicle_types[vehicle_type_index].get_id()

        # it's necessary convert back from UTC to local time zone to handle daylight saving time
        booking_amount = self.get_booking_amount(utc_start_datetime_string, utc_end_datetime_string, slot_vehicle_type_id)
        print(booking_amount)

        datetime_start = datetime_UTC_to_local(utc_start_datetime_string)
        datetime_end = datetime_UTC_to_local(utc_end_datetime_string)
        booking_dialog = BookingDialog(datetime_start, datetime_end, booking_duration, parking_slot_number, user_vehicles, slot_vehicle_type_id, booking_amount, self)
        # if clicked OK button
        if booking_dialog.exec_():
            vehicle = booking_dialog.selected_vehicle()

            if not vehicle:
                return

            try:
                vehicle_index = user_vehicles.index(Vehicle(license_plate = vehicle.split()[0]))
                slot_index = self.parking_lot.get_parking_slots().index(ParkingSlot(number = parking_slot_number))
            except ValueError:
                return
            else:
                vehicle_id = user_vehicles[vehicle_index].get_id()
                parking_slot_id = int(self.parking_lot.get_parking_slots()[slot_index].get_id())
            #print(vehicle_id)
            #print(self.parking_lot)


            booking =   {
                        "datetime_start": utc_start_datetime_string,
                        "datetime_end": utc_end_datetime_string,
                        "id_vehicle": vehicle_id,
                        "id_parking_slot": parking_slot_id
                        }

            response = make_http_request(self.https_session, "post", "users/" + str(self.user.get_id()) + "/bookings", json = booking)
            if response:
                json_response = response.json()
                QMessageBox.information(self, "Booking response", json_response["message"] + "\nPrice: " + str(json_response["amount"]))
                self.get_bookings(self.parking_lot, False)

    def showEvent(self, event):
        self.get_parking_lots()
        if self.parking_lots_list:
            self.cal.selectionChanged.connect(lambda : self.get_bookings(self.parking_lots[self.parking_lots_list.currentRow()], False))
            self.get_bookings(self.parking_lots[self.parking_lots_list.currentRow()], False)
        else:
            try:
                self.cal.disconnect()
            except:
                pass
