# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 4.0

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/pramos/Documents/Univ/SETR_Proj3/tests

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pramos/Documents/Univ/SETR_Proj3/tests/build

# Include any dependencies generated for this target.
include Unity/CMakeFiles/unity.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include Unity/CMakeFiles/unity.dir/compiler_depend.make

# Include the progress variables for this target.
include Unity/CMakeFiles/unity.dir/progress.make

# Include the compile flags for this target's objects.
include Unity/CMakeFiles/unity.dir/flags.make

Unity/CMakeFiles/unity.dir/codegen:
.PHONY : Unity/CMakeFiles/unity.dir/codegen

Unity/CMakeFiles/unity.dir/src/unity.c.o: Unity/CMakeFiles/unity.dir/flags.make
Unity/CMakeFiles/unity.dir/src/unity.c.o: /home/pramos/Documents/Univ/SETR_Proj3/tests/Unity/src/unity.c
Unity/CMakeFiles/unity.dir/src/unity.c.o: Unity/CMakeFiles/unity.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/pramos/Documents/Univ/SETR_Proj3/tests/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object Unity/CMakeFiles/unity.dir/src/unity.c.o"
	cd /home/pramos/Documents/Univ/SETR_Proj3/tests/build/Unity && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT Unity/CMakeFiles/unity.dir/src/unity.c.o -MF CMakeFiles/unity.dir/src/unity.c.o.d -o CMakeFiles/unity.dir/src/unity.c.o -c /home/pramos/Documents/Univ/SETR_Proj3/tests/Unity/src/unity.c

Unity/CMakeFiles/unity.dir/src/unity.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/unity.dir/src/unity.c.i"
	cd /home/pramos/Documents/Univ/SETR_Proj3/tests/build/Unity && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/pramos/Documents/Univ/SETR_Proj3/tests/Unity/src/unity.c > CMakeFiles/unity.dir/src/unity.c.i

Unity/CMakeFiles/unity.dir/src/unity.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/unity.dir/src/unity.c.s"
	cd /home/pramos/Documents/Univ/SETR_Proj3/tests/build/Unity && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/pramos/Documents/Univ/SETR_Proj3/tests/Unity/src/unity.c -o CMakeFiles/unity.dir/src/unity.c.s

# Object files for target unity
unity_OBJECTS = \
"CMakeFiles/unity.dir/src/unity.c.o"

# External object files for target unity
unity_EXTERNAL_OBJECTS =

Unity/libunity.a: Unity/CMakeFiles/unity.dir/src/unity.c.o
Unity/libunity.a: Unity/CMakeFiles/unity.dir/build.make
Unity/libunity.a: Unity/CMakeFiles/unity.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/pramos/Documents/Univ/SETR_Proj3/tests/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library libunity.a"
	cd /home/pramos/Documents/Univ/SETR_Proj3/tests/build/Unity && $(CMAKE_COMMAND) -P CMakeFiles/unity.dir/cmake_clean_target.cmake
	cd /home/pramos/Documents/Univ/SETR_Proj3/tests/build/Unity && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/unity.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
Unity/CMakeFiles/unity.dir/build: Unity/libunity.a
.PHONY : Unity/CMakeFiles/unity.dir/build

Unity/CMakeFiles/unity.dir/clean:
	cd /home/pramos/Documents/Univ/SETR_Proj3/tests/build/Unity && $(CMAKE_COMMAND) -P CMakeFiles/unity.dir/cmake_clean.cmake
.PHONY : Unity/CMakeFiles/unity.dir/clean

Unity/CMakeFiles/unity.dir/depend:
	cd /home/pramos/Documents/Univ/SETR_Proj3/tests/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pramos/Documents/Univ/SETR_Proj3/tests /home/pramos/Documents/Univ/SETR_Proj3/tests/Unity /home/pramos/Documents/Univ/SETR_Proj3/tests/build /home/pramos/Documents/Univ/SETR_Proj3/tests/build/Unity /home/pramos/Documents/Univ/SETR_Proj3/tests/build/Unity/CMakeFiles/unity.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : Unity/CMakeFiles/unity.dir/depend

