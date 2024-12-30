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

    gamemodeConfigOptions->mBuffer[0].copy(u"Toggle H&S Gravity On");
    gamemodeConfigOptions->mBuffer[1].copy(u"Toggle H&S Gravity Off");
    gamemodeConfigOptions->mBuffer[1].copy(u"Toggle Player Collision On");
    gamemodeConfigOptions->mBuffer[2].copy(u"Toggle Player Collision Off");
    gamemodeConfigOptions->mBuffer[3].copy(u"Toggle Player Bounce Off");
    gamemodeConfigOptions->mBuffer[4].copy(u"Toggle Player Bounce On");
    gamemodeConfigOptions->mBuffer[5].copy(u"Toggle Cappy Bounce Off");
    gamemodeConfigOptions->mBuffer[6].copy(u"Toggle Cappy Bounce On");
    gamemodeConfigOptions->mBuffer[7].copy(u"Toggle Cappy Collision Off");
    gamemodeConfigOptions->mBuffer[8].copy(u"Toggle Cappy Collision On");

    return gamemodeConfigOptions->mBuffer;
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
        case 2: {
            if (GameModeManager::instance()->isMode(GameMode::HIDEANDSEEK)) {
                curMode->mIsUseGravity = false;
            }
            return true;
        }
        case 3: {
            if (GameModeManager::instance()->isMode(GameMode::HIDEANDSEEK)) {
                curMode->mIsUseGravity = false;
            }
            return true;
        }
        case 4: {
            if (GameModeManager::instance()->isMode(GameMode::HIDEANDSEEK)) {
                curMode->mIsUseGravity = false;
            }
            return true;
        }
        case 5: {
            if (GameModeManager::instance()->isMode(GameMode::HIDEANDSEEK)) {
                curMode->mIsUseGravity = false;
            }
            return true;
        }
        case 6: {
            if (GameModeManager::instance()->isMode(GameMode::HIDEANDSEEK)) {
                curMode->mIsUseGravity = false;
            }
            return true;
        }
        case 7: {
            if (GameModeManager::instance()->isMode(GameMode::HIDEANDSEEK)) {
                curMode->mIsUseGravity = false;
            }
            return true;
        }
        case 8: {
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