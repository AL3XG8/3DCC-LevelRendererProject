The scripts in this folder do smaller functions - they're designed to be called by other scripts rather than used directly.

TestProject.bat
	This script is built to test a gateware-dependent project. It will clone gateware, make the project, then build the project in debug and release mode. 
	After it finishes, it deletes the folders it creates
	Its arguments are:
		1: The relative path from the script to the folder containing the cmakelists.txt
		2: The relative path to the location where the project expects to find Gateware.h
		3: The name of the project in its cmakelists.txt

MakeProject.bat
	This script makes a cmake project. After it finishes, it deletes the build folder it creates.
	It does not attempt to build the projects, it just tries to make them. 
	It was built to test our debugging demos - they will not build successfully since they're intentionallly in a broken state, but cmake should still be able to generate their projects.
	Its arguments are:
		1: The relative path from the script to the folder containing the cmakelists.txt