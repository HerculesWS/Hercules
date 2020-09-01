<?php
/**
 * Part of php-sqllint
 *
 * PHP version 5
 *
 * @category Tools
 * @package  PHP-SQLlint
 * @author   Christian Weiske <cweiske@cweiske.de>
 * @license  https://www.gnu.org/licenses/agpl.html GNU AGPL v3
 * @link     http://cweiske.de/php-sqllint.htm
 */
namespace phpsqllint;

/**
 * Output for emacs' compilation mode
 *
 * @category Tools
 * @package  PHP-SQLlint
 * @author   Christian Weiske <cweiske@cweiske.de>
 * @license  https://www.gnu.org/licenses/agpl.html GNU AGPL v3
 * @link     http://www.emacswiki.org/emacs/CreatingYourOwnCompileErrorRegexp
 */
class Renderer_Emacs implements Renderer
{
    protected $filename;

    /**
     * Begin syntax check output rendering
     *
     * @param string $filename Path to the SQL file
     *
     * @return void
     */
    public function startRendering($filename)
    {
        $this->filename = $filename;
    }

    /**
     * Output errors in GNU style; see emacs compilation.txt
     *
     * @param string  $msg   Error message
     * @param string  $token Character which caused the error
     * @param integer $line  Line at which the error occured
     * @param integer $col   Column at which the error occured
     *
     * @return void
     */
    public function displayError($msg, $token, $line, $col)
    {
        echo $this->filename
            . ':' . $line
            . '.' . $col
            . ':Error:'
            . ' '. $msg
            . "\n";
    }

    /**
     * Finish syntax check output rendering; no syntax errors found
     *
     * @return void
     */
    public function finishOk()
    {
        //do nothing
    }
}
?>
