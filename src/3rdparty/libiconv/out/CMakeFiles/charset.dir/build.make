# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.2

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
CMAKE_SOURCE_DIR = /home/houzh/alitest/src/3rdparty/libiconv

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/houzh/alitest/src/3rdparty/libiconv/out

# Include any dependencies generated for this target.
include CMakeFiles/charset.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/charset.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/charset.dir/flags.make

CMakeFiles/charset.dir/libcharset/lib/localcharset.c.o: CMakeFiles/charset.dir/flags.make
CMakeFiles/charset.dir/libcharset/lib/localcharset.c.o: ../libcharset/lib/localcharset.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/houzh/alitest/src/3rdparty/libiconv/out/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/charset.dir/libcharset/lib/localcharset.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/charset.dir/libcharset/lib/localcharset.c.o   -c /home/houzh/alitest/src/3rdparty/libiconv/libcharset/lib/localcharset.c

CMakeFiles/charset.dir/libcharset/lib/localcharset.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/charset.dir/libcharset/lib/localcharset.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/houzh/alitest/src/3rdparty/libiconv/libcharset/lib/localcharset.c > CMakeFiles/charset.dir/libcharset/lib/localcharset.c.i

CMakeFiles/charset.dir/libcharset/lib/localcharset.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/charset.dir/libcharset/lib/localcharset.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/houzh/alitest/src/3rdparty/libiconv/libcharset/lib/localcharset.c -o CMakeFiles/charset.dir/libcharset/lib/localcharset.c.s

CMakeFiles/charset.dir/libcharset/lib/localcharset.c.o.requires:
.PHONY : CMakeFiles/charset.dir/libcharset/lib/localcharset.c.o.requires

CMakeFiles/charset.dir/libcharset/lib/localcharset.c.o.provides: CMakeFiles/charset.dir/libcharset/lib/localcharset.c.o.requires
	$(MAKE) -f CMakeFiles/charset.dir/build.make CMakeFiles/charset.dir/libcharset/lib/localcharset.c.o.provides.build
.PHONY : CMakeFiles/charset.dir/libcharset/lib/localcharset.c.o.provides

CMakeFiles/charset.dir/libcharset/lib/localcharset.c.o.provides.build: CMakeFiles/charset.dir/libcharset/lib/localcharset.c.o

CMakeFiles/charset.dir/libcharset/lib/relocatable.c.o: CMakeFiles/charset.dir/flags.make
CMakeFiles/charset.dir/libcharset/lib/relocatable.c.o: ../libcharset/lib/relocatable.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/houzh/alitest/src/3rdparty/libiconv/out/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/charset.dir/libcharset/lib/relocatable.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/charset.dir/libcharset/lib/relocatable.c.o   -c /home/houzh/alitest/src/3rdparty/libiconv/libcharset/lib/relocatable.c

CMakeFiles/charset.dir/libcharset/lib/relocatable.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/charset.dir/libcharset/lib/relocatable.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/houzh/alitest/src/3rdparty/libiconv/libcharset/lib/relocatable.c > CMakeFiles/charset.dir/libcharset/lib/relocatable.c.i

CMakeFiles/charset.dir/libcharset/lib/relocatable.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/charset.dir/libcharset/lib/relocatable.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/houzh/alitest/src/3rdparty/libiconv/libcharset/lib/relocatable.c -o CMakeFiles/charset.dir/libcharset/lib/relocatable.c.s

CMakeFiles/charset.dir/libcharset/lib/relocatable.c.o.requires:
.PHONY : CMakeFiles/charset.dir/libcharset/lib/relocatable.c.o.requires

CMakeFiles/charset.dir/libcharset/lib/relocatable.c.o.provides: CMakeFiles/charset.dir/libcharset/lib/relocatable.c.o.requires
	$(MAKE) -f CMakeFiles/charset.dir/build.make CMakeFiles/charset.dir/libcharset/lib/relocatable.c.o.provides.build
.PHONY : CMakeFiles/charset.dir/libcharset/lib/relocatable.c.o.provides

CMakeFiles/charset.dir/libcharset/lib/relocatable.c.o.provides.build: CMakeFiles/charset.dir/libcharset/lib/relocatable.c.o

# Object files for target charset
charset_OBJECTS = \
"CMakeFiles/charset.dir/libcharset/lib/localcharset.c.o" \
"CMakeFiles/charset.dir/libcharset/lib/relocatable.c.o"

# External object files for target charset
charset_EXTERNAL_OBJECTS =

libcharset.so: CMakeFiles/charset.dir/libcharset/lib/localcharset.c.o
libcharset.so: CMakeFiles/charset.dir/libcharset/lib/relocatable.c.o
libcharset.so: CMakeFiles/charset.dir/build.make
libcharset.so: CMakeFiles/charset.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C shared library libcharset.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/charset.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/charset.dir/build: libcharset.so
.PHONY : CMakeFiles/charset.dir/build

CMakeFiles/charset.dir/requires: CMakeFiles/charset.dir/libcharset/lib/localcharset.c.o.requires
CMakeFiles/charset.dir/requires: CMakeFiles/charset.dir/libcharset/lib/relocatable.c.o.requires
.PHONY : CMakeFiles/charset.dir/requires

CMakeFiles/charset.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/charset.dir/cmake_clean.cmake
.PHONY : CMakeFiles/charset.dir/clean

CMakeFiles/charset.dir/depend:
	cd /home/houzh/alitest/src/3rdparty/libiconv/out && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/houzh/alitest/src/3rdparty/libiconv /home/houzh/alitest/src/3rdparty/libiconv /home/houzh/alitest/src/3rdparty/libiconv/out /home/houzh/alitest/src/3rdparty/libiconv/out /home/houzh/alitest/src/3rdparty/libiconv/out/CMakeFiles/charset.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/charset.dir/depend

