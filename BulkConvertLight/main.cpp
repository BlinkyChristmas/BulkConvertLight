//
//  main.cpp
//  BulkConvertLight
//
//  Created by Charles Kerr on 2/26/24.
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <stdexcept>

#include <vector>

#include "lightfile/lightfile.hpp"

using namespace std::string_literals ;
// =======================================================================================================================
auto gatherFiles(const std::filesystem::path &path , const std::string &extension) -> std::vector<std::filesystem::path> {
    auto directories = std::vector<std::filesystem::path>();
    // We will not be recursive
    for (auto const& dir_entry : std::filesystem::directory_iterator{path}){
        if (dir_entry.is_regular_file()) {
            if (dir_entry.path().extension().string() == extension) {
                directories.push_back(dir_entry.path()) ;
            }
        }
    }
    return directories ;
}

// =======================================================================================================================
int main(int argc, const char * argv[]) {
    
    auto exitcode = EXIT_SUCCESS ;
    try {
        if (argc != 3) {
            throw (std::runtime_error("Insufficent arguments: inputdir outputdir")) ;
        }
        auto inputpath = std::filesystem::path(argv[1]) ;
        auto outputpath = std::filesystem::path(argv[2]) ;
        if (std::filesystem::exists(inputpath)) {
            // we want all the subdirectories
            auto directories = std::vector<std::filesystem::path>();
            
            // We will not be recursive
            for (auto const& dir_entry : std::filesystem::directory_iterator{inputpath}){
                if (dir_entry.is_directory()) {
                    directories.push_back(dir_entry.path()) ;
                }
            }
            // Ok, so now we have all the directories
            if (!directories.empty()) {
                if (!std::filesystem::exists(outputpath)) {
                    std::filesystem::create_directories(outputpath) ;
                }
                for (const auto &dir:directories) {
                    auto files = gatherFiles(dir, ".diybpb") ;
                    if (!files.empty()){
                        // There are files so we need to make a new directory
                        auto newdir = outputpath / dir.filename() ;
                        if (!std::filesystem::exists(newdir)) {
                            std::filesystem::create_directories(newdir) ;
                        }
                        for (auto file:files) {
                            auto outputfile = newdir / file.stem().replace_extension(".light") ;
                            auto musicname = file.stem().string() ;
                            
                            auto infile = std::ifstream(file.string(),std::ios::binary) ;
                            if (!infile.is_open()) {
                                throw std::runtime_error("Unable to open: "s + file.string()) ;
                            }
                            auto outfile = std::ofstream(outputfile.string(),std::ios::binary) ;
                            if (!outfile.is_open()) {
                                throw std::runtime_error("Unable to create: "s + outputfile.string()) ;
                            }
                            
                            auto oldheader = LightHeader(infile) ;
                            auto newheader = LightHeader() ;
                            
                            newheader.frameCount = oldheader.frameCount ;
                            newheader.frameLength = oldheader.frameLength ;
                            newheader.sourceName = musicname ;
                            newheader.signature = LightHeader::SIGNATURE ;
                            newheader.write(outfile) ;
                            
                            auto current = infile.tellg()   ;
                            infile.seekg(0,std::ios::end) ;
                            auto size = infile.tellg() - current ;
                            infile.seekg(current, std::ios::beg) ;
                            auto buffer = std::vector<char>(size,0) ;
                            infile.read(buffer.data(),buffer.size()) ;
                            outfile.write(buffer.data(),buffer.size()) ;
                            infile.close();
                            outfile.close() ;
                        }
                    }
                }
            }
            
        }
    }
    catch(const std::exception &e) {
        std::cerr << e.what() << std::endl;
        exitcode = EXIT_FAILURE ;
    }
    catch (...) {
        std::cerr << "Uknown Error!" << std::endl;
        exitcode = EXIT_FAILURE ;

    }
    return exitcode;
}
