# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

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
CMAKE_COMMAND = /opt/clion-2018.3.4/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /opt/clion-2018.3.4/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/user/tusov

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/user/tusov/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/parser.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/parser.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/parser.dir/flags.make

CMakeFiles/parser.dir/src/parser/src/LogicChunk.cpp.o: CMakeFiles/parser.dir/flags.make
CMakeFiles/parser.dir/src/parser/src/LogicChunk.cpp.o: ../src/parser/src/LogicChunk.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/user/tusov/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/parser.dir/src/parser/src/LogicChunk.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/parser.dir/src/parser/src/LogicChunk.cpp.o -c /home/user/tusov/src/parser/src/LogicChunk.cpp

CMakeFiles/parser.dir/src/parser/src/LogicChunk.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/parser.dir/src/parser/src/LogicChunk.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/user/tusov/src/parser/src/LogicChunk.cpp > CMakeFiles/parser.dir/src/parser/src/LogicChunk.cpp.i

CMakeFiles/parser.dir/src/parser/src/LogicChunk.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/parser.dir/src/parser/src/LogicChunk.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/user/tusov/src/parser/src/LogicChunk.cpp -o CMakeFiles/parser.dir/src/parser/src/LogicChunk.cpp.s

CMakeFiles/parser.dir/src/parser/src/LogicPacketsStream.cpp.o: CMakeFiles/parser.dir/flags.make
CMakeFiles/parser.dir/src/parser/src/LogicPacketsStream.cpp.o: ../src/parser/src/LogicPacketsStream.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/user/tusov/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/parser.dir/src/parser/src/LogicPacketsStream.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/parser.dir/src/parser/src/LogicPacketsStream.cpp.o -c /home/user/tusov/src/parser/src/LogicPacketsStream.cpp

CMakeFiles/parser.dir/src/parser/src/LogicPacketsStream.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/parser.dir/src/parser/src/LogicPacketsStream.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/user/tusov/src/parser/src/LogicPacketsStream.cpp > CMakeFiles/parser.dir/src/parser/src/LogicPacketsStream.cpp.i

CMakeFiles/parser.dir/src/parser/src/LogicPacketsStream.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/parser.dir/src/parser/src/LogicPacketsStream.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/user/tusov/src/parser/src/LogicPacketsStream.cpp -o CMakeFiles/parser.dir/src/parser/src/LogicPacketsStream.cpp.s

CMakeFiles/parser.dir/src/parser/src/structs.cpp.o: CMakeFiles/parser.dir/flags.make
CMakeFiles/parser.dir/src/parser/src/structs.cpp.o: ../src/parser/src/structs.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/user/tusov/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/parser.dir/src/parser/src/structs.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/parser.dir/src/parser/src/structs.cpp.o -c /home/user/tusov/src/parser/src/structs.cpp

CMakeFiles/parser.dir/src/parser/src/structs.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/parser.dir/src/parser/src/structs.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/user/tusov/src/parser/src/structs.cpp > CMakeFiles/parser.dir/src/parser/src/structs.cpp.i

CMakeFiles/parser.dir/src/parser/src/structs.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/parser.dir/src/parser/src/structs.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/user/tusov/src/parser/src/structs.cpp -o CMakeFiles/parser.dir/src/parser/src/structs.cpp.s

CMakeFiles/parser.dir/src/parser/src/token.cpp.o: CMakeFiles/parser.dir/flags.make
CMakeFiles/parser.dir/src/parser/src/token.cpp.o: ../src/parser/src/token.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/user/tusov/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/parser.dir/src/parser/src/token.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/parser.dir/src/parser/src/token.cpp.o -c /home/user/tusov/src/parser/src/token.cpp

CMakeFiles/parser.dir/src/parser/src/token.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/parser.dir/src/parser/src/token.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/user/tusov/src/parser/src/token.cpp > CMakeFiles/parser.dir/src/parser/src/token.cpp.i

CMakeFiles/parser.dir/src/parser/src/token.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/parser.dir/src/parser/src/token.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/user/tusov/src/parser/src/token.cpp -o CMakeFiles/parser.dir/src/parser/src/token.cpp.s

CMakeFiles/parser.dir/src/parser/src/VovitChunk.cpp.o: CMakeFiles/parser.dir/flags.make
CMakeFiles/parser.dir/src/parser/src/VovitChunk.cpp.o: ../src/parser/src/VovitChunk.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/user/tusov/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/parser.dir/src/parser/src/VovitChunk.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/parser.dir/src/parser/src/VovitChunk.cpp.o -c /home/user/tusov/src/parser/src/VovitChunk.cpp

CMakeFiles/parser.dir/src/parser/src/VovitChunk.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/parser.dir/src/parser/src/VovitChunk.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/user/tusov/src/parser/src/VovitChunk.cpp > CMakeFiles/parser.dir/src/parser/src/VovitChunk.cpp.i

CMakeFiles/parser.dir/src/parser/src/VovitChunk.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/parser.dir/src/parser/src/VovitChunk.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/user/tusov/src/parser/src/VovitChunk.cpp -o CMakeFiles/parser.dir/src/parser/src/VovitChunk.cpp.s

# Object files for target parser
parser_OBJECTS = \
"CMakeFiles/parser.dir/src/parser/src/LogicChunk.cpp.o" \
"CMakeFiles/parser.dir/src/parser/src/LogicPacketsStream.cpp.o" \
"CMakeFiles/parser.dir/src/parser/src/structs.cpp.o" \
"CMakeFiles/parser.dir/src/parser/src/token.cpp.o" \
"CMakeFiles/parser.dir/src/parser/src/VovitChunk.cpp.o"

# External object files for target parser
parser_EXTERNAL_OBJECTS =

libparser.a: CMakeFiles/parser.dir/src/parser/src/LogicChunk.cpp.o
libparser.a: CMakeFiles/parser.dir/src/parser/src/LogicPacketsStream.cpp.o
libparser.a: CMakeFiles/parser.dir/src/parser/src/structs.cpp.o
libparser.a: CMakeFiles/parser.dir/src/parser/src/token.cpp.o
libparser.a: CMakeFiles/parser.dir/src/parser/src/VovitChunk.cpp.o
libparser.a: CMakeFiles/parser.dir/build.make
libparser.a: CMakeFiles/parser.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/user/tusov/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX static library libparser.a"
	$(CMAKE_COMMAND) -P CMakeFiles/parser.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/parser.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/parser.dir/build: libparser.a

.PHONY : CMakeFiles/parser.dir/build

CMakeFiles/parser.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/parser.dir/cmake_clean.cmake
.PHONY : CMakeFiles/parser.dir/clean

CMakeFiles/parser.dir/depend:
	cd /home/user/tusov/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/user/tusov /home/user/tusov /home/user/tusov/cmake-build-debug /home/user/tusov/cmake-build-debug /home/user/tusov/cmake-build-debug/CMakeFiles/parser.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/parser.dir/depend

