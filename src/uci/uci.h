#include "utils/type.h"
#include <string>

class UCI{
public:
    UCI() = default;
    void loop();

private:
    void parseCommand(const std::string &command);
    void handleUCI();
    void handleIsReady();
    void handleQuit();
};