# lslargest

**This project has been superseded and is now part of the [fileutil](https://github.com/nluka/fileutil) project as the `sizerank` function.**

lslargest is a command-line utility that displays a ranked list of the largest files in a directory. It was written as a utility for the development of [Ctructure](https://github.com/nluka/Ctructure), but may be useful otherwise.

## Usage

Command-line usage: `<root> [<option> [<value>]]...`

| Option         | Shortcut | Value                   | Default | Description |
| ---------------| -------- | ----------------------- | ------- | ----------- |
| `--version`    | `-v`     | N/A                     | N/A     | prints program version |
| `--rank-limit` | `-r`     | [1, 1,000,000)          | 10      | how many files to list |
| `--max-size`   | `-m`     | [1, UINTMAX_MAX]        | any     | max file size (in bytes) to consider |
| `--extensions` | `-e`     | CSV                     | all     | which file extensions to consider |
| `--save`       | `-s`     | pathname                | nil     | pathname of file to write output into |
| `--console`    | `-c`     | 0 \| 1                  | N/A     | whether file rankings are printed to console |

## Code Style

This project follows my [cpp-style-guide](https://github.com/nluka/cpp-style-guide).
