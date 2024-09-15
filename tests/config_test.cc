#include "myserver/config.h"
#include "myserver/log.h"
#include <yaml-cpp/yaml.h>

myserver::ConfigVar<int>::ptr g_int_value_config = 
    myserver::Config::Lookup("system.port", (int)8080, "system port");

myserver::ConfigVar<float>::ptr g_float_value_config = 
    myserver::Config::Lookup("system.value", (float)10.2f, "system value");

void printYaml(const YAML::Node& node, int level){
    if (node.IsScalar()) {
        LOG_INFO(ROOT_LOGGER()) << std::string(level * 4, ' ')
            << node.Scalar() << " - " << node.Type() << " - " << level;
    } else if (node.IsNull()) {
        LOG_INFO(ROOT_LOGGER()) << std::string(level * 4, ' ')
            << "NULL - " << node.Type() << level;
    } else if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            LOG_INFO(ROOT_LOGGER()) << std::string(level * 4, ' ')
                << it->first << " - " << it->second.Type() << " - " << level;
            printYaml(it->second, level+1);
        }
    } else if (node.IsSequence()) {
        for (size_t i = 0; i < node.size(); ++i) {
            LOG_INFO(ROOT_LOGGER()) << std::string(level * 4, ' ')
                << i << " - " << node[i].Type() << " - " << level;
            printYaml(node[i], level+1);
        }
    }
}

void testYaml() {
    YAML::Node root = YAML::LoadFile("/apps/myserver/bin/conf/log.yml");
    printYaml(root, 0);
}

void testConfig(){
    LOG_INFO(ROOT_LOGGER()) << "before: " << g_int_value_config->getValue();
    LOG_INFO(ROOT_LOGGER()) << "before: " << g_float_value_config->getValue();
    YAML::Node root = YAML::LoadFile("/apps/myserver/bin/conf/log.yml");
    myserver::Config::LoadFromYaml(root);

    LOG_INFO(ROOT_LOGGER()) << "after: " << g_int_value_config->getValue();
    LOG_INFO(ROOT_LOGGER()) << "after: " << g_float_value_config->getValue();
}

int main(int argc,char** argv){
    // testYaml();
    testConfig();
    return 0;
}