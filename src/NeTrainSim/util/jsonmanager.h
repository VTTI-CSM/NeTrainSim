#ifndef JSONMANAGER_H
#define JSONMANAGER_H

#include "util/vector.h"
#include <string>
#include <QJsonObject>
#include <QJsonArray>

class JSONManager
{
public:
    JSONManager(Vector<std::string> fileKeys, Vector<std::string> fileCriticalKeys);
    ~JSONManager();
    Vector<Vector<std::string>> read(const std::string& filename);
    void write(const std::string& filename, const Vector<Vector<std::string>>& data);

private:
    Vector<std::string> keys;
    Vector<std::string> criticalKeys;
};

#endif // JSONMANAGER_H
