# plutracer
Pathtracer written by friends, for the lulz
No telling quite how this will turn out

# building

To build you will need a C++ development environemnt (Visual Studio, XCode, just regular unix cmdline tooling) and CMake

1. Obtain Source Code 

	If your are looking to contribute, follow the instructions in the contributing section, otherwise just clone the git repo, but make sure to clone recursively to get all the dependencies
	````
		git clone --recursive https://github.com/andrew-pa/plutracer
	````
	there I even typed it for you

2. Run CMake

	+ Open a shell (Bash or Powershell) in the root directory of the project
	+ Make a new directory named build (`mkdir build`) and change into it (`cd build`)
	+ Run CMake on the root directory of the project (`cmake ..`)


3. Build with platform build tools
	
	This part is somewhat platform dependent, unlike the rest

	+ On Windows with Visual Studio
		
		Open the .sln file in the build directory

	+ Other platforms

		Something should be in the build directory that looks useful (maybe XCode project files or something)

	+ Platforms with Makefiles
		
		run `make` in the build directory
	
# contributing

If you want to contribute:

1. Fork the project on github
2. Make changes on your fork
	
	When you pull down the source code, clone your fork instead, so if your username was 'foo':
	````
		git clone --recursive https://github.com/foo/plutracer
	````
	use typical git commit/push/pull on your fork, if you don't know go find a git tutorial there are many, it's not hard
	also see STYLE.md

3. When you are done making changes to fix a bug/add a feature, make a pull request against andrew-pa's repository
	
	In the message/description that goes with it, try to detail what you did, include issue numbers (prefixed with # so github does all the fancy linking)

4. Eventually the PR will get merged into master, at which point you will want to update your fork to pull in any changes made by others
5. Once you have the source code, basically repeat from the end of step 2, no more need for cloning although `git pull` might be necessary
	
Maybe one day people will actually get repo read/write access but for now this way I get to look over all the PRs before people commit garbage
