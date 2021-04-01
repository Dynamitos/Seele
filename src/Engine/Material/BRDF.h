#pragma once
#include "MinimalEngine.h"
#include "Containers/List.h"
#include <nlohmann/json_fwd.hpp>

namespace Seele
{
class BRDF
{
public:
    static BRDF* getBRDFByName(const std::string& name);
    static List<BRDF*> getBRDFList();

    virtual void generateMaterialCode(std::ofstream& codeStream, nlohmann::json codeJson) = 0;
protected:
    BRDF(const char* name);
    virtual ~BRDF();
    static List<BRDF*> globalBRDFList;
    const char* name;
};

#define DECLARE_BRDF(inputClass) \
    public: \
    static inputClass staticType;

#define IMPLEMENT_BRDF(inputClass) \
    inputClass inputClass::staticType( \
        #inputClass);

class BlinnPhong : public BRDF
{
    DECLARE_BRDF(BlinnPhong)
public:
    virtual void generateMaterialCode(std::ofstream& codeStream, nlohmann::json codeJson);
protected:
    BlinnPhong(const char* name);
    virtual ~BlinnPhong();
};
} // namespace Seele
