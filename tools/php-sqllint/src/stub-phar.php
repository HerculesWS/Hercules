#!/usr/bin/env php
<?php
/**
 * Phar stub file for php-sqllint. Handles startup of the .phar file.
 *
 * PHP version 5
 *
 * @category  Tools
 * @package   PHP-SQLlint
 * @author    Christian Weiske <cweiske@cweiske.de>
 * @copyright 2015 Christian Weiske
 * @license   http://www.gnu.org/licenses/agpl.html GNU AGPL v3
 * @link      http://cweiske.de/php-sqllint.htm
 */
Phar::mapPhar('php-sqllint.phar');
require 'phar://php-sqllint.phar/bin/phar-php-sqllint.php';
__HALT_COMPILER();
?>
