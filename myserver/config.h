#ifndef __MYSERVER_CONFIG_H__
#define __MYSERVER_CONFIG_H__

#include <memory>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "myserver/log.h"

namespace myserver {

// 配置变量的基类
class ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name, const std::string& description = "")
        :m_name(name)
        ,m_description(description){
    }

    virtual ~ConfigVarBase() {};

    const std::string& getName() const { return m_name; }
    const std::string& getDescription() const { return m_description; }

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& val) = 0;

protected:
    std::string m_name;         // 配置参数名称
    std::string m_description;  // 配置参数描述
};


// 配置参数模板子类,保存对应类型的参数值
template<class T>
class ConfigVar : public ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVar> ptr;

    ConfigVar(const std::string& name, const T& default_value, const std::string& description = "")
        :ConfigVarBase(name, description)
        ,m_val(default_value) {
    }

    std::string toString() override {
        try{
            return boost::lexical_cast<std::string>(m_val);
        }catch(std::exception& e){
            LOG_ERROR(ROOT_LOGGER()) << "ConfigVar::toString exception"
            << e.what() << "convert: " << typeid(m_val).name() << " to string";
        }
        return "";
    }

    bool fromString(const std::string& val) override {
        try{
            m_val = boost::lexical_cast<T>(m_val);
        }catch(std::exception& e){
            LOG_ERROR(ROOT_LOGGER()) << "ConfigVar::fromString exception"
            << e.what() << "convert: string to " << typeid(m_val).name();
        }
        return false;
    }

    const T getValue() const { return m_val; }
    void setValue(const T& val) { m_val = val; }
private:
    T m_val;
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
     * @return 返回对应的配置参数,如果参数名存在但是类型不匹配则返回nullptr
     * @exception 如果参数名包含非法字符[^0-9a-z_.] 抛出异常 std::invalid_argument
     */
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name,
            const T& default_value, const std::string& description = ""){
        auto tmp = Lookup<T>(name);
        if (tmp){
            LOG_INFO(ROOT_LOGGER()) << "Lookup name=" << name << " exists";
            return tmp;
        }
        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIGKLMNOPQRSTUVWXYZ._0123456789") 
                != std::string::npos){
            LOG_ERROR(ROOT_LOGGER()) << "Lookup name invaild" << name;
            throw std::invalid_argument(name);
        }

        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
        s_data[name] = v;
        return v;
    }

    /**
     * @brief 查找配置参数
     * @param[in] name 配置参数名称
     * @return 返回配置参数名为name的配置参数
     */
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name){
        auto it = s_data.find(name);
        if (it == s_data.end()){
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }
private:
    static ConfigVarMap s_data;
};


}


#endif