#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <filesystem>
#include "nlohmann/json.hpp" // Requires https://github.com/nlohmann/json

namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace std;

// Global structures
unordered_map<string, vector<string>> registry;
unordered_set<string> visited;

void resolveDependencies(const string &pkg, vector<string> &installOrder)
{
    if (visited.count(pkg))
        return;
    visited.insert(pkg);

    for (const string &dep : registry[pkg])
    {
        resolveDependencies(dep, installOrder);
    }

    installOrder.push_back(pkg); // Install after all its deps
}

unordered_map<string, vector<string>> loadRegistry(const string &filename)
{
    ifstream in(filename);
    if (!in)
    {
        cerr << "Failed to open registry file.\n";
        exit(1);
    }

    json j;
    in >> j;

    unordered_map<string, vector<string>> reg;
    for (auto &[key, value] : j.items())
    {
        reg[key] = value.get<vector<string>>();
    }
    return reg;
}

vector<string> readRequirements(const string &filename)
{
    vector<string> reqs;
    ifstream in(filename);
    string pkg;
    while (getline(in, pkg))
    {
        if (!pkg.empty())
            reqs.push_back(pkg);
    }
    return reqs;
}

void installPackage(const string &pkg)
{
    fs::path path = "packages/" + pkg;
    if (!fs::exists(path))
    {
        fs::create_directories(path);
        ofstream dummy(path / "installed.txt");
        dummy << "This is a dummy file for package: " << pkg << "\n";
        cout << "Installed " << pkg << "\n";
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3 || string(argv[1]) != "install")
    {
        cerr << "Usage: ./mypip install myreqs.txt\n";
        return 1;
    }

    string requirementsFile = argv[2];

    registry = loadRegistry("registry.json");
    vector<string> reqs = readRequirements(requirementsFile);
    vector<string> installOrder;

    for (const string &pkg : reqs)
    {
        resolveDependencies(pkg, installOrder);
    }

    fs::create_directories("packages");

    for (const string &pkg : installOrder)
    {
        installPackage(pkg);
    }

    cout << " All dependencies installed successfully.\n";
    return 0;
}
