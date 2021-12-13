[![CMake](https://github.com/JAJames/Jupiter-Bot/actions/workflows/cmake.yml/badge.svg)](https://github.com/JAJames/Jupiter-Bot/actions/workflows/cmake.yml)

## Configuration
There are also many configuration options in the Config  files. Please read through these to get an idea of what
exactly you can actually configure the bot to do. Typically only one instance of the bot is needed, regardless of
the number of game servers, as long as they will be configured similarly.

Some INI files, particularly RenX.Medals.ini, RenX.ModSystem.ini, SetJoin.ini, and RenX.SetJoin.ini, may be overwritten
at runtime. All comments in the file will be lost when this occurs. These will be changed to not be overwritten in the
future.

## Command Line
All examples prefix arguments with `--`, however the bot also accepts arguments prefixed with `-`, and will accept
either `=` or a space (` `) for argument values. All parameters are optional, and all switches are unset by default.
### Syntax
`Bot.exe [--config=Config.ini] [--pluginsdir=Plugins\] [--configsdir=Configs\] [--echo-parameters] [--exit]`
### Options
* `--config`: Specifies the path to the application config file.
* `--pluginsdir`: Specifies the path to the Plugins directory (where the .so or .dll files are).
* `--configsdir`: Specifies the path to the Configs directory (where the .ini files, other than main config, are)
* `--echo-parameters`: Echos the parameters on the command-line back to the user (useful for script debugging)
* `--exit`: Immediately exits the application post-initialization (useful for startup crash testing).

## Commands
The commands available depend on where commands are being invoked, the invoking user's access level, and the plugins
loaded. To get a full list of available commands, setup the bot and execute the help command (`!help`). The help command
also provides command-specific help messages explaining what each command does and its syntax.
