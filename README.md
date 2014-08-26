#Jumper

Jumper is a proof of concept application which shows how it is possible to jump from process to process without interrupting program flow.

This is accomplished by forking every second and exiting the parent process leaving an orphaned child.

:warning:  ***Terminating instances of this program is not a trivial task***

##Compilation

Just execute `make`

Three builds are created:
 * `djumper` - Debug build - includes gdb debug symbols and `-fstack-protector-all`
 * `jumper`  - Normal build
 * `ojumper` - O3 build - enables `-O3`, frame pointers and silences extraneous output

##Usage

Simply run one of the generated builds. Program output is saved to stdout and `log.txt`

##Future work
- [x] ~~Constantly delete and rename the build to make killing by name **fun**~~ Completed using advanced deception
- [ ] Turn into a library so turning other applications into rapidly forking hellspawn is easy
- [ ] Replace `signal` and `SIGALRM` with timers for even more frequent forking
