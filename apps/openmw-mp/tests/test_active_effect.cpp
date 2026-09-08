#include <components/openmw-mp/Base/records/Records.hpp>
#include <cstdio>
#include "apps/openmw/mwmp/RecordConvertPlayer.hpp"
using namespace mwmp;
static int fails=0;
static void ck(const char* w,bool ok){ std::printf("  %-50s %s\n",w,ok?"ok":"FAIL"); if(!ok)++fails; }
static mwmp::records::ActiveEffect trip(mwmp::records::ActiveEffect in){
    ESM::ActiveEffect e{}; RecordConvert::toEngine(in,e);
    mwmp::records::ActiveEffect o{}; RecordConvert::fromEngine(e,o); return o; }
int main()
{
    // DrainSkill (effect 24) targeting Long Blade (skill 5)
    { mwmp::records::ActiveEffect a{}; a.mEffectId=ESM::MagicEffect::refIdToIndex(ESM::MagicEffect::DrainSkill);
      a.mArg=5; a.mMagnitude=10.f; a.mDuration=30.f; a.mTimeLeft=12.f; a.mEffectIndex=1;
      auto b=trip(a);
      ck("DrainSkill effect id round-trips", b.mEffectId==a.mEffectId);
      ck("DrainSkill arg resolves as skill 5", b.mArg==5);
      ck("magnitude/duration preserved", b.mMagnitude==10.f && b.mDuration==30.f && b.mTimeLeft==12.f); }
    // DamageAttribute targeting Strength (attribute 0)
    { mwmp::records::ActiveEffect a{}; a.mEffectId=ESM::MagicEffect::refIdToIndex(ESM::MagicEffect::DamageAttribute);
      a.mArg=0; auto b=trip(a);
      ck("DamageAttribute arg resolves as attribute 0", b.mArg==0); }
    // FortifyAttribute targeting Luck (attribute 7)
    { mwmp::records::ActiveEffect a{}; a.mEffectId=ESM::MagicEffect::refIdToIndex(ESM::MagicEffect::FortifyAttribute);
      a.mArg=7; auto b=trip(a);
      ck("FortifyAttribute arg 7 round-trips", b.mArg==7); }
    // FireDamage: no skill/attribute arg
    { mwmp::records::ActiveEffect a{}; a.mEffectId=ESM::MagicEffect::refIdToIndex(ESM::MagicEffect::FireDamage);
      a.mArg=-1; auto b=trip(a);
      ck("FireDamage keeps arg -1", b.mArg==-1); }
    // Summon: arg is an actor index, not a skill
    { mwmp::records::ActiveEffect a{}; a.mEffectId=ESM::MagicEffect::refIdToIndex(ESM::MagicEffect::SummonScamp);
      a.mArg=42; auto b=trip(a);
      ck("SummonScamp actor index round-trips", b.mArg==42); }
    // magnitude range collapsed onto the single wire value
    { mwmp::records::ActiveEffect a{}; a.mEffectId=14; a.mArg=-1; a.mMagnitude=7.f;
      ESM::ActiveEffect e{}; RecordConvert::toEngine(a,e);
      ck("magnitude range collapsed onto wire value",
         e.mMinMagnitude==7.f && e.mMaxMagnitude==7.f && e.mMagnitude==7.f); }
    std::printf("\n%s\n", fails?"FAILURES":"ActiveEffect conversion verified");
    return fails;
}
