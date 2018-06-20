-- phpMyAdmin SQL Dump
-- version 2.11.9.5
-- http://www.phpmyadmin.net
--
-- Host: 172.20.18.82
-- Generation Time: Jan 05, 2012 at 05:26 PM
-- Server version: 4.1.22
-- PHP Version: 4.4.9

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

--
-- Database: `db1056070_my`
--

-- --------------------------------------------------------

--
-- Table structure for table `test_leaderboards`
--

CREATE TABLE IF NOT EXISTS `test_leaderboards` (
  `bestTime` int(11) NOT NULL default '0',
  `userName` varchar(32) NOT NULL default 'unknown',
  `dateSubmitted` datetime NOT NULL default '0000-00-00 00:00:00',
  KEY `bestTime` (`bestTime`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `test_leaderboards`
--

INSERT INTO `test_leaderboards` (`bestTime`, `userName`, `dateSubmitted`) VALUES
(13, 'Jasio', '0000-00-00 00:00:00'),
(17, 'Pysio', '0000-00-00 00:00:00'),
(21, 'MasterOfMind', '0000-00-00 00:00:00'),
(35, 'BadAss', '0000-00-00 00:00:00'),
(43, 'Koczkodan', '0000-00-00 00:00:00'),
(16, 'Patriota', '0000-00-00 00:00:00'),
(16, 'Szemrota', '2010-08-19 23:17:05');
