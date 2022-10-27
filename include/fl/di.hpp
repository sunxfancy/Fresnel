#pragma once

#include <any>
#include <functional>
#include <map>
#include <string>

namespace fl {

template <typename T>
using sptr = std::shared_ptr<T>;
template <typename T>
using uptr = std::unique_ptr<T>;
template <typename T>
using wptr = std::weak_ptr<T>;

class DI;

// 这里创建了一个接口，用来向目标类注入依赖
class IModule {
public:
    virtual void DepsOn(DI* di) {}
    virtual void Init() {}
    virtual ~IModule() {}
};

class DI {
public:
    using DepsCall = std::function<IModule*(void)>;

    template <typename T>
    sptr<T> create(std::string name) {
        auto p = constructors.find(name);
        if (p == constructors.end()) return nullptr;
        IModule* module = constructors[name]();
        module->DepsOn(this);
        module->Init();
        return sptr<T>(dynamic_cast<T*>(module));
    }

    template <class T>
    sptr<T> get(std::string name) {
        auto p = record.find(name);
        if (p != record.end()) 
            if (auto sp = p->second.lock()) 
                return std::dynamic_pointer_cast<T>(sp);
        auto sp = create<T>(name);
        sptr<IModule> t = std::dynamic_pointer_cast<IModule>(sp);
        record[name] = t;
        return sp;
    }

    template <class T>
    sptr<T> getInst(std::string name) {
        auto p = cache.find(name);
        if (p != cache.end()) return p->second;
        auto sp     = get<T>(name);
        cache[name] = sp;
        return sp;
    }

    DepsCall& operator[](std::string name) { return constructors[name]; }

protected:
    std::map<std::string, wptr<IModule> > record;
    std::map<std::string, sptr<IModule> > cache;
    std::map<std::string, DepsCall>       constructors;
};

#define DEPS_ON           \
    friend class IModule; friend class DI; \
    virtual void DepsOn(DI* di);

#define DEPS_REG(type, obj) obj = di->get<type>(#type)

}  // namespace fl
