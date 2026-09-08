#include <components/openmw-mp/Base/records/Records.hpp>
#include <cstdio>
#include <string>
#include <algorithm>
#include <components/esm/attr.hpp>
#include <components/esm3/loadskil.hpp>
#include <components/openmw-mp/Base/records/StatNames.hpp>
static std::string lower(std::string_view v){ std::string s(v); std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){return (char)std::tolower(c);}); return s; }
int main()
{
    int fails=0;
    for (int i=0;i<ESM::Attribute::Length;++i){
        std::string engine=ESM::Attribute::indexToRefId(i).serializeText();
        std::string ours=lower(mwmp::records::sAttributeNames[i]);
        if(engine!=ours){ std::printf("  ATTR %d: engine='%s' ours='%s' MISMATCH\n",i,engine.c_str(),ours.c_str()); ++fails; }
    }
    for (int i=0;i<ESM::Skill::Length;++i){
        std::string engine=ESM::Skill::indexToRefId(i).serializeText();
        std::string ours=lower(mwmp::records::sSkillNames[i]);
        if(engine!=ours){ std::printf("  SKILL %d: engine='%s' ours='%s' MISMATCH\n",i,engine.c_str(),ours.c_str()); ++fails; }
    }
    std::printf("checked %d attributes + %d skills against engine RefId spellings\n",
                (int)ESM::Attribute::Length,(int)ESM::Skill::Length);
    std::printf("%s\n", fails?"DRIFT DETECTED":"all names match the engine");
    return fails;
}
