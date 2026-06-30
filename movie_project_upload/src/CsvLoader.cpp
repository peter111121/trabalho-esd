#include "CsvLoader.h"
#include <fstream>
#include <iostream>
#include <unordered_map>

namespace {  // these helpers are private to this file

// Remove leading/trailing spaces, tabs, newlines
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Split ONE csv line into fields, respecting quotes
std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                // a doubled "" means a literal quote character
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    field += '"';
                    ++i;            // skip the second quote
                } else {
                    inQuotes = false;   // closing quote
                }
            } else {
                field += c;
            }
        } else {
            if (c == '"')      inQuotes = true;
            else if (c == ',') { fields.push_back(field); field.clear(); }
            else               field += c;
        }
    }
    fields.push_back(field);  // last field after the final comma
    return fields;
}

// Split a single field like "Action, Comedy" into {"Action","Comedy"}
std::vector<std::string> splitOnComma(const std::string& s) {
    std::vector<std::string> out;
    std::string token;
    for (char c : s) {
        if (c == ',') {
            std::string t = trim(token);
            if (!t.empty()) out.push_back(t);
            token.clear();
        } else {
            token += c;
        }
    }
    std::string t = trim(token);
    if (!t.empty()) out.push_back(t);
    return out;
}

// Pull the year out of a date string like "2008-09-12"
int parseYear(const std::string& date) {
    std::string d = trim(date);
    if (d.size() >= 4) {
        try { return std::stoi(d.substr(0, 4)); }
        catch (...) { return -1; }
    }
    return -1;
}

// Turn "92" into 92.0; empty or broken -> -1.0
double parseDouble(const std::string& s) {
    std::string t = trim(s);
    if (t.empty()) return -1.0;
    try { return std::stod(t); }
    catch (...) { return -1.0; }
}

} // end anonymous namespace

std::vector<Movie> CsvLoader::load(const std::string& path) {
    std::vector<Movie> movies;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR: could not open " << path << std::endl;
        return movies;   // empty list signals failure
    }

    std::string line;
    if (!std::getline(file, line)) {
        std::cerr << "ERROR: file is empty\n";
        return movies;
    }

    // The first line is the header. Build a map: column name -> position.
    std::vector<std::string> headers = splitCsvLine(line);
    std::unordered_map<std::string, int> col;
    for (int i = 0; i < (int)headers.size(); ++i) {
        col[trim(headers[i])] = i;
    }

    // Safely fetch a field by column NAME (returns "" if missing)
    auto get = [&](const std::vector<std::string>& f, const std::string& name) -> std::string {
        auto it = col.find(name);
        if (it == col.end()) return "";
        int idx = it->second;
        if (idx < 0 || idx >= (int)f.size()) return "";
        return trim(f[idx]);
    };

    while (std::getline(file, line)) {
        if (trim(line).empty()) continue;
        std::vector<std::string> f = splitCsvLine(line);

        Movie m;
        m.id             = get(f, "rotten_tomatoes_link");
        m.title          = get(f, "movie_title");
        m.year           = parseYear(get(f, "original_release_date"));
        m.genres         = splitOnComma(get(f, "genres"));
        m.director       = get(f, "directors");
        m.actors         = splitOnComma(get(f, "actors"));
        m.criticRating   = parseDouble(get(f, "tomatometer_rating"));
        m.audienceRating = parseDouble(get(f, "audience_rating"));
        m.criticCount    = (int)parseDouble(get(f, "tomatometer_count"));
        m.audienceCount  = (int)parseDouble(get(f, "audience_count"));
        m.runtime        = (int)parseDouble(get(f, "runtime"));

        if (!m.title.empty())
            movies.push_back(std::move(m));
    }

    return movies;
}