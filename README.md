# Shared Memory Text Editor

## Overview
This project implements a collaborative text editor using shared memory and multithreading in C. The editor leverages `ncurses` for the user interface, shared memory (`shmget`, `shmat`, etc.) for inter-process communication, and `pthread` for managing concurrent access to shared resources.

## Features
- Supports editing text collaboratively across multiple processes.
- Real-time display of shared memory content.
- Arrow key navigation.
- Shared memory management, including creation and cleanup.

## Dependencies
- `gcc`: The GNU Compiler Collection.
- `ncurses`: A library for terminal-based UI.
- POSIX shared memory.
- POSIX threads (`pthread`).

## Compilation and Build Instructions

### Compile Command
```bash
gcc -Wall -c editor_2.c
```

### Build Command
```bash
gcc -Wall -o editor editor_2.c -pthread -lm -D_LARGE_THREADS -lncurses -lrt
```

## Usage

### Running the Editor
1. Launch the editor executable:
   ```bash
   ./editor
   ```
2. On startup, choose whether to create a new file or open an existing shared memory region.
   - Input `Y` to create a new file.
   - Input `N` to attach to an existing shared memory region and provide its key.

### Editing
- **Arrow Keys**: Navigate through the text.
- **Typing**: Enter text directly.
- **`/` Key**: Exit the editor.

### Cleaning Up Shared Memory
Upon exiting, the editor prompts whether to delete the shared memory region. Input `Y` to delete or `N` to retain it.

## Code Breakdown

### Key Components
1. **Shared Memory Initialization**
   - Shared memory is created or attached using `shmget` and `shmat`.
   - Synchronization between processes is managed using a shared mutex.

2. **UI and Navigation**
   - The editor uses `ncurses` for terminal-based navigation and rendering.
   - Cursor movement is managed with arrow keys.

3. **Real-Time Display**
   - A background thread continuously updates the terminal display by reading the shared memory.
   - The `display_shared_mem` thread ensures all updates are visible in real-time.

4. **Thread Synchronization**
   - A mutex locks critical sections to ensure thread-safe access to shared memory.
   - The `dirty_bit` and `local_bit` flags track changes to the shared memory.

5. **Memory Cleanup**
   - The program offers an option to delete the shared memory regions upon exit.

### Key Functions

#### `write_into_mem`
Writes input characters into the shared memory.

#### `display_shared_mem`
Displays the contents of the shared memory in real-time, ensuring synchronization between processes.

#### `key_operation`
Handles arrow key inputs to navigate the text cursor within the shared memory region.

#### `main`
Coordinates shared memory creation/attachment, initializes UI, spawns threads, and manages the main editor loop.

## Shared Memory Layout
- **Shared Memory Segment**: Stores the text data being edited.
- **File Attribute Segment**: Tracks metadata like the file length.
- **Dirty Bit Segment**: Indicates changes to shared memory for synchronization.
- **Mutex Segment**: Ensures thread-safe access to shared memory.

## Limitations
- The editor supports only basic text editing functionalities.
- Maximum file size is limited by the shared memory size.
- The editor runs in a terminal and does not support advanced text editing features like undo/redo or file saving/loading.

## Future Improvements
- Add support for multiple files.
- Implement undo/redo functionality.
- Allow saving and loading files from disk.
- Enhance navigation with support for line breaks and word-wrapping.

## References
- `man shmget`
- `man pthread`
- `man ncurses`
- POSIX shared memory and threading documentation.
