#include <components/openmw-mp/Base/records/Records.hpp>
#include <cstdio>
#include "apps/openmw/mwmp/RecordConvertCommon.hpp"

using namespace mwmp;
static int fails = 0;
static void check(const char* what, bool ok) {
    std::printf("  %-42s %s\n", what, ok ? "ok" : "FAIL");
    if (!ok) ++fails;
}
int main()
{
    // effect with N/A skill/attribute (the 0.47 "-1" convention)
    mwmp::records::EffectList wire;
    wire.mList.push_back({ 14, -1, -1, 2, 0, 30, 5, 25 });   // fire damage
    wire.mList.push_back({ 79, 5, -1, 0, 10, 60, 1, 3 });    // drain skill (longblade)

    ESM::EffectList engine;
    RecordConvert::toEngine(wire, engine);
    mwmp::records::EffectList back;
    RecordConvert::fromEngine(engine, back);

    check("effect count preserved", back.mList.size() == 2);
    check("effectID 14 round-trips", back.mList[0].mEffectID == 14);
    check("skill -1 stays -1", back.mList[0].mSkill == -1);
    check("attribute -1 stays -1", back.mList[0].mAttribute == -1);
    check("magnitude preserved", back.mList[0].mMagnMax == 25);
    check("effectID 79 round-trips", back.mList[1].mEffectID == 79);
    check("skill 5 round-trips", back.mList[1].mSkill == 5);
    check("engine indices assigned 0,1",
          engine.mList[0].mIndex == 0 && engine.mList[1].mIndex == 1);

    // inventory
    mwmp::records::InventoryList inv;
    inv.mList.push_back({ 3, "iron dagger" });
    ESM::InventoryList einv; RecordConvert::toEngine(inv, einv);
    mwmp::records::InventoryList binv; RecordConvert::fromEngine(einv, binv);
    check("inventory item round-trips",
          binv.mList.size() == 1 && binv.mList[0].mItem == "iron dagger" && binv.mList[0].mCount == 3);

    // body parts
    mwmp::records::PartReferenceList parts;
    parts.mParts.push_back({ 5, "b_n_dark elf_m_foot", "" });
    ESM::PartReferenceList eparts; RecordConvert::toEngine(parts, eparts);
    mwmp::records::PartReferenceList bparts; RecordConvert::fromEngine(eparts, bparts);
    check("body part male id round-trips",
          bparts.mParts.size() == 1 && bparts.mParts[0].mMale == "b_n_dark elf_m_foot");
    check("body part empty female stays empty", bparts.mParts[0].mFemale.empty());

    std::printf("\n%s\n", fails ? "FAILURES" : "all shared conversions verified");
    return fails;
}
