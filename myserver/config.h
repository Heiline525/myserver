#ifndef __MYSERVER_CONFIG_H__
#define __MYSERVER_CONFIG_H__

#include <memory>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include "myserver/log.h"
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <functional>

namespace myserver {

// 配置变量的基类
class ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name, const std::string& description = "")
        :m_name(name)
        ,m_description(description){
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    }

    virtual ~ConfigVarBase() {};

    const std::string& getName() const { return m_name; }
    const std::string& getDescription() const { return m_description; }

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& val) = 0;
    virtual std::string getTypeName() const = 0;
protected:
    std::string m_name;         // 配置参数名称
    std::string m_description;  // 配置参数描述
};

// 基本类型转换
template<class F, class T>
class LexicalCast {
public:
    T operator()(const F& v) {
        return boost::lexical_cast<T>(v);
    }
};

// 类型转换偏特化： string --> vector
template<class T>
class LexicalCast<std::string, std::vector<T>> {
public:
    std::vector<T> operator()(const std::string& v) { 
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> vec_T;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec_T.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec_T;
    }
};

// 类型转换偏特化： vector --> string
template<class T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator()(const std::vector<T>& v) {    
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 类型转换偏特化： string --> list
template<class T>
class LexicalCast<std::string, std::list<T>> {
public:
    std::list<T> operator()(const std::string& v) { 
        YAML::Node node = YAML::Load(v);
        typename std::list<T> list_T;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            list_T.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return list_T;
    }
};

// 类型转换偏特化： list --> string
template<class T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator()(const std::list<T>& v) {    
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


// 类型转换偏特化： string --> set
template<class T>
class LexicalCast<std::string, std::set<T>> {
public:
    std::set<T> operator()(const std::string& v) { 
        YAML::Node node = YAML::Load(v);
        typename std::set<T> set_T;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            set_T.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return set_T;
    }
};

// 类型转换偏特化： set --> string
template<class T>
class LexicalCast<std::set<T>, std::string> {
public:
    std::string operator()(const std::set<T>& v) {    
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 类型转换偏特化： string --> unordered_set
template<class T>
class LexicalCast<std::string, std::unordered_set<T>> {
public:
    std::unordered_set<T> operator()(const std::string& v) { 
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> set_T;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            set_T.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return set_T;
    }
};

// 类型转换偏特化： unordered_set --> string
template<class T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
    std::string operator()(const std::unordered_set<T>& v) {    
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 类型转换偏特化： string --> map
template<class T>
class LexicalCast<std::string, std::map<std::string, T> > {
public:
    std::map<std::string, T> operator()(const std::string& v) { 
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> map_T;
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            map_T.insert(std::make_pair(it->first.Scalar(), 
                                        LexicalCast<std::string, T>()(ss.str())));
        }
        return map_T;
    }
};

// 类型转换偏特化： map --> string
template<class T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(const std::map<std::string, T>& v) {    
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 类型转换偏特化： string --> unordered_map
template<class T>
class LexicalCast<std::string, std::unordered_map<std::string, T> > {
public:
    std::unordered_map<std::string, T> operator()(const std::string& v) { 
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> map_T;
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            map_T.insert(std::make_pair(it->first.Scalar(), 
                                        LexicalCast<std::string, T>()(ss.str())));
        }
        return map_T;
    }
};

// 类型转换偏特化： unordered_map --> string
template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, T>& v) {    
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// 配置参数模板子类,保存对应类型的参数值
// FromStr T operator()（const std::string&)，默认特例化支持基本类型转换
// ToStr std::string operator() (const T&)，默认特例化支持基本类型转换
template<class T, class FromStr = LexicalCast<std::string, T>, 
                  class ToStr = LexicalCast<T, std::string> >
class ConfigVar : public ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void (const T& old_value, const T& new_value)> on_change_cb;

    ConfigVar(const std::string& name, const T& default_value, const std::string& description = "")
        :ConfigVarBase(name, description)
        ,m_val(default_value) {
    }

    std::string toString() override {
        try{
            return ToStr()(m_val);
        }catch(std::exception& e){
            LOG_ERROR(ROOT_LOGGER()) << "ConfigVar::toString exception "
            << e.what() << " convert: " << typeid(m_val).name() << " to string";
        }
        return "";
    }

    bool fromString(const std::string& val) override {
        try{
            setValue(FromStr()(val));
        }catch(std::exception& e){
            LOG_ERROR(ROOT_LOGGER()) << "ConfigVar::fromString exception "
            << e.what() << " convert: string to " << typeid(val).name();
        }
        return false;
    }

    std::string getTypeName() const override { return typeid(T).name(); }
    const T getValue() const { return m_val; }
    // 设置当前参数的值,如果参数的值有发生变化,则通知对应的注册回调函数
    void setValue(const T& val) { 
        if (m_val == val) {
            return;
        }
        for (auto &i : m_cbs) {
            i.second(m_val, val);   // 传入旧值，新值给回调函数
        }
        m_val = val;
    }
    
    // 对于变更回调函数数组的管理
    void addListener(uint64_t key, on_change_cb cb) { m_cbs[key] = cb; }
    void delListener(uint64_t key) { m_cbs.erase(key); }
    on_change_cb getListener(uint64_t key) {
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }
    void clearListener() { m_cbs.clear(); }
private:
    T m_val;
    std::map<uint64_t, on_change_cb> m_cbs;     // 变更回调函数数组，uint64_t唯一，一般使用hash值
};

// ConfigVar的管理类
class Config {
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

    /**
     * @brief 获取/创建对应参数名的配置参数
     * @param[in] name 配置参数名称
     * @param[in] default_value 参数默认值
     * @param[in] description 参数描述
     * @details 获取参数名为name的配置参数,如果存在直接返回
     *          如果不存在,创建参数配置并用default_value赋值
     * @return 返回对应的配置参数, 如果参数名存在但是类型不匹配则返回nullptr
     * @exception 如果参数名包含非法字符[^0-9a-z_.] 抛出异常 std::invalid_argument
     */
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name,
            const T& default_value, const std::string& description = ""){
        auto it = GetDatas().find(name);
        if (it != GetDatas().end()) {
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
            if (tmp){
                LOG_INFO(ROOT_LOGGER()) << "Lookup name=" << name << " exists";
                return tmp;
            } else {
                LOG_ERROR(ROOT_LOGGER()) << "Lookup name=" << name 
                                         << " exists but type not " << typeid(T).name()
                                         << " real_type=" << it->second->getTypeName()
                                         << " " << it->second->toString();
                return nullptr;
            }
        }

        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIGKLMNOPQRSTUVWXYZ._0123456789") 
                != std::string::npos){
            LOG_ERROR(ROOT_LOGGER()) << "Lookup name invaild" << name;
            throw std::invalid_argument(name);
        }

        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
        GetDatas()[name] = v;
        return v;
    }

    /**
     * @brief 查找配置参数
     * @param[in] name 配置参数名称
     * @return 返回配置参数名为name的配置参数
     */
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name){
        auto it = GetDatas().find(name);
        if (it == GetDatas().end()){
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    // 使用YAML::Node初始化配置模块
    static void LoadFromYaml(const YAML::Node& root);
    static void ListAllMember(const std::string& prefix,
                              const YAML::Node& node,
                              std::list<std::pair<std::string, const YAML::Node>>& output);
    /**
     * @brief 查找配置参数, 返回配置参数的基类
     * @param[in] name 配置参数名称
     */
    static ConfigVarBase::ptr LookupBase(const std::string& name);
private:
    static ConfigVarMap& GetDatas() {
        static ConfigVarMap s_data;
        return s_data;
    }
};


}


#endif