The scripts in this folder are built to simplify the process of working on curriculum.

TestAllProjects.ps1
	This script tests all of the assignment templates and debugging demos and logs the results.
	It will keep track of any that fail to be made or built and report them at the end of its output.
	It needs no arguments.

TestAllProjectsAndLogOutput.cmd
	This script calls TestAllProjects.ps1 and routes its output to a log file.
	It can be run directly via windows explorer instead of needing to run it via command line.