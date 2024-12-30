#include "server/hns/HideAndSeekConfigMenu.hpp"
#include <cmath>
#include "logger.hpp"
#include "server/gamemode/GameModeManager.hpp"
#include "server/hns/HideAndSeekMode.hpp"
#include "server/Client.hpp"

HideAndSeekConfigMenu::HideAndSeekConfigMenu() : GameModeConfigMenu() {}

void HideAndSeekConfigMenu::initMenu(const al::LayoutInitInfo &initInfo) {
    
}

const sead::WFixedSafeString<0x200> *HideAndSeekConfigMenu::getStringData() {
    sead::SafeArray<sead::WFixedSafeString<0x200>, mItemCount>* gamemodeConfigOptions =
        new sead::SafeArray<sead::WFixedSafeString<0x200>, mItemCount>();

    mItems->mBuffer[0].copy(u"Toggle H&S Gravity (OFF)");
    mItems->mBuffer[1].copy(u"Mario Collision (ON)    ");
    mItems->mBuffer[2].copy(u"Mario Bounce (ON)       ");
    mItems->mBuffer[3].copy(u"Cappy Collision (OFF)   ");
    mItems->mBuffer[4].copy(u"Cappy Bounce (OFF)      ");
}

const sead::WFixedSafeString<0x200>* HideAndSeekConfigMenu::getStringData() {
    // Gravity
    const char16_t* gravity = (
        HideAndSeekInfo::mIsUseGravity
        ? u"Toggle H&S Gravity (ON) "
        : u"Toggle H&S Gravity (OFF)"
    );

    // Collision Toggles
    const char16_t* marioCollision = (
        HideAndSeekInfo::mHasMarioCollision
        ? u"Mario Collision (ON)    "
        : u"Mario Collision (OFF)   "
    );
    const char16_t* marioBounce = (
        HideAndSeekInfo::mHasMarioBounce
        ? u"Mario Bounce (ON)       "
        : u"Mario Bounce (OFF)      "
    );
    const char16_t* cappyCollision = (
        HideAndSeekInfo::mHasCappyCollision
        ? u"Cappy Collision (ON)    "
        : u"Cappy Collision (OFF)   "
    );
    const char16_t* cappyBounce = (
        HideAndSeekInfo::mHasCappyBounce
        ? u"Cappy Bounce (ON)       "
        : u"Cappy Bounce (OFF)      "
    );

    mItems->mBuffer[0].copy(gravity);
    mItems->mBuffer[1].copy(marioCollision);
    mItems->mBuffer[2].copy(marioBounce);
    mItems->mBuffer[3].copy(cappyCollision);
    mItems->mBuffer[4].copy(cappyBounce);

    return mItems->mBuffer;
}

bool HideAndSeekConfigMenu::updateMenu(int selectIndex) {

    HideAndSeekInfo *curMode = GameModeManager::instance()->getInfo<HideAndSeekInfo>();

    Logger::log("Setting Gravity Mode.\n");

    if (!curMode) {
        Logger::log("Unable to Load Mode info!\n");
        return true;   
    }
    
    switch (selectIndex) {
        case 0: {
            if (GameModeManager::instance()->isMode(GameMode::HIDEANDSEEK)) {
                curMode->mIsUseGravity = true;
            }
            return true;
        }
        case 1: {
            if (GameModeManager::instance()->isMode(GameMode::HIDEANDSEEK)) {
                curMode->mIsUseGravity = false;
            }
            return true;
        }
        default:
            Logger::log("Failed to interpret Index!\n");
            return false;
    }
    
}