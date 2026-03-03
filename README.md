# MFT Parser (C++)

A high-performance C++ library and command-line tool for parsing NTFS Master File Table ($MFT) and related forensic artifacts. This project is a C++ port of Eric Zimmerman's MFT Parser, designed for cross-platform compatibility and efficiency.

## Overview

The tool provides a comprehensive analysis of NTFS file systems, allowing for the extraction of metadata, recovery of deleted files, and inspection of system files like $Boot, $LogFile, $Secure, and $UsnJrnl.

## Features

*   **Parse $MFT**: Extracts full file record metadata (SI, FN, Data, etc.) to CSV and JSON.
*   **Parse $Boot**: Analyzes NTFS boot sector and cluster geometry.
*   **Parse $LogFile**: Scans NTFS transaction logs for record pages.
*   **Parse $UsnJrnl**: Processes the Update Sequence Number (USN) journal.
*   **Parse $Secure**: Extracts security descriptors from the $SDS stream.
*   **Data Recovery**:
    *   Recover resident files (stored directly in MFT records).
    *   Export non-resident file cluster information for external carving.
*   **Cross-Platform**: Builds and runs on macOS, Linux, and Windows.
*   **Recursive Analysis**: Automatically detects and processes supported artifacts in a directory structure.

## Building

This project uses CMake. Ensure you have CMake (3.10+) and a C++17 compatible compiler installed.

**Generate Build Files**

    mkdir build
    cd build
    cmake ..

**Compile**

    make -j4

This will create the following binaries:
*   `mft_cli`: The main command-line tool.
*   `unit_tests`: Test suite.
*   `mft_example`: Example usage of the library.

## Usage

The CLI tool supports several commands for different artifacts.

**Analyze a Directory**
Recursively scans a directory for all supported NTFS files ($MFT, $Boot, $LogFile, etc.) and processes them.

    ./mft_cli all "/path/to/extracted/ntfs/root" output_folder

**Parse Single Artifacts**

*   **MFT**: `./mft_cli mft $MFT output_dir`
*   **Boot Sector**: `./mft_cli boot $Boot output_dir`
*   **USN Journal**: `./mft_cli i30 $UsnJrnl output_dir` (Use `recover` for raw stream recovery details)

**Recover Deleted Files**
Extracts resident files and generates recovery info for non-resident files from an MFT.

    ./mft_cli recover $MFT output_dir

## Output Formats

*   **CSV**: Detailed record lists for MFT, USN, and parsing summaries.
*   **JSON**: Statistical summaries and boot sector info.
*   **Raw**: Recovered file content (for resident data).
*   **Info Files**: Text files containing cluster runs for non-resident data recovery.

## Testing

Run the unit tests to verify the build integrity:

    ./unit_tests

## License

GNU General Public License v3.0

## Credits

Inspired by the original C# implementation by Eric Zimmerman: https://github.com/EricZimmerman/MFT

