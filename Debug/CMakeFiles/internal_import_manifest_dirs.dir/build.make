# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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
CMAKE_SOURCE_DIR = /home/zeus/projects/pong-source

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zeus/projects/pong-source/Debug

# Utility rule file for internal_import_manifest_dirs.

# Include the progress variables for this target.
include CMakeFiles/internal_import_manifest_dirs.dir/progress.make

CMakeFiles/internal_import_manifest_dirs: manifests/default/MANIFEST.json

manifests/default/MANIFEST.json:
	$(CMAKE_COMMAND) -E cmake_progress_report /home/zeus/projects/pong-source/Debug/CMakeFiles $(CMAKE_PROGRESS_1)
	@echo "Generating manifests/default/MANIFEST.json"
	/usr/bin/cmake -E make_directory /home/zeus/projects/pong-source/Debug/manifests/default
	/usr/bin/cmake -E create_symlink /home/zeus/projects/pong-source/src/MANIFEST.json /home/zeus/projects/pong-source/Debug/manifests/default/MANIFEST.json

internal_import_manifest_dirs: CMakeFiles/internal_import_manifest_dirs
internal_import_manifest_dirs: manifests/default/MANIFEST.json
internal_import_manifest_dirs: CMakeFiles/internal_import_manifest_dirs.dir/build.make
.PHONY : internal_import_manifest_dirs

# Rule to build all files generated by this target.
CMakeFiles/internal_import_manifest_dirs.dir/build: internal_import_manifest_dirs
.PHONY : CMakeFiles/internal_import_manifest_dirs.dir/build

CMakeFiles/internal_import_manifest_dirs.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/internal_import_manifest_dirs.dir/cmake_clean.cmake
.PHONY : CMakeFiles/internal_import_manifest_dirs.dir/clean

CMakeFiles/internal_import_manifest_dirs.dir/depend:
	cd /home/zeus/projects/pong-source/Debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zeus/projects/pong-source /home/zeus/projects/pong-source /home/zeus/projects/pong-source/Debug /home/zeus/projects/pong-source/Debug /home/zeus/projects/pong-source/Debug/CMakeFiles/internal_import_manifest_dirs.dir/DependInfo.cmake
.PHONY : CMakeFiles/internal_import_manifest_dirs.dir/depend

