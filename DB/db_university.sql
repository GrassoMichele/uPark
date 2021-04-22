-- phpMyAdmin SQL Dump
-- version 5.0.4
-- https://www.phpmyadmin.net/
--
-- Host: upark-server-db.mysql.database.azure.com:3306
-- Generation Time: Apr 22, 2021 at 07:37 AM
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
-- Database: `db_university`
--
CREATE DATABASE IF NOT EXISTS `db_university` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE `db_university`;

-- --------------------------------------------------------

--
-- Table structure for table `upark_users`
--

CREATE TABLE `upark_users` (
  `id` int(10) UNSIGNED NOT NULL,
  `email` varchar(320) CHARACTER SET latin1 COLLATE latin1_general_ci NOT NULL,
  `upark_code` varchar(10) NOT NULL,
  `id_user_category` int(10) UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT TABLESPACE `innodb_system`;

--
-- Dumping data for table `upark_users`
--

INSERT INTO `upark_users` (`id`, `email`, `upark_code`, `id_user_category`) VALUES
(1, 'antonio@gmail.com', 'RS2213RT', 1),
(2, 'giulia@live.it', 'FGH766TI', 2),
(7, 'tecnico@tecnico.it', 'UPARK9', 6);

-- --------------------------------------------------------

--
-- Table structure for table `user_categories`
--

CREATE TABLE `user_categories` (
  `id` int(10) UNSIGNED NOT NULL,
  `name` varchar(50) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT TABLESPACE `innodb_system`;

--
-- Dumping data for table `user_categories`
--

INSERT INTO `user_categories` (`id`, `name`) VALUES
(4, 'Giardiniere'),
(8, 'Guardiola'),
(5, 'Operatore ecologico'),
(2, 'Professore'),
(7, 'Segreteria'),
(3, 'Stagista'),
(1, 'Studente'),
(6, 'Tecnico');

--
-- Indexes for dumped tables
--

--
-- Indexes for table `upark_users`
--
ALTER TABLE `upark_users`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `email` (`email`),
  ADD UNIQUE KEY `uparkCode` (`upark_code`),
  ADD KEY `id_categoria_utente` (`id_user_category`);

--
-- Indexes for table `user_categories`
--
ALTER TABLE `user_categories`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `nome` (`name`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `upark_users`
--
ALTER TABLE `upark_users`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=8;

--
-- AUTO_INCREMENT for table `user_categories`
--
ALTER TABLE `user_categories`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=11;

--
-- Constraints for dumped tables
--

--
-- Constraints for table `upark_users`
--
ALTER TABLE `upark_users`
  ADD CONSTRAINT `upark_users_ibfk_1` FOREIGN KEY (`id_user_category`) REFERENCES `user_categories` (`id`) ON UPDATE CASCADE;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
