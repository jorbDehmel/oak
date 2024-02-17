/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "oakc_fns.hpp"
#include "options.hpp"

#define pm_assert(expression, message)                                                                                 \
    ((bool)(expression) ? true : throw package_error(message " (Failed assertion: '" #expression "')"))

std::ostream &operator<<(std::ostream &strm, const PackageInfo &info)
{
    strm << "Package '" << info.name << "'\n"
         << "Version '" << info.version << "'\n"
         << "License '" << info.license << "'\n"
         << "Released '" << info.date << "'\n"
         << "Author(s) '" << info.author << "'\n"
         << "Email(s) '" << info.email << "'\n"
         << "Via '" << info.source << ":/" << info.path << "'\n\n"
         << info.about << "\n\n"
         << "Include path '/usr/include/oak/" << info.name << "/" << info.toInclude << "'\n"
         << "System Deps: '" << info.sysDeps << "'\n"
         << "Oak Deps: '" << info.oakDeps << "'\n";

    return strm;
}

void install(const std::string &What, AcornSettings &settings)
{
    std::string line;

    if (settings.installCommand == "")
    {
        // Get os-release info
        pm_assert(system("cat /etc/os-release | grep ^ID= > .oak_build/temp.txt") == 0, "Failed to poll OS name");

        std::ifstream osName(".oak_build/temp.txt");
        pm_assert(osName.is_open(), "Failed to poll OS name");
        getline(osName, line);
        osName.close();

        if (line.substr(0, 3) == "ID=")
        {
            line.erase(0, 3);
        }

        for (int i = 0; i < line.size(); i++)
        {
            line[i] = tolower(line[i]);
        }

        if (line == "arch")
        {
            settings.installCommand = "sudo pacman --needed -S ";
        }
        else if (line == "ubuntu")
        {
            settings.installCommand = "sudo apt-get install ";
        }
        else if (line == "fedora")
        {
            settings.installCommand = "sudo dnf install ";
        }
    }

    int result = -1;
    while (result != 0)
    {
        if (settings.installCommand == "")
        {
            std::cout << tags::yellow_bold
                      << "Local Linux installation command could not be automatically detected\n"
                         "for distribution '"
                      << line << "' during installation of packages '" << What
                      << "'\n"
                         "Please enter a command prefix which will install all packages\n"
                         "which follow it (for example, in Ubuntu: `sudo apt-get install`).\n"
                         "To manually install these instead, enter 'MANUAL'.\n"
                      << "Command: " << tags::reset;

            getline(std::cin, settings.installCommand);
            if (settings.installCommand == "MANUAL")
            {
                std::cout << tags::yellow_bold << "Skipping automatic installation. Be sure to manually install\n"
                          << "package(s) '" << What << "' or things may break!\n"
                          << tags::reset << std::flush;
                break;
            }
        }

        std::string command = settings.installCommand + " " + What + " 2>&1 > /dev/null";
        result = system(command.c_str());

        if (result != 0)
        {
            std::cout << tags::red_bold << "Command `" << command << "` failed. Please check your install command.\n"
                      << "Note: If this is persistant, enter 'MANUAL' to skip this step. However, be\n"
                      << "sure to install the requested package(s) ('" << What << "') manually.\n"
                      << tags::reset;
            settings.installCommand = "";
        }
    }

    return;
}

// Remove any leading or trailing quotes
void cleanString(std::string &What)
{
    while (What.front() == '"' || What.front() == '\'')
    {
        What.erase(What.begin());
    }

    while (What.back() == '"' || What.back() == '\'')
    {
        What.pop_back();
    }

    return;
}

PackageInfo loadPackageInfo(const std::string &Filepath, AcornSettings &settings)
{
    std::ifstream inp(Filepath);
    pm_assert(inp.is_open(), "Failed to load file '" + Filepath + "'");

    PackageInfo toAdd;

    std::string name, content, garbage;
    while (!inp.eof())
    {
        inp >> name;
        if (inp.eof())
        {
            break;
        }

        // Commenting
        if (name.size() >= 2 && name.substr(0, 2) == "//")
        {
            getline(inp, garbage);
            continue;
        }

        inp >> garbage;
        pm_assert(garbage == "=", "Malformed package info file.");
        if (inp.eof())
        {
            break;
        }

        inp >> content;

        if (content[0] == '"' || content[0] == '\'')
        {
            while (content.back() != content.front())
            {
                inp >> garbage;
                content += " " + garbage;

                if (inp.eof())
                {
                    break;
                }
            }

            content = content.substr(1, content.size() - 2);
        }

        cleanString(content);

        // Adds case resiliency
        for (int i = 0; i < name.size(); i++)
        {
            name[i] = toupper(name[i]);
        }

        if (name == "NAME")
        {
            toAdd.name = content;
        }
        else if (name == "VERSION")
        {
            toAdd.version = content;
        }
        else if (name == "LICENSE")
        {
            toAdd.license = content;
        }
        else if (name == "SOURCE")
        {
            toAdd.source = content;
        }
        else if (name == "AUTHOR")
        {
            toAdd.author = content;
        }
        else if (name == "EMAIL")
        {
            toAdd.email = content;
        }
        else if (name == "ABOUT")
        {
            toAdd.about = content;
        }
        else if (name == "DATE")
        {
            toAdd.date = content;
        }
        else if (name == "INCLUDE")
        {
            toAdd.toInclude = content;
        }
        else if (name == "SYS_DEPS")
        {
            toAdd.sysDeps = content;
        }
        else if (name == "OAK_DEPS")
        {
            toAdd.oakDeps = content;
        }
        else if (name == "PATH")
        {
            toAdd.path = content;
        }
        else
        {
            throw package_error("Invalid item '" + name + "'");
        }
    }

    inp.close();

    if (toAdd.name == "")
    {
        throw package_error("Malformed package file '" + Filepath + "': Must inclue NAME field.");
    }
    else if (toAdd.source == "")
    {
        throw package_error("Malformed package file '" + Filepath + "': Must inclue SOURCE field.");
    }

    settings.packages[toAdd.name] = toAdd;
    return toAdd;
}

void savePackageInfo(const PackageInfo &Info, const std::string &Filepath)
{
    std::ofstream out(Filepath);
    pm_assert(out.is_open(), "Failed to open file '" + Filepath + "'");

    out << "NAME = '" << Info.name << "'\n"
        << "VERSION = '" << Info.version << "'\n"
        << "LICENSE = '" << Info.license << "'\n"
        << "DATE = '" << Info.date << "'\n"
        << "AUTHOR = '" << Info.author << "'\n"
        << "EMAIL = '" << Info.email << "'\n"
        << "SOURCE = '" << Info.source << "'\n"
        << "ABOUT = '" << Info.about << "'\n"
        << "INCLUDE = '" << Info.toInclude << "'\n"
        << "SYS_DEPS = '" << Info.sysDeps << "'\n"
        << "OAK_DEPS = '" << Info.oakDeps << "'\n"
        << "PATH = '" << Info.path << "'\n";

    out.close();
    return;
}

/*
Imagine using a compiled language for scripting; Couldn't be me
*/
void downloadPackage(const std::string &URLArg, AcornSettings &settings, const bool &reinstall,
                     const std::string &pathInput)
{
    std::cout << tags::violet_bold << "\nInstalling Oak package '" << URLArg << "'\n\n" << tags::reset;

    std::string URL = URLArg;

    // Check if package is already installed
    for (auto p : settings.packages)
    {
        if (p.second.source == URLArg || p.second.name == URLArg)
        {
            if (reinstall)
            {
                URL = p.second.source;
            }
            else
            {
                std::cout << tags::yellow_bold << "Package '" << p.first << "' was already installed and loaded.\n"
                          << tags::reset;
                return;
            }
        }
    }

    // Create temp location
    std::string tempFolderName = PACKAGE_TEMP_LOCATION + "oak_tmp_" + std::to_string(time(NULL));

    // Clone package using git
    if (fs::exists(URL))
    {
        // Local file
        fs::create_directories(tempFolderName);
        fs::copy(URL + "/", tempFolderName, fs::copy_options::recursive);
    }
    else
    {
        std::cout << tags::yellow_bold << "Local copy failed.\nAttempting git copy...\n" << tags::reset;

        try
        {
            // URL
            if (system((std::string(CLONE_COMMAND) + URL + " " + tempFolderName + "  2>&1 > /dev/null && file " +
                        tempFolderName + "/*.oak > /dev/null")
                           .c_str()) != 0)
            {
                throw package_error("Git resolution failure; Likely an invalid source.");
            }
        }
        catch (package_error &e)
        {
            // Package from packages_list.txt

            std::cout << "Checking /usr/include/oak/packages_list.txt...\n";

            // Load packages_list.txt
            std::ifstream packagesList(PACKAGES_LIST_PATH);
            pm_assert(packagesList.is_open(),
                      URL + " is not a valid package URL or name and failed to load packages list.");

            std::string line;
            while (getline(packagesList, line))
            {
                if (strncmp(line.c_str(), "//", 2) == 0 || line.size() == 0)
                {
                    continue;
                }

                pm_assert(line.find(' ') != std::string::npos, "Malformed packages_list.txt");

                // name source path
                std::string name, source, path;

                auto breakPoint = line.find(' ');

                name = line.substr(0, breakPoint);
                source = line.substr(breakPoint + 1);

                pm_assert(source.find(' ') != std::string::npos, "Malformed packages_list.txt");
                breakPoint = source.find(' ');

                path = source.substr(breakPoint + 1);
                source = source.substr(0, breakPoint);

                if (name == URL)
                {
                    std::cout << "Package '" << name << "' found in /usr/include/oak/packages_list.txt w/ repo URL of '"
                              << source << "'\n";

                    downloadPackage(source, settings, reinstall, path);
                    return;
                }
            }

            // If it has gotten to this point, the following is true:
            // - It is not present in packages_list.txt
            // - It is not a valid URL
            // - It is not a valid local file
            // So it should error rather than proceed.

            std::cout << tags::red_bold << "\nPackage '" << URLArg << "'\n"
                      << "does not exist locally, is not a valid Git repo, and\n"
                      << "does not have an installation URL known by acorn.\n"
                      << tags::reset;
            throw package_error("Package '" + URLArg + "' does not exist in any form.");
        }
    }

    try
    {
        std::string path = pathInput;
        if (path == "")
        {
            path = ".";
        }

        while (!fs::exists(tempFolderName + "/" + path + "/" + INFO_FILE))
        {
            std::cout << tags::yellow_bold << "Failed to locate info file.\n"
                      << "Enter the path within the folder to install from [default .]: " << tags::reset;
            getline(std::cin, path);
            if (path == "")
            {
                path = ".";
            }

            pm_assert(path.find("..") == std::string::npos && path[0] != '/',
                      "Illegal path: May not contain '..' or begin with '/'.");
        }

        // At this point, info file must exist

        bool needsMake = false;
        needsMake = fs::exists(tempFolderName + "/" + path + "/Makefile") ||
                    fs::exists(tempFolderName + "/" + path + "/makefile");

        // Read info file
        PackageInfo info = loadPackageInfo(tempFolderName + "/" + path + "/" + INFO_FILE, settings);

        std::cout << tags::green << "Loaded package from " << URL << "\n" << info << '\n' << tags::reset << std::flush;

        // Ensure valid formatting
        for (char c : info.name)
        {
            pm_assert(!('A' <= c && c <= 'Z'), "Cannot install package with illegal name.");
        }

        // Prepare destination
        std::string destFolderName = PACKAGE_INCLUDE_PATH + info.name;

        try
        {
            if (fs::exists(destFolderName))
            {
                fs::remove_all(destFolderName);
            }

            fs::create_directories(destFolderName);
        }
        catch (fs::filesystem_error &e)
        {
            std::cout << tags::red_bold << e.what() << ": Retry as sudo/admin.\n" << tags::reset;
            throw package_error("Failed to create package file(s).");
        }

        // Install system deps
        if (info.sysDeps != "")
        {
            install(info.sysDeps, settings);
        }

        // Install Oak deps
        if (info.oakDeps != "")
        {
            // Split into packages and install them
            std::string current;
            for (const char &c : info.oakDeps)
            {
                if (c == ' ')
                {
                    if (settings.packages.count(current) == 0)
                    {
                        downloadPackage(current, settings);
                    }
                    current = "";
                }
                else
                {
                    current.push_back(c);
                }
            }

            if (current != "" && settings.packages.count(current) == 0)
            {
                downloadPackage(current, settings);
            }
        }

        // Make package if needed
        if (needsMake)
        {
            if (system(("make -C " + tempFolderName + "/" + path +
                        "  2>&1 > oak_package_makefile.log && rm oak_package_makefile.log")
                           .c_str()) != 0)
            {
                throw package_error("Make failure; See oak_package_makefile.log for details.");
            }
        }

        // Copy files
        system(("sudo cp " + tempFolderName + "/" + path + "/*.c " + destFolderName).c_str());
        system(("sudo cp " + tempFolderName + "/" + path + "/*.h " + destFolderName).c_str());
        system(("sudo cp " + tempFolderName + "/" + path + "/*.o " + destFolderName).c_str());
        system(("sudo cp " + tempFolderName + "/" + path + "/*.oak " + destFolderName).c_str());
        system(("sudo cp " + tempFolderName + "/" + path + "/*.txt " + destFolderName).c_str());

        // Clean up garbage; Doesn't really matter if this fails
        if (fs::remove_all(PACKAGE_TEMP_LOCATION) == 0)
        {
            std::cout << tags::yellow_bold << "Warning: Failed to erase trash folder '" << PACKAGE_TEMP_LOCATION
                      << "'.\n"
                      << "If left unfixed, this could cause issues.\n"
                      << tags::reset;
        }
    }
    catch (std::runtime_error &e)
    {
        // Clean up garbage

        if (fs::remove_all(PACKAGE_TEMP_LOCATION) == 0)
        {
            std::cout << tags::yellow_bold << "Warning: Failed to erase trash folder '" << PACKAGE_TEMP_LOCATION
                      << "'.\n"
                      << "If left unfixed, this could cause issues.\n"
                      << tags::reset;
        }

        throw package_error(e.what());
    }

    return;
}

std::list<std::string> getPackageFiles(const std::string &Name, AcornSettings &settings)
{
    // If package is not already loaded
    if (settings.packages.count(Name) == 0)
    {
        if (fs::exists("/usr/include/oak/" + Name))
        {
            // Installed, but not loaded; Load and continue
            loadPackageInfo("/usr/include/oak/" + Name + "/" + INFO_FILE, settings);
        }
        else
        {
            // Not installed or loaded; Throw error
            throw package_error("Package '" + Name + "' could not be found. Ensure it is installed.");
        }
    }

    // Loaded and installed
    PackageInfo info = settings.packages[Name];
    std::list<std::string> out;
    std::string cur, toSplit = info.toInclude;

    for (int i = 0; i < toSplit.size(); i++)
    {
        if (toSplit[i] == ',')
        {
            if (cur != "")
            {
                out.push_back(PACKAGE_INCLUDE_PATH + Name + "/" + cur);
            }
            cur = "";
        }
        else
        {
            cur.push_back(toSplit[i]);
        }
    }

    if (cur != "")
    {
        out.push_back(PACKAGE_INCLUDE_PATH + Name + "/" + cur);
    }

    return out;
}

// Remove macro
#undef pm_assert
