<?php
/**
 * Part of php-sqllint
 *
 * PHP version 5
 *
 * @category  Tools
 * @package   PHP-SQLlint
 * @author    Christian Weiske <cweiske@cweiske.de>
 * @copyright 2014 Christian Weiske
 * @license   http://www.gnu.org/licenses/agpl.html GNU AGPL v3
 * @link      http://cweiske.de/php-sqllint.htm
 */
namespace phpsqllint;

/**
 * Class autoloader, PSR-0 compliant.
 *
 * @category  Tools
 * @package   PHP-SQLlint
 * @author    Christian Weiske <cweiske@cweiske.de>
 * @copyright 2014 Christian Weiske
 * @license   http://www.gnu.org/licenses/agpl.html GNU AGPL v3
 * @version   Release: @package_version@
 * @link      http://cweiske.de/php-sqllint.htm
 */
class Autoloader
{
    /**
     * Load the given class
     *
     * @param string $class Class name
     *
     * @return void
     */
    public function load($class)
    {
        $file = strtr($class, '_\\', '//') . '.php';
        if (stream_resolve_include_path($file)) {
            include $file;
        }
    }

    /**
     * Register this autoloader
     *
     * @return void
     */
    public static function register()
    {
        set_include_path(
            get_include_path() . PATH_SEPARATOR . __DIR__ . '/../'
        );
        spl_autoload_register(array(new self(), 'load'));
    }
}
?>