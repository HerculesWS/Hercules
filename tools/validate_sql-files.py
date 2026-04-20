import os
import subprocess
from pathlib import Path

def lint_sql_files_with_sqlfluff(sql_directory_path):
    """
    Lint all .sql files in the given directory and its subdirectories using SQLFluff.

    Args:
        sql_directory_path (str): Path to the directory containing SQL files.
    """
    sql_directory = Path(sql_directory_path)
    if not sql_directory.exists() or not sql_directory.is_dir():
        print(f"Error: {sql_directory_path} is not a valid directory.")
        return

    sql_file_list = list(sql_directory.rglob('*.sql'))
    if not sql_file_list:
        print(f"No .sql files found in {sql_directory_path}.")
        return

    for sql_file_path in sql_file_list:
        print(f"\nLinting {sql_file_path} with SQLFluff...\n")
        try:
            lint_process = subprocess.run(['sqlfluff', 'lint', '--dialect', 'mysql', '--verbose', str(sql_file_path)], capture_output=True, text=True, timeout=120)
            if lint_process.returncode != 0:
                print(f"Errors in {sql_file_path}:")
                if lint_process.stdout:
                    print(f"STDOUT:\n{lint_process.stdout}")
                if lint_process.stderr:
                    print(f"STDERR:\n{lint_process.stderr}")
                print()
            else:
                print(f"{sql_file_path} is valid!\n")
        except subprocess.TimeoutExpired:
            print(f"Linting timed out for {sql_file_path}.")
        except FileNotFoundError:
            print("Error: SQLFluff is not installed or not in PATH.")
            break
        except Exception as e:
            print(f"Unexpected error linting {sql_file_path}: {e}")

if __name__ == "__main__":
    # Specify the directory containing your .sql files
    sql_directory_path = './sql-files'
    lint_sql_files_with_sqlfluff(sql_directory_path)
