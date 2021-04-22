-- phpMyAdmin SQL Dump
-- version 5.0.4
-- https://www.phpmyadmin.net/
--
-- Host: upark-server-db.mysql.database.azure.com:3306
-- Generation Time: Apr 22, 2021 at 07:35 AM
-- Server version: 8.0.15
-- PHP Version: 8.0.2

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `db_upark`
--
CREATE DATABASE IF NOT EXISTS `db_upark` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE `db_upark`;

-- --------------------------------------------------------

--
-- Table structure for table `bookings`
--

CREATE TABLE `bookings` (
  `id` int(11) NOT NULL,
  `datetime_start` timestamp NOT NULL,
  `datetime_end` timestamp NOT NULL,
  `entry_time` timestamp NULL DEFAULT NULL,
  `exit_time` timestamp NULL DEFAULT NULL,
  `amount` decimal(6,2) NOT NULL,
  `id_user` int(11) DEFAULT NULL,
  `id_vehicle` int(11) DEFAULT NULL,
  `id_parking_slot` int(11) NOT NULL,
  `note` varchar(500) CHARACTER SET latin1 COLLATE latin1_general_ci DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 TABLESPACE `innodb_system`;

-- --------------------------------------------------------

--
-- Table structure for table `hourly_rates`
--

CREATE TABLE `hourly_rates` (
  `id` int(11) NOT NULL,
  `amount` decimal(3,2) NOT NULL DEFAULT '1.00'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT TABLESPACE `innodb_system`;

-- --------------------------------------------------------

--
-- Table structure for table `parking_lots`
--

CREATE TABLE `parking_lots` (
  `id` int(11) NOT NULL,
  `name` varchar(20) NOT NULL,
  `street` varchar(50) NOT NULL,
  `num_parking_slots` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 TABLESPACE `innodb_system`;

-- --------------------------------------------------------

--
-- Table structure for table `parking_lots_user_categories_allowed`
--

CREATE TABLE `parking_lots_user_categories_allowed` (
  `id` int(11) NOT NULL,
  `id_parking_lot` int(11) NOT NULL,
  `id_user_category` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 TABLESPACE `innodb_system`;

-- --------------------------------------------------------

--
-- Table structure for table `parking_slots`
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
-- Table structure for table `users`
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
-- Table structure for table `user_categories`
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
-- Table structure for table `vehicles`
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
-- Table structure for table `vehicle_types`
--

CREATE TABLE `vehicle_types` (
  `id` int(11) NOT NULL,
  `name` varchar(20) NOT NULL,
  `rate_percentage` decimal(3,2) NOT NULL DEFAULT '1.00'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 TABLESPACE `innodb_system`;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `bookings`
--
ALTER TABLE `bookings`
  ADD PRIMARY KEY (`id`),
  ADD KEY `id_user` (`id_user`),
  ADD KEY `id_vehicle` (`id_vehicle`),
  ADD KEY `bookings_ibfk_3` (`id_parking_slot`);

--
-- Indexes for table `hourly_rates`
--
ALTER TABLE `hourly_rates`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `parking_lots`
--
ALTER TABLE `parking_lots`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `name` (`name`);

--
-- Indexes for table `parking_lots_user_categories_allowed`
--
ALTER TABLE `parking_lots_user_categories_allowed`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `parking_lot_user_category` (`id_parking_lot`,`id_user_category`) USING BTREE,
  ADD KEY `id_user_category` (`id_user_category`);

--
-- Indexes for table `parking_slots`
--
ALTER TABLE `parking_slots`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `slot_identifier` (`number`,`id_parking_lot`) USING BTREE,
  ADD KEY `id_vehicle_type` (`id_vehicle_type`),
  ADD KEY `parking_slots_ibfk_3` (`id_parking_lot`);

--
-- Indexes for table `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `email` (`email`),
  ADD KEY `id_user_category` (`id_user_category`);

--
-- Indexes for table `user_categories`
--
ALTER TABLE `user_categories`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `name` (`name`),
  ADD KEY `id_hourly_rate` (`id_hourly_rate`);

--
-- Indexes for table `vehicles`
--
ALTER TABLE `vehicles`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `license_plate` (`license_plate`),
  ADD KEY `id_user` (`id_user`),
  ADD KEY `id_vehicle_type` (`id_vehicle_type`);

--
-- Indexes for table `vehicle_types`
--
ALTER TABLE `vehicle_types`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `name` (`name`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `bookings`
--
ALTER TABLE `bookings`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `hourly_rates`
--
ALTER TABLE `hourly_rates`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `parking_lots`
--
ALTER TABLE `parking_lots`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `parking_lots_user_categories_allowed`
--
ALTER TABLE `parking_lots_user_categories_allowed`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `parking_slots`
--
ALTER TABLE `parking_slots`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `users`
--
ALTER TABLE `users`
  MODIFY `id` int(10) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `user_categories`
--
ALTER TABLE `user_categories`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `vehicles`
--
ALTER TABLE `vehicles`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `vehicle_types`
--
ALTER TABLE `vehicle_types`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- Constraints for dumped tables
--

--
-- Constraints for table `bookings`
--
ALTER TABLE `bookings`
  ADD CONSTRAINT `bookings_ibfk_1` FOREIGN KEY (`id_user`) REFERENCES `users` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `bookings_ibfk_2` FOREIGN KEY (`id_vehicle`) REFERENCES `vehicles` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  ADD CONSTRAINT `bookings_ibfk_3` FOREIGN KEY (`id_parking_slot`) REFERENCES `parking_slots` (`id`) ON DELETE RESTRICT ON UPDATE CASCADE;

--
-- Constraints for table `parking_lots_user_categories_allowed`
--
ALTER TABLE `parking_lots_user_categories_allowed`
  ADD CONSTRAINT `parking_lots_user_categories_allowed_ibfk_2` FOREIGN KEY (`id_user_category`) REFERENCES `user_categories` (`id`) ON DELETE RESTRICT ON UPDATE CASCADE,
  ADD CONSTRAINT `parking_lots_user_categories_allowed_ibfk_3` FOREIGN KEY (`id_parking_lot`) REFERENCES `parking_lots` (`id`) ON DELETE RESTRICT ON UPDATE CASCADE;

--
-- Constraints for table `parking_slots`
--
ALTER TABLE `parking_slots`
  ADD CONSTRAINT `parking_slots_ibfk_2` FOREIGN KEY (`id_vehicle_type`) REFERENCES `vehicle_types` (`id`) ON UPDATE CASCADE,
  ADD CONSTRAINT `parking_slots_ibfk_3` FOREIGN KEY (`id_parking_lot`) REFERENCES `parking_lots` (`id`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `users`
--
ALTER TABLE `users`
  ADD CONSTRAINT `users_ibfk_1` FOREIGN KEY (`id_user_category`) REFERENCES `user_categories` (`id`) ON UPDATE CASCADE;

--
-- Constraints for table `user_categories`
--
ALTER TABLE `user_categories`
  ADD CONSTRAINT `user_categories_ibfk_1` FOREIGN KEY (`id_hourly_rate`) REFERENCES `hourly_rates` (`id`) ON UPDATE CASCADE;

--
-- Constraints for table `vehicles`
--
ALTER TABLE `vehicles`
  ADD CONSTRAINT `vehicles_ibfk_1` FOREIGN KEY (`id_vehicle_type`) REFERENCES `vehicle_types` (`id`) ON UPDATE CASCADE,
  ADD CONSTRAINT `vehicles_ibfk_2` FOREIGN KEY (`id_user`) REFERENCES `users` (`id`) ON DELETE SET NULL ON UPDATE CASCADE;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
