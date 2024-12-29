To build (in the parent folder of old-photo-parallel-A-code where the binnary shall be):
	make -C old-photo-parallel-A-code/

To clean (in the parent folder of old-photo-parallel-A-code: files *.o):
	make clean -C old-photo-parallel-A-code/
	
To clean (in the parent folder of old-photo-parallel-A-code: everything):
	make clean-all -C old-photo-parallel-A-code/

Run './run_all_B.sh' in old-photo-parallel-B to clean all and make
	
Code example:
	./old-photo-parallel-B ./Dataset-1/ 7 -size

Important Notes:
	folders, directories, relative and absolute pathnames must not have neither spaces nor tabs
	development was done In VS Code and building, cleaning and testing in a bash shell, side a side. See the code in an IDE, test the code in a shell.
	the Makefile is prepared to build in the parent folder of old-photo-parallel-B-code where the binnary is to be called.
	'paper-texture.png' is to be in old-photo-parallel-B-code.
	in old_photo_PAR_B folder there are intermediate folders with the name with the nest image processing; sepia processing in done into old_photo_PAR_A.
	my system does not have CLOCK_MONOTONIC, used CLOCK_REALTIME instead.
	prepared for not having an image-list.txt.
	all requested features are implemented.

Other Notes:
	Code refactoring and adaptation of old-photo-pipeline with requested new features.
	GitHub repository: https://github.com/barbacal/PConc-Project-B

