#include "myserver/config.h"
#include "myserver/log.h"

myserver::ConfigVar<int>::ptr g_int_value_config = 
    myserver::Config::Lookup("system.port", (int)8080, "system port");

myserver::ConfigVar<float>::ptr g_float_value_config = 
    myserver::Config::Lookup("system.value", (float)10.2f, "system value");

int main(int argc,char** argv){
	LOG_INFO(ROOT_LOGGER()) << g_int_value_config->getValue();
    LOG_INFO(ROOT_LOGGER()) << g_float_value_config->toString();
    return 0;
}