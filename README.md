# File Organizer (C++17)

A small command-line tool that tidies a messy folder by sorting its files into subfolders by type (Images, Documents, Audio, Video, Archives, Other).

It never overwrites anything: if a file with the same name already exists in the target folder, the new one is renamed (`photo.jpg` -> `photo_1.jpg`). A `--dry-run` mode lets you preview every move before changing a single file.

## Features

- Sorts files by extension into category folders
- Case-insensitive extensions (`.JPG` and `.jpg` are treated the same)
- **Never overwrites files**: duplicates get a numbered name
- **Dry-run mode** that shows what would happen without touching anything
- Skips hidden and system files (`.DS_Store`, `Thumbs.db`, `desktop.ini`)
- Unknown types and files without an extension go to `Other/`
- Subfolders are left alone
- Friendly error messages and proper exit codes (0 = success, 1 = error)
- Summary of what was moved at the end

## Build

Requires a C++17 compiler (g++ 8+, clang, or MSVC).

```bash
g++ -std=c++17 -Wall -Wextra organizer.cpp -o organizer
```

Or with CMake:

```bash
cmake -S . -B build
cmake --build build
```

On Windows with Visual Studio's compiler:

```
cl /std:c++17 /EHsc organizer.cpp
```

## Usage

```bash
./organizer [folder] [--dry-run] [--help]
```

| Option | Meaning |
|---|---|
| `folder` | Folder to organize. If omitted, the program asks for it |
| `--dry-run` | Show what would be moved, change nothing |
| `--help`, `-h` | Show usage |

**Tip:** run with `--dry-run` first, and try the tool on a copy of a folder before using it on important files.

### Example

Before:

```
demo/
  photo.JPG
  vacation.png
  song.mp3
  report.pdf
  notes.txt
  archive.ZIP
  movie.mkv
  weird.xyz
  README
  .DS_Store
  old_stuff/
```

Preview:

```
$ ./organizer demo --dry-run
DRY RUN: nothing will be changed.

Would move README  ->  Other
Would move archive.ZIP  ->  Archives
Would move movie.mkv  ->  Video
Would move notes.txt  ->  Documents
Would move photo.JPG  ->  Images
Would move report.pdf  ->  Documents
Would move song.mp3  ->  Audio
Would move vacation.png  ->  Images
Would move weird.xyz  ->  Other

Dry run finished. Would move 9 file(s).
  Archives    1
  Audio       1
  Documents   2
  Images      2
  Other       2
  Video       1
Skipped 1 hidden/system file(s).
```

After the real run:

```
demo/
  Archives/archive.ZIP
  Audio/song.mp3
  Documents/notes.txt
  Documents/report.pdf
  Images/photo.JPG
  Images/vacation.png
  Other/README
  Other/weird.xyz
  Video/movie.mkv
  .DS_Store
  old_stuff/
```

If `Images/photo.JPG` already exists and another `photo.JPG` is organized later, the new one is saved as `Images/photo_1.JPG`.

## Categories

| Folder | Extensions |
|---|---|
| Images | .jpg .jpeg .png .gif |
| Documents | .pdf .docx .txt .xlsx |
| Audio | .mp3 .wav |
| Video | .mp4 .mkv .avi |
| Archives | .zip .rar |
| Other | everything else |

To add a type, add one line to the `categories` map in `main()`.

## How it works

1. Reads the folder from the command line (or asks for it) and checks that it exists.
2. **Collects** the regular files first into a `std::vector<fs::path>`, so the folder isn't modified while it is being read.
3. For each file, looks up its category in a `std::map` using a lowercased extension.
4. Builds the destination (`folder/Category/name`) and, if the name is taken, finds a free numbered name.
5. Creates the category folder and moves the file with `std::filesystem::rename`, using `std::error_code` so one failure doesn't stop the rest.
6. Prints a summary.

## Skills practiced

- C++17 `std::filesystem` (paths, directory iteration, rename, create_directories)
- `std::map`, `std::vector`, `std::string` and `<algorithm>` (`transform`, `sort`)
- Lambdas, command-line arguments, error handling with `std::error_code` and exceptions
- Breaking a program into small, single-purpose functions

## Possible improvements

- Recursive mode to organize subfolders too
- An undo option that reverses the last run
- A config file for custom categories
- Sorting by date instead of type

## Author

*Your name here.*
