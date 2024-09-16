#include "myserver/config.h"
#include "myserver/log.h"
#include <yaml-cpp/yaml.h>

myserver::ConfigVar<int>::ptr g_int_value_config = 
    myserver::Config::Lookup("system.port", (int)8080, "system port");

// myserver::ConfigVar<int>::ptr g_int_valuex_config = 
//     myserver::Config::Lookup("system.port", (int)8080, "system port");

// myserver::ConfigVar<float>::ptr g_int_valuexx_config = 
//     myserver::Config::Lookup("system.port", (float)8080, "system port");

myserver::ConfigVar<float>::ptr g_float_value_config = 
    myserver::Config::Lookup("system.value", (float)10.2f, "system value");

myserver::ConfigVar<std::vector<int>>::ptr g_int_vec_value_config = 
    myserver::Config::Lookup("system.int_vec", std::vector<int>{1,2}, "system int vec");

myserver::ConfigVar<std::list<int>>::ptr g_int_list_value_config = 
    myserver::Config::Lookup("system.int_list", std::list<int>{1,2}, "system int list");

myserver::ConfigVar<std::set<int>>::ptr g_int_set_value_config = 
    myserver::Config::Lookup("system.int_set", std::set<int>{1,2}, "system int set");

myserver::ConfigVar<std::unordered_set<int>>::ptr g_int_uset_value_config = 
    myserver::Config::Lookup("system.int_uset", std::unordered_set<int>{1,2}, "system int uset");

myserver::ConfigVar<std::map<std::string,int>>::ptr g_str_int_map_value_config = 
    myserver::Config::Lookup("system.str_int_map", std::map<std::string,int>{{"k",2}}, "system str-int map");

myserver::ConfigVar<std::unordered_map<std::string,int>>::ptr g_str_int_umap_value_config = 
    myserver::Config::Lookup("system.str_int_umap", std::unordered_map<std::string,int>{{"k",2}}, "system str-int umap");

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

#define XX(g_var, name, prefix) {                                   \
        auto v = g_var->getValue();                                 \
        for (auto & i : v) {                                        \
            LOG_INFO(ROOT_LOGGER()) << #prefix " " #name ": " << i; \
        }                                                           \
        LOG_INFO(ROOT_LOGGER()) << #prefix " " #name " yaml: "      \
                                << g_var->toString();               \
    }

#define XX_MAP(g_var, name, prefix) {                               \
        auto v = g_var->getValue();                                 \
        for (auto & i : v) {                                        \
            LOG_INFO(ROOT_LOGGER()) << #prefix " " #name ": {"      \
                        << i.first << " - " << i.second << "}";     \
        }                                                           \
        LOG_INFO(ROOT_LOGGER()) << #prefix " " #name " yaml: "      \
                                << g_var->toString();               \
    }

    XX(g_int_vec_value_config, int_vec, before);
    XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    XX(g_int_uset_value_config, int_uset, before);
    XX_MAP(g_str_int_map_value_config, str_int_map, before);
    XX_MAP(g_str_int_umap_value_config, str_int_umap, before);

    YAML::Node root = YAML::LoadFile("/apps/myserver/bin/conf/log.yml");
    myserver::Config::LoadFromYaml(root);
    
    LOG_INFO(ROOT_LOGGER()) << "after: " << g_int_value_config->getValue();
    LOG_INFO(ROOT_LOGGER()) << "after: " << g_float_value_config->getValue();
    XX(g_int_vec_value_config, int_vec, after);
    XX(g_int_list_value_config, int_list, after);
    XX(g_int_set_value_config, int_set, after);
    XX(g_int_uset_value_config, int_uset, after);
    XX_MAP(g_str_int_map_value_config, str_int_map, after);
    XX_MAP(g_str_int_umap_value_config, str_int_umap, after);


}

class Person {
public:
    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name=" << m_name
           << " age=" << m_age
           << " sex=" << m_sex
           << "]";
        return ss.str();
    }
    std::string m_name = "";
    int m_age = 0;
    bool m_sex = 0;
};

namespace myserver {
// 类型转换偏特化： string --> class Person
template<>
class LexicalCast<std::string, Person> {
public:
    Person operator()(const std::string& v) { 
        YAML::Node node = YAML::Load(v);
        Person p;
        p.m_name = node["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();
        return p;
    }
};

// 类型转换偏特化： class Person --> string
template<>
class LexicalCast<Person, std::string> {
public:
    std::string operator()(const Person& p) { 
        YAML::Node node;
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};  
}

myserver::ConfigVar<Person>::ptr g_person = 
    myserver::Config::Lookup("class.person", Person(), "class person");

myserver::ConfigVar<std::map<std::string, Person>>::ptr g_person_map = 
    myserver::Config::Lookup("class.map", std::map<std::string, Person>(), "class person map");

myserver::ConfigVar<std::map<std::string, std::vector<Person>>>::ptr g_person_vec_map = 
    myserver::Config::Lookup("class.vec_map", std::map<std::string, std::vector<Person>>(), "class person vec_map");

void testClass() {
    LOG_INFO(ROOT_LOGGER()) << "before: " << g_person->getValue().toString(); 
    LOG_INFO(ROOT_LOGGER()) << "before yaml " << g_person->toString();

#define XX_PM(g_var, prefix){                                           \
        auto m = g_person_map->getValue();                              \
        for (auto& i : m) {                                             \
            LOG_INFO(ROOT_LOGGER()) << prefix << ": " << i.first        \
                                    << " - " << i.second.toString();    \
        }                                                               \
        LOG_INFO(ROOT_LOGGER()) << prefix << ": size=" << m.size();     \
    }
    XX_PM(g_person_map, "class.map before");
    LOG_INFO(ROOT_LOGGER()) << "before: " << g_person_vec_map->toString();

    YAML::Node root = YAML::LoadFile("/apps/myserver/bin/conf/log.yml");
    myserver::Config::LoadFromYaml(root);

    LOG_INFO(ROOT_LOGGER()) << "after: " << g_person->getValue().toString();
    LOG_INFO(ROOT_LOGGER()) << "after yaml " << g_person->toString();
    XX_PM(g_person_map, "class.map after");
    LOG_INFO(ROOT_LOGGER()) << "after: " << g_person_vec_map->toString();
}   

int main(int argc,char** argv){
    // testYaml();
    // testConfig();
    testClass();
    return 0;
}