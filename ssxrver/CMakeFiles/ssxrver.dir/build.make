# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.11

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/sunxiaochuan/sunshouxun/WebServer/ssxrver

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sunxiaochuan/sunshouxun/WebServer/ssxrver

# Include any dependencies generated for this target.
include CMakeFiles/ssxrver.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ssxrver.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ssxrver.dir/flags.make

CMakeFiles/ssxrver.dir/main.cpp.o: CMakeFiles/ssxrver.dir/flags.make
CMakeFiles/ssxrver.dir/main.cpp.o: main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sunxiaochuan/sunshouxun/WebServer/ssxrver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ssxrver.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ssxrver.dir/main.cpp.o -c /home/sunxiaochuan/sunshouxun/WebServer/ssxrver/main.cpp

CMakeFiles/ssxrver.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ssxrver.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sunxiaochuan/sunshouxun/WebServer/ssxrver/main.cpp > CMakeFiles/ssxrver.dir/main.cpp.i

CMakeFiles/ssxrver.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ssxrver.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sunxiaochuan/sunshouxun/WebServer/ssxrver/main.cpp -o CMakeFiles/ssxrver.dir/main.cpp.s

# Object files for target ssxrver
ssxrver_OBJECTS = \
"CMakeFiles/ssxrver.dir/main.cpp.o"

# External object files for target ssxrver
ssxrver_EXTERNAL_OBJECTS =

ssxrver: CMakeFiles/ssxrver.dir/main.cpp.o
ssxrver: CMakeFiles/ssxrver.dir/build.make
ssxrver: base/libBaseFunctions.a
ssxrver: net/libNetFunctions.a
ssxrver: http/libHttpFunctions.a
ssxrver: CMakeFiles/ssxrver.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sunxiaochuan/sunshouxun/WebServer/ssxrver/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ssxrver"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ssxrver.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ssxrver.dir/build: ssxrver

.PHONY : CMakeFiles/ssxrver.dir/build

CMakeFiles/ssxrver.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ssxrver.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ssxrver.dir/clean

CMakeFiles/ssxrver.dir/depend:
	cd /home/sunxiaochuan/sunshouxun/WebServer/ssxrver && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sunxiaochuan/sunshouxun/WebServer/ssxrver /home/sunxiaochuan/sunshouxun/WebServer/ssxrver /home/sunxiaochuan/sunshouxun/WebServer/ssxrver /home/sunxiaochuan/sunshouxun/WebServer/ssxrver /home/sunxiaochuan/sunshouxun/WebServer/ssxrver/CMakeFiles/ssxrver.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ssxrver.dir/depend

