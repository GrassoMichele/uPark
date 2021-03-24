-- phpMyAdmin SQL Dump
-- version 4.9.5deb2
-- https://www.phpmyadmin.net/
--
-- Host: upark-server-db.mysql.database.azure.com:3306
-- Creato il: Mar 24, 2021 alle 10:53
-- Versione del server: 8.0.15
-- Versione PHP: 7.4.3

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `db_upark`
--

-- --------------------------------------------------------

--
-- Struttura della tabella `bookings`
--

CREATE TABLE `bookings` (
  `id` int(11) NOT NULL,
  `datetime_start` timestamp NOT NULL,
  `datetime_end` timestamp NOT NULL,
  `entry_time` time DEFAULT NULL,
  `exit_time` time DEFAULT NULL,
  `amount` decimal(6,2) NOT NULL,
  `id_user` int(11) DEFAULT NULL,
  `id_vehicle` int(11) DEFAULT NULL,
  `id_parking_slot` int(11) DEFAULT NULL,
  `note` varchar(500) CHARACTER SET latin1 COLLATE latin1_general_ci DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 TABLESPACE `innodb_system`;

-- --------------------------------------------------------

--
-- Struttura della tabella `events`
--

CREATE TABLE `events` (
  `id` int(11) NOT NULL,
  `name` varchar(20) NOT NULL,
  `upark_code` varchar(10) NOT NULL,
  `validity_start` timestamp NOT NULL,
  `validity_end` timestamp NOT NULL,
  `id_event_type` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 TABLESPACE `innodb_system`;

-- --------------------------------------------------------

--
-- Struttura della tabella `event_participations`
--

CREATE TABLE `event_participations` (
  `id_guest_user` int(11) NOT NULL,
  `id_event` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 TABLESPACE `innodb_system`;

-- --------------------------------------------------------

--
-- Struttura della tabella `event_types`
--

CREATE TABLE `event_types` (
  `id` int(11) NOT NULL,
  `name` varchar(20) NOT NULL,
  `id_hourly_rate` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 TABLESPACE `innodb_system`;

-- --------------------------------------------------------

--
-- Struttura della tabella `hourly_rates`
--

CREATE TABLE `hourly_rates` (
  `id` int(11) NOT NULL,
  `amount` decimal(3,2) NOT NULL DEFAULT '1.00'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT TABLESPACE `innodb_system`;

-- --------------------------------------------------------

--
-- Struttura della tabella `parking_lots`
--

CREATE TABLE `parking_lots` (
  `id` int(11) NOT NULL,
  `name` varchar(20) NOT NULL,
  `street` varchar(50) NOT NULL,
  `num_parking_slots` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 TABLESPACE `innodb_system`;

-- --------------------------------------------------------

--
-- Struttura della tabella `parking_lots_user_categories_allowed`
--

CREATE TABLE `parking_lots_user_categories_allowed` (
  `id` int(11) NOT NULL,
  `id_parking_lot` int(11) NOT NULL,
  `id_user_category` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 TABLESPACE `innodb_system`;

-- --------------------------------------------------------

--
-- Struttura della tabella `parking_slots`
--

CREATE TABLE `parking_slots` (
  `id` int(11) NOT NULL,
  `number` int(11) NOT NULL,
  `id_parking_lot` int(11) NOT NULL,
  `id_vehicle_type` int(11) NOT NULL,
  `reserved_disability` tinyint(1) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 TABLESPACE `innodb_system`;

-- --------------------------------------------------------

--
-- Struttura della tabella `users`
--

CREATE TABLE `users` (
  `id` int(10) NOT NULL,
  `email` varchar(320) CHARACTER SET latin1 COLLATE latin1_general_ci NOT NULL,
  `name` varchar(20) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `surname` varchar(20) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `password` varchar(20) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `wallet` decimal(6,2) NOT NULL DEFAULT '0.00',
  `disability` tinyint(1) NOT NULL DEFAULT '0',
  `active_account` tinyint(1) DEFAULT '1',
  `id_user_category` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 TABLESPACE `innodb_system`;

-- --------------------------------------------------------

--
-- Struttura della tabella `user_categories`
--

CREATE TABLE `user_categories` (
  `id` int(11) NOT NULL,
  `name` varchar(50) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,
  `id_hourly_rate` int(11) DEFAULT NULL,
  `service_validity_start` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `service_validity_end` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 TABLESPACE `innodb_system`;

-- --------------------------------------------------------

--
-- Struttura della tabella `vehicles`
--

CREATE TABLE `vehicles` (
  `id` int(11) NOT NULL,
  `license_plate` varchar(10) NOT NULL,
  `brand` varchar(20) NOT NULL,
  `model` varchar(20) NOT NULL,
  `id_user` int(11) DEFAULT NULL,
  `id_vehicle_type` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 TABLESPACE `innodb_system`;

-- --------------------------------------------------------

--
-- Struttura della tabella `vehicle_types`
--

CREATE TABLE `vehicle_types` (
  `id` int(11) NOT NULL,
  `name` varchar(20) NOT NULL,
  `rate_percentage` decimal(3,2) NOT NULL DEFAULT '1.00'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 TABLESPACE `innodb_system`;

--
-- Indici per le tabelle scaricate
--

--
-- Indici per le tabelle `bookings`
--
ALTER TABLE `bookings`
  ADD PRIMARY KEY (`id`),
  ADD KEY `id_user` (`id_user`),
  ADD KEY `id_vehicle` (`id_vehicle`),
  ADD KEY `id_parking_slot` (`id_parking_slot`);

--
-- Indici per le tabelle `events`
--
ALTER TABLE `events`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `upark_code` (`upark_code`),
  ADD KEY `id_event_type` (`id_event_type`);

--
-- Indici per le tabelle `event_participations`
--
ALTER TABLE `event_participations`
  ADD PRIMARY KEY (`id_guest_user`,`id_event`),
  ADD KEY `id_guest_user` (`id_guest_user`),
  ADD KEY `id_event` (`id_event`);

--
-- Indici per le tabelle `event_types`
--
ALTER TABLE `event_types`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `name` (`name`),
  ADD KEY `id_hourly_rate` (`id_hourly_rate`);

--
-- Indici per le tabelle `hourly_rates`
--
ALTER TABLE `hourly_rates`
  ADD PRIMARY KEY (`id`);

--
-- Indici per le tabelle `parking_lots`
--
ALTER TABLE `parking_lots`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `name` (`name`);

--
-- Indici per le tabelle `parking_lots_user_categories_allowed`
--
ALTER TABLE `parking_lots_user_categories_allowed`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `parking_lot_user_category` (`id_parking_lot`,`id_user_category`) USING BTREE,
  ADD KEY `id_user_category` (`id_user_category`);

--
-- Indici per le tabelle `parking_slots`
--
ALTER TABLE `parking_slots`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `slot_identifier` (`number`,`id_parking_lot`) USING BTREE,
  ADD KEY `id_vehicle_type` (`id_vehicle_type`),
  ADD KEY `parking_slots_ibfk_3` (`id_parking_lot`);

--
-- Indici per le tabelle `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `email` (`email`),
  ADD KEY `id_user_category` (`id_user_category`);

--
-- Indici per le tabelle `user_categories`
--
ALTER TABLE `user_categories`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `name` (`name`),
  ADD KEY `id_hourly_rate` (`id_hourly_rate`);

--
-- Indici per le tabelle `vehicles`
--
ALTER TABLE `vehicles`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `license_plate` (`license_plate`),
  ADD KEY `id_user` (`id_user`),
  ADD KEY `id_vehicle_type` (`id_vehicle_type`);

--
-- Indici per le tabelle `vehicle_types`
--
ALTER TABLE `vehicle_types`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `name` (`name`);

--
-- AUTO_INCREMENT per le tabelle scaricate
--

--
-- AUTO_INCREMENT per la tabella `bookings`
--
ALTER TABLE `bookings`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT per la tabella `events`
--
ALTER TABLE `events`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT per la tabella `event_types`
--
ALTER TABLE `event_types`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT per la tabella `hourly_rates`
--
ALTER TABLE `hourly_rates`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT per la tabella `parking_lots`
--
ALTER TABLE `parking_lots`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT per la tabella `parking_lots_user_categories_allowed`
--
ALTER TABLE `parking_lots_user_categories_allowed`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT per la tabella `parking_slots`
--
ALTER TABLE `parking_slots`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT per la tabella `users`
--
ALTER TABLE `users`
  MODIFY `id` int(10) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT per la tabella `user_categories`
--
ALTER TABLE `user_categories`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT per la tabella `vehicles`
--
ALTER TABLE `vehicles`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT per la tabella `vehicle_types`
--
ALTER TABLE `vehicle_types`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- Limiti per le tabelle scaricate
--

--
-- Limiti per la tabella `bookings`
--
ALTER TABLE `bookings`
  ADD CONSTRAINT `bookings_ibfk_1` FOREIGN KEY (`id_user`) REFERENCES `users` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `bookings_ibfk_2` FOREIGN KEY (`id_vehicle`) REFERENCES `vehicles` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `bookings_ibfk_3` FOREIGN KEY (`id_parking_slot`) REFERENCES `parking_slots` (`id`) ON DELETE RESTRICT ON UPDATE CASCADE;

--
-- Limiti per la tabella `events`
--
ALTER TABLE `events`
  ADD CONSTRAINT `events_ibfk_1` FOREIGN KEY (`id_event_type`) REFERENCES `event_types` (`id`) ON UPDATE CASCADE;

--
-- Limiti per la tabella `event_participations`
--
ALTER TABLE `event_participations`
  ADD CONSTRAINT `event_participations_ibfk_1` FOREIGN KEY (`id_event`) REFERENCES `events` (`id`) ON UPDATE CASCADE,
  ADD CONSTRAINT `event_participations_ibfk_2` FOREIGN KEY (`id_guest_user`) REFERENCES `users` (`id`) ON UPDATE CASCADE;

--
-- Limiti per la tabella `event_types`
--
ALTER TABLE `event_types`
  ADD CONSTRAINT `event_types_ibfk_1` FOREIGN KEY (`id_hourly_rate`) REFERENCES `hourly_rates` (`id`) ON UPDATE CASCADE;

--
-- Limiti per la tabella `parking_lots_user_categories_allowed`
--
ALTER TABLE `parking_lots_user_categories_allowed`
  ADD CONSTRAINT `parking_lots_user_categories_allowed_ibfk_2` FOREIGN KEY (`id_user_category`) REFERENCES `user_categories` (`id`) ON DELETE RESTRICT ON UPDATE CASCADE,
  ADD CONSTRAINT `parking_lots_user_categories_allowed_ibfk_3` FOREIGN KEY (`id_parking_lot`) REFERENCES `parking_lots` (`id`) ON DELETE RESTRICT ON UPDATE CASCADE;

--
-- Limiti per la tabella `parking_slots`
--
ALTER TABLE `parking_slots`
  ADD CONSTRAINT `parking_slots_ibfk_2` FOREIGN KEY (`id_vehicle_type`) REFERENCES `vehicle_types` (`id`) ON UPDATE CASCADE,
  ADD CONSTRAINT `parking_slots_ibfk_3` FOREIGN KEY (`id_parking_lot`) REFERENCES `parking_lots` (`id`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Limiti per la tabella `users`
--
ALTER TABLE `users`
  ADD CONSTRAINT `users_ibfk_1` FOREIGN KEY (`id_user_category`) REFERENCES `user_categories` (`id`) ON UPDATE CASCADE;

--
-- Limiti per la tabella `user_categories`
--
ALTER TABLE `user_categories`
  ADD CONSTRAINT `user_categories_ibfk_1` FOREIGN KEY (`id_hourly_rate`) REFERENCES `hourly_rates` (`id`) ON UPDATE CASCADE;

--
-- Limiti per la tabella `vehicles`
--
ALTER TABLE `vehicles`
  ADD CONSTRAINT `vehicles_ibfk_1` FOREIGN KEY (`id_vehicle_type`) REFERENCES `vehicle_types` (`id`) ON UPDATE CASCADE,
  ADD CONSTRAINT `vehicles_ibfk_2` FOREIGN KEY (`id_user`) REFERENCES `users` (`id`) ON DELETE SET NULL ON UPDATE CASCADE;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
