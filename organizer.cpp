// File Organizer
// Sorts the files in a folder into subfolders by file type.
//   photo.JPG -> <folder>/Images/photo.JPG
//
// Features: case-insensitive extensions, no overwriting (duplicates are renamed),
// hidden/system files are skipped, and a --dry-run mode that only shows what
// would happen.
//
// Build: g++ -std=c++17 -Wall -Wextra organizer.cpp -o organizer
// Usage: ./organizer [folder] [--dry-run] [--help]

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

using CategoryMap = std::map<std::string, std::string>;

// Returns a lowercase copy of s, so ".JPG" and ".jpg" are treated the same.
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char ch) { return std::tolower(ch); });
    return s;
}

// Looks up the category for an extension such as ".jpg".
// Returns "Other" for unknown extensions and for files with no extension.
std::string getCategory(const std::string& ext, const CategoryMap& categories) {
    auto it = categories.find(toLower(ext));
    if (it != categories.end()) {
        return it->second;
    }
    return "Other";
}

// True for hidden files (.DS_Store, .gitignore) and common system files.
bool isHiddenOrSystem(const std::string& name) {
    if (!name.empty() && name[0] == '.') {
        return true;
    }
    std::string lower = toLower(name);
    return lower == "desktop.ini" || lower == "thumbs.db";
}

// Removes one pair of surrounding quotes, in case the path was pasted as "C:/my folder".
std::string stripQuotes(std::string s) {
    if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') ||
                          (s.front() == '\'' && s.back() == '\''))) {
        s = s.substr(1, s.size() - 2);
    }
    return s;
}

// Collects the regular files directly inside folder (subfolders are ignored).
// We collect first and move later, because changing a folder while looping
// over it can skip files or misbehave.
std::vector<fs::path> collectFiles(const fs::path& folder, int& hiddenSkipped) {
    std::vector<fs::path> files;
    for (const auto& entry : fs::directory_iterator(folder)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (isHiddenOrSystem(entry.path().filename().string())) {
            hiddenSkipped++;
            continue;
        }
        files.push_back(entry.path());
    }
    std::sort(files.begin(), files.end());  // predictable order
    return files;
}

// If dest already exists, returns a free name: photo.jpg -> photo_1.jpg -> photo_2.jpg ...
fs::path makeUniquePath(const fs::path& dest) {
    std::error_code ec;
    if (!fs::exists(dest, ec)) {
        return dest;
    }
    const std::string stem = dest.stem().string();
    const std::string ext = dest.extension().string();
    for (int n = 1;; n++) {
        fs::path candidate = dest.parent_path() / (stem + "_" + std::to_string(n) + ext);
        if (!fs::exists(candidate, ec)) {
            return candidate;
        }
    }
}

void printUsage() {
    std::cout << "File Organizer - sorts files into folders by type.\n\n"
              << "Usage:\n"
              << "  organizer [folder] [--dry-run]\n\n"
              << "Options:\n"
              << "  --dry-run   Show what would be moved, without changing anything\n"
              << "  --help, -h  Show this message\n\n"
              << "If no folder is given, the program asks for one.\n";
}

int main(int argc, char* argv[]) {
    const CategoryMap categories = {
        {".jpg", "Images"},    {".jpeg", "Images"},   {".png", "Images"},
        {".gif", "Images"},    {".pdf", "Documents"}, {".docx", "Documents"},
        {".txt", "Documents"}, {".xlsx", "Documents"}, {".mp3", "Audio"},
        {".wav", "Audio"},     {".mp4", "Video"},     {".mkv", "Video"},
        {".avi", "Video"},     {".zip", "Archives"},  {".rar", "Archives"}};

    // ---- read the command-line arguments ----
    bool dryRun = false;
    std::string input;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        } else if (arg == "--dry-run") {
            dryRun = true;
        } else if (arg.rfind("--", 0) == 0) {  // starts with "--" but is not known
            std::cerr << "Error: unknown option \"" << arg << "\".\n\n";
            printUsage();
            return 1;
        } else if (input.empty()) {
            input = arg;
        } else {
            std::cerr << "Error: please give only one folder.\n";
            return 1;
        }
    }

    if (input.empty()) {
        std::cout << "Enter folder path: ";
        if (!std::getline(std::cin, input)) {
            return 1;
        }
    }
    input = stripQuotes(input);

    fs::path folder = input;
    if (input.empty() || !fs::is_directory(folder)) {
        std::cerr << "Error: \"" << input << "\" is not a valid folder.\n";
        return 1;
    }

    try {
        int hiddenSkipped = 0;
        std::vector<fs::path> files = collectFiles(folder, hiddenSkipped);

        if (dryRun) {
            std::cout << "DRY RUN: nothing will be changed.\n\n";
        }

        std::map<std::string, int> counts;  // files moved (or that would move) per category
        int renamed = 0;
        int failed = 0;

        for (const auto& file : files) {
            const std::string name = file.filename().string();
            const std::string category = getCategory(file.extension().string(), categories);

            fs::path targetDir = folder / category;
            fs::path dest = makeUniquePath(targetDir / file.filename());
            const bool wasRenamed = (dest.filename() != file.filename());

            if (!dryRun) {
                std::error_code ec;
                fs::create_directories(targetDir, ec);
                if (ec) {
                    std::cerr << "Could not create folder " << targetDir.string() << ": "
                              << ec.message() << "\n";
                    failed++;
                    continue;
                }
                fs::rename(file, dest, ec);
                if (ec) {
                    std::cerr << "Could not move " << name << ": " << ec.message() << "\n";
                    failed++;
                    continue;
                }
            }

            std::cout << (dryRun ? "Would move " : "Moved ") << name << "  ->  " << category;
            if (wasRenamed) {
                std::cout << "/" << dest.filename().string() << "  (name already taken)";
                renamed++;
            }
            std::cout << "\n";
            counts[category]++;
        }

        // ---- summary ----
        int total = 0;
        for (const auto& [name, n] : counts) {
            total += n;
        }

        std::cout << "\n" << (dryRun ? "Dry run finished. Would move " : "Done. Moved ")
                  << total << " file(s).\n";
        for (const auto& [name, n] : counts) {
            std::cout << "  " << std::left << std::setw(12) << name << n << "\n";
        }
        if (renamed > 0) {
            std::cout << "Renamed " << renamed << " file(s) to avoid overwriting.\n";
        }
        if (hiddenSkipped > 0) {
            std::cout << "Skipped " << hiddenSkipped << " hidden/system file(s).\n";
        }
        if (failed > 0) {
            std::cout << failed << " file(s) could not be moved.\n";
            return 1;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
