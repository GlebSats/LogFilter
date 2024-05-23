#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

void processingFile(const fs::path &inputFile, const fs::path& outputFile) {
    // Открытие файла из входного каталога
    std::ifstream readFile(inputFile);
    if (!readFile.is_open()) {
        throw std::exception("Opening input file error");
    }
    // Создание файла выходного каталога
    std::ofstream writeFile(outputFile);
    if (!writeFile.is_open()) {
        throw std::exception("Creating output file error");
    }
    //Поиск важных записей и их копирование
    std::string line;
    bool importantLog = false;
    bool importantRecords = false;
    while (std::getline(readFile, line)) {
        if (line.find("[ERROR]") == 0 || line.find("[WARNING]") == 0) {
            importantLog = true;
            importantRecords = true;
        }
        if (line.find("[INFO]") == 0 || line.find("[DEBUG]") == 0) {
            importantLog = false;
        }
        if (importantLog) {
            writeFile << line + '\n';
        }
    }
    //Закрытие файлов
    readFile.close();
    writeFile.close();
    //Удаление пустого файла
    if (!importantRecords) {
        fs::remove(outputFile);
    }
}

void processingFolders(const fs::path input, const fs::path output) {
    for (const auto& entry: fs::recursive_directory_iterator(input)) {
        if (entry.is_regular_file() && (entry.path().extension() == ".txt" || entry.path().extension() == ".log")) {
            fs::path relativePath = fs::relative(entry.path(), input);
            fs::path outputFile = output / relativePath;
            fs::create_directories(outputFile.parent_path());
            processingFile(entry.path(), outputFile); //Просмотр входного файла и копирование записей
        }
    }
    //Удаление пустых папок
    for (auto entry = fs::recursive_directory_iterator(output); entry != fs::recursive_directory_iterator();) {
        fs::path a = entry->path();
        if (entry->is_directory() && fs::is_empty(entry->path())) {
            auto currentPath = entry->path();
            auto it = std::next(entry);
            fs::remove(currentPath);
            entry = it;
        }
        else {
            entry++;
        }
    }
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");

    if (argc != 3) {
        std::cerr << "Wrong arguments.\nUse: " << argv[0] << " input_directory output_directory" << std::endl;
        return EXIT_FAILURE;
    }

    fs::path inputDirectory(argv[1]);
    fs::path outputDirectory(argv[2]);

    if (!fs::exists(inputDirectory) || !fs::is_directory(inputDirectory)) {
        std::cerr << "Operation failed: Input directory does not exist or is not a directory" << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        fs::remove_all(outputDirectory); //Очистка выходного каталога, если имеется
        processingFolders(inputDirectory, outputDirectory); //Рекурсивный обход входного каталога

        std::cout << "Operation succeeded" << std::endl;
        return EXIT_SUCCESS;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Operation failed: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
}


