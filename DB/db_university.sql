-- phpMyAdmin SQL Dump
-- version 4.9.5deb2
-- https://www.phpmyadmin.net/
--
-- Host: upark-server-db.mysql.database.azure.com:3306
-- Creato il: Mar 24, 2021 alle 10:51
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
-- Database: `db_university`
--

-- --------------------------------------------------------

--
-- Struttura della tabella `upark_users`
--

CREATE TABLE `upark_users` (
  `id` int(10) UNSIGNED NOT NULL,
  `email` varchar(320) CHARACTER SET latin1 COLLATE latin1_general_ci NOT NULL,
  `upark_code` varchar(10) NOT NULL,
  `id_user_category` int(10) UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT TABLESPACE `innodb_system`;

-- --------------------------------------------------------

--
-- Struttura della tabella `user_categories`
--

CREATE TABLE `user_categories` (
  `id` int(10) UNSIGNED NOT NULL,
  `name` varchar(50) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=COMPACT TABLESPACE `innodb_system`;

--
-- Indici per le tabelle scaricate
--

--
-- Indici per le tabelle `upark_users`
--
ALTER TABLE `upark_users`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `email` (`email`),
  ADD UNIQUE KEY `uparkCode` (`upark_code`),
  ADD KEY `id_categoria_utente` (`id_user_category`);

--
-- Indici per le tabelle `user_categories`
--
ALTER TABLE `user_categories`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `nome` (`name`);

--
-- AUTO_INCREMENT per le tabelle scaricate
--

--
-- AUTO_INCREMENT per la tabella `upark_users`
--
ALTER TABLE `upark_users`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT per la tabella `user_categories`
--
ALTER TABLE `user_categories`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- Limiti per le tabelle scaricate
--

--
-- Limiti per la tabella `upark_users`
--
ALTER TABLE `upark_users`
  ADD CONSTRAINT `upark_users_ibfk_1` FOREIGN KEY (`id_user_category`) REFERENCES `user_categories` (`id`) ON UPDATE CASCADE;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
