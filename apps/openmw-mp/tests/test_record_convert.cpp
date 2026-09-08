#include <components/openmw-mp/Base/records/Records.hpp>
#include <cstdio>
#include "apps/openmw/mwmp/RecordConvert.hpp"
using namespace mwmp;
static int fails=0;
static void ck(const char* w,bool ok){ std::printf("  %-46s %s\n",w,ok?"ok":"FAIL"); if(!ok)++fails; }

template <class W, class E>
static W trip(const W& in) { E eng{}; RecordConvert::toEngine(in,eng); W out{}; RecordConvert::fromEngine(eng,out); return out; }

int main()
{
    { // Potion: mAutoCalc <-> mFlags rename
        mwmp::records::Potion p{}; p.mId="p_restore_health"; p.mScript="myscript";
        p.mData.mWeight=0.5f; p.mData.mValue=25; p.mData.mAutoCalc=1;
        auto b=trip<mwmp::records::Potion,ESM::Potion>(p);
        ck("Potion id/script round-trip", b.mId=="p_restore_health" && b.mScript=="myscript");
        ck("Potion mAutoCalc survives rename to mFlags", b.mData.mAutoCalc==1);
        ck("Potion weight/value preserved", b.mData.mWeight==0.5f && b.mData.mValue==25);
    }
    { // Miscellaneous: mIsKey <-> mFlags rename
        mwmp::records::Miscellaneous m{}; m.mId="misc_key"; m.mData.mIsKey=1; m.mData.mValue=7;
        auto b=trip<mwmp::records::Miscellaneous,ESM::Miscellaneous>(m);
        ck("Misc mIsKey survives rename to mFlags", b.mData.mIsKey==1 && b.mData.mValue==7);
    }
    { // Weapon: std::array damage pairs
        mwmp::records::Weapon w{}; w.mId="iron dagger"; w.mEnchant="ench1";
        w.mData.mChop[0]=2; w.mData.mChop[1]=8;
        w.mData.mSlash[0]=3; w.mData.mSlash[1]=9;
        w.mData.mThrust[0]=4; w.mData.mThrust[1]=10;
        auto b=trip<mwmp::records::Weapon,ESM::Weapon>(w);
        ck("Weapon chop pair round-trips", b.mData.mChop[0]==2 && b.mData.mChop[1]==8);
        ck("Weapon slash/thrust round-trip",
           b.mData.mSlash[1]==9 && b.mData.mThrust[1]==10);
        ck("Weapon enchant refId round-trips", b.mEnchant=="ench1");
    }
    { // Ingredient: RefId[4] arrays with -1 for unused slots
        mwmp::records::Ingredient g{}; g.mId="ingred_test";
        g.mData.mEffectID[0]=14; g.mData.mSkills[0]=-1; g.mData.mAttributes[0]=-1;
        g.mData.mEffectID[1]=79; g.mData.mSkills[1]=5;  g.mData.mAttributes[1]=-1;
        for(int i=2;i<4;++i){ g.mData.mEffectID[i]=-1; g.mData.mSkills[i]=-1; g.mData.mAttributes[i]=-1; }
        auto b=trip<mwmp::records::Ingredient,ESM::Ingredient>(g);
        ck("Ingredient effect ids round-trip", b.mData.mEffectID[0]==14 && b.mData.mEffectID[1]==79);
        ck("Ingredient skill 5 round-trips", b.mData.mSkills[1]==5);
        ck("Ingredient unused slots stay -1",
           b.mData.mEffectID[3]==-1 && b.mData.mSkills[0]==-1 && b.mData.mAttributes[2]==-1);
    }
    { // Creature: int[6] attack array + AI + inventory
        mwmp::records::Creature c{}; c.mId="rat_test"; c.mScript="";
        for(int i=0;i<6;++i) c.mData.mAttack[i]=i*3;
        c.mAiData.mFight=90; c.mAiData.mServices=1234;
        c.mInventory.mList.push_back({2,"iron dagger"});
        auto b=trip<mwmp::records::Creature,ESM::Creature>(c);
        ck("Creature attack array round-trips", b.mData.mAttack[5]==15 && b.mData.mAttack[1]==3);
        ck("Creature AI data round-trips", b.mAiData.mFight==90 && b.mAiData.mServices==1234);
        ck("Creature inventory round-trips",
           b.mInventory.mList.size()==1 && b.mInventory.mList[0].mItem=="iron dagger");
    }
    { // NPC: many refIds + NPDT + shared structs
        mwmp::records::NPC n{}; n.mId="test_npc"; n.mRace="dark elf"; n.mClass="acrobat";
        n.mFaction="thieves guild"; n.mHair="b_n_dark elf_m_hair01"; n.mHead="b_n_dark elf_m_head01";
        n.mNpdt.mLevel=7; n.mNpdt.mHealth=80; n.mNpdtType=52;
        auto b=trip<mwmp::records::NPC,ESM::NPC>(n);
        ck("NPC refId fields round-trip",
           b.mRace=="dark elf" && b.mClass=="acrobat" && b.mFaction=="thieves guild");
        ck("NPC hair/head round-trip", b.mHair=="b_n_dark elf_m_hair01" && b.mHead=="b_n_dark elf_m_head01");
        ck("NPC npdt round-trips", b.mNpdt.mLevel==7 && b.mNpdt.mHealth==80 && b.mNpdtType==52);
    }
    { // Spell with effects
        mwmp::records::Spell s{}; s.mId="fireball"; s.mData.mCost=15;
        s.mEffects.mList.push_back({14,-1,-1,2,0,30,5,25});
        auto b=trip<mwmp::records::Spell,ESM::Spell>(s);
        ck("Spell effects round-trip",
           b.mEffects.mList.size()==1 && b.mEffects.mList[0].mEffectID==14 && b.mEffects.mList[0].mMagnMax==25);
    }
    std::printf("\n%s\n", fails?"FAILURES":"all 25-type conversions verified");
    return fails;
}
