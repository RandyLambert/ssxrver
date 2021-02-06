#include "util/Init.h"
int main(int argc,char** argv) // -f conf_file_path
{
    if(argc != 1 && argc != 3)
        LOG_FATAL << "-f conf_file_path";
    if(argc == 1) {
        ssxrver::util::Init::getInstance().start(std::string());
    } else if(argc == 3) {
        if(strcmp(argv[1],"-f") != 0) {
            LOG_FATAL << "-f conf_file_path";
        }
        ssxrver::util::Init::getInstance().start(argv[2]);
    }
    return 0;
}