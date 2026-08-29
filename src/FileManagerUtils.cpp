#include"BasicIncludes.h"

std::string FileManagerUtils::GetExt(const std::string&file){
    size_t ext = file.find_last_of(".");
    if(ext == std::string::npos)
        return "";
    return file.substr(ext+1);
}

bool FileManagerUtils::isBook(const char*file){
    std::string ext = FileManagerUtils::GetExt(file);
    if(ext== "CBZ" || ext == "cbz" || ext == "pdf" || ext == "PDF" || ext == "cbt" || ext == "CBT" || ext == "epub" || ext == "EPUB" || ext == "fb2" || ext == "FB2" || ext == "mobi" || ext == "MOBI" || ext == "xps" || ext == "XPS")
        return true;
    return false;
}

std::vector<FileManagerUtils::FileItem> FileManagerUtils::GetFileList(const char* path) {
    std::vector<FileManagerUtils::FileItem> lista;
    DIR* dir = opendir(path);
        if (std::string(path) != "ux0:/") {
        lista.push_back({"..", true});
}
    
    if (dir != NULL) {
        struct dirent* ent;
        while ((ent = readdir(dir)) != NULL) {
            std::string name = ent->d_name;
            
            // 1. Salta sempre la cartella attuale "."
            if (name == ".") continue;

            std::string fullPath = std::string(path);
            if (fullPath.back() != '/') fullPath += "/";
            fullPath += name;

            // 3. Usa stat() per ottenere informazioni sul file
            struct stat st;
            bool isDirectory = false;
            
            if (stat(fullPath.c_str(), &st) == 0) {
                isDirectory = S_ISDIR(st.st_mode); // Macro che restituisce true se è una cartella
            }

            if (isDirectory) {
                lista.push_back({name, true});
            } else if (isBook(name.c_str())) {
                lista.push_back({name, false});
            }
        }
        closedir(dir);
    }

    std::sort(lista.begin(), lista.end(), [](const FileItem& a, const FileItem& b) {
        if (a.isDir != b.isDir) return a.isDir;
        return a.name < b.name;
    });

    return lista;
}